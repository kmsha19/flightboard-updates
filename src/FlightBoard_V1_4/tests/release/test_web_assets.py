#!/usr/bin/env python3
"""Offline provenance tests for the downloadable UI and compiled rescue page."""
import gzip
import hashlib
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('prepare_release', ROOT / 'tools/prepare_release.py')
release = importlib.util.module_from_spec(spec)
spec.loader.exec_module(release)

class WebAssetTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix='zcs-web-provenance-')
        self.root = Path(self.temp.name)
        (self.root / 'assets').mkdir()
        html = '<html><script>const hello="World";</script></html>'
        packed = gzip.compress(html.encode(), mtime=0)
        sha = hashlib.sha256(packed).hexdigest()
        url = 'https://raw.githubusercontent.com/kmsha19/flightboard-updates/main/store/zcs-web-4.1.0.bin'
        self.metadata = {'schema': 1, 'id': '__web', 'version': '4.1.0', 'size': len(packed),
                         'sha256': sha, 'url': url, 'sourceSha256': hashlib.sha256(html.encode()).hexdigest(),
                         'minifiedSha256': hashlib.sha256(html.encode()).hexdigest()}
        self.write('assets/zcs-web.bin', packed)
        self.write('assets/zcs-web.json', json.dumps(self.metadata))
        self.write('FlightBoardWeb.h', 'const char* html=R"FBPAGE('+html+')FBPAGE";')
        self.write('FlightBoardWebGzip.h', f'constexpr uint32_t ZCS_WEB_SIZE = {len(packed)};\nconst char ZCS_WEB_SHA256[] PROGMEM = "{sha}";\nconst char ZCS_WEB_URL[] PROGMEM = "{url}";\n')
        template = '<p>__WEB_VERSION__ __WEB_SIZE__ __WEB_SHA256__ __WEB_URL__</p>'
        self.write('assets/bootstrap.html', template)
        rendered = template.replace('__WEB_VERSION__', '4.1.0').replace('__WEB_SIZE__', str(len(packed))).replace('__WEB_SHA256__', sha).replace('__WEB_URL__', url)
        self.write('ZcsBootstrap.h', 'const uint8_t ZCS_BOOTSTRAP_GZIP[] = {' + ','.join(f'0x{n:02X}' for n in gzip.compress(rendered.encode(), mtime=0)) + '};')

    def tearDown(self): self.temp.cleanup()
    def write(self, name, value): (self.root / name).write_bytes(value.encode() if isinstance(value,str) else value)
    def test_content_addressed_pair(self):
        version='4.1.3';meta=self.metadata.copy();meta['version']=version
        meta['url']='https://raw.githubusercontent.com/kmsha19/flightboard-updates/main/store/zcs-web-'+meta['sha256']+'.bin'
        self.write('assets/zcs-web.json',json.dumps(meta))
        header=f'constexpr uint32_t ZCS_WEB_SIZE = {meta["size"]};\nconst char ZCS_WEB_SHA256[] PROGMEM = "{meta["sha256"]}";\nconst char ZCS_WEB_URL[] PROGMEM = "{meta["url"]}";\nconst char ZCS_WEB_BINDING[] PROGMEM = "ZCSWEB1:{meta["size"]}:{meta["sha256"]}:";\n'
        self.write('FlightBoardWebGzip.h',header)
        template=(self.root/'assets/bootstrap.html').read_text()
        for key,value in {'VERSION':version,'SIZE':str(meta['size']),'SHA256':meta['sha256'],'URL':meta['url']}.items():template=template.replace('__WEB_'+key+'__',value)
        self.write('ZcsBootstrap.h','const uint8_t ZCS_BOOTSTRAP_GZIP[] = {'+','.join(f'0x{n:02X}' for n in gzip.compress(template.encode(),mtime=0))+'};')
        release.validate_web_assets(self.root,version)
        self.write('FlightBoardWebGzip.h',header.replace('ZCSWEB1:','WRONG:'))
        with self.assertRaises(ValueError):release.validate_web_assets(self.root,version)

    def check(self): release.validate_web_assets(self.root, '4.1.0')

    def test_valid_complete_public_assets(self): self.check()
    def test_wrong_version(self):
        with self.assertRaises(ValueError): release.validate_web_assets(self.root,'4.1.1')
    def test_metadata_changes(self):
        for key in self.metadata:
            with self.subTest(key=key):
                changed = dict(self.metadata);changed[key] = 999 if type(changed[key]) is int else 'different'
                self.write('assets/zcs-web.json',json.dumps(changed))
                with self.assertRaises(ValueError): self.check()
    def test_missing_assets(self):
        for name in ['assets/zcs-web.bin','assets/zcs-web.json','assets/bootstrap.html','FlightBoardWeb.h','ZcsBootstrap.h']:
            with self.subTest(name=name):
                path=self.root/name;data=path.read_bytes();path.unlink()
                with self.assertRaises(ValueError): self.check()
                path.write_bytes(data)
    def test_changed_compiled_fingerprint(self):
        path=self.root/'FlightBoardWebGzip.h';original=path.read_text()
        for a,b in [('ZCS_WEB_SIZE = ', 'ZCS_WEB_SIZE = 9'),(self.metadata['sha256'],'0'*64),('zcs-web-4.1.0.bin','zcs-web-4.2.0.bin')]:
            with self.subTest(a=a):
                path.write_text(original.replace(a,b))
                with self.assertRaises(ValueError): self.check()
    def test_stale_source_or_bootstrap(self):
        for name in ['FlightBoardWeb.h','assets/bootstrap.html']:
            path=self.root/name;old=path.read_text()
            path.write_text(old.replace('<', '<!-- changed -->',1))
            with self.assertRaises(ValueError): self.check()
            path.write_text(old)
    def test_different_web_bytes(self):
        self.write('assets/zcs-web.bin',gzip.compress(b'different valid gzip',mtime=0))
        with self.assertRaises(ValueError): self.check()
    def test_different_embedded_bootstrap(self):
        self.write('ZcsBootstrap.h',','.join(f'0x{n:02X}' for n in gzip.compress(b'wrong rescue page',mtime=0)))
        with self.assertRaises(ValueError): self.check()
    def test_symlink_asset(self):
        source=self.root/'assets/zcs-web.bin';saved=self.root/'saved.bin';source.rename(saved);source.symlink_to(saved)
        with self.assertRaises(ValueError): self.check()
    def test_legacy_releases_still_supported(self):
        self.write('FlightBoardWebGzip.h','const uint8_t FLIGHT_BOARD_HTML_GZIP[]={};')
        (self.root/'assets/zcs-web.bin').unlink();self.check()

if __name__=='__main__': unittest.main()
