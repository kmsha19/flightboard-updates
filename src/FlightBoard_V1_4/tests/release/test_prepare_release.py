#!/usr/bin/env python3
"""Offline release-packaging checks using synthetic public build fixtures."""
import hashlib
import importlib.util
import json
from pathlib import Path
import shutil
import struct
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('prepare_release', ROOT / 'tools/prepare_release.py')
release = importlib.util.module_from_spec(spec)
spec.loader.exec_module(release)


def image(elf=b'Public ELF test fixture', payload=b'\x01\x02\x03\x04', appended=True):
    header = bytearray(24)
    header[0], header[1], header[12], header[23] = 0xe9, 2, 9, int(appended)
    descriptor = bytearray(256)
    descriptor[:4] = b'\x32\x54\xcd\xab'
    descriptor[144:176] = hashlib.sha256(elf).digest()
    descriptor[208:208+len(release.FAMILY)] = release.FAMILY.encode()
    data = header + struct.pack('<II', 0x3c000020, len(descriptor)) + descriptor
    data += struct.pack('<II', 0x40370000, len(payload)) + payload
    checksum = 0xef
    for value in descriptor + payload:
        checksum ^= value
    data += bytes((15-len(data)) % 16) + bytes([checksum])
    if appended:
        data += hashlib.sha256(data).digest()
    return bytes(data)


class ReleaseTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix='flightboard-release-test-')
        self.root = Path(self.temp.name)
        self.sketch = self.root / 'FlightBoard_V1_4'
        self.build = self.root / 'build'
        self.compiled = self.build / 'sketch'
        self.output = self.root / 'distribution'
        self.sketch.mkdir()
        self.compiled.mkdir(parents=True)
        self.binary = self.build / 'FlightBoard_V1_4.ino.bin'
        self.elf = self.binary.with_suffix('.elf')
        self.options = {'sketchLocation': str(self.sketch),
                        'fqbn': 'esp32:esp32:esp32s3:USBMode=hwcdc,CDCOnBoot=cdc',
                        'customBuildProperties': ''}
        self.set_version('2.0.8')
        (self.sketch / 'Other.h').write_text('// public header\n')
        shutil.copy2(self.sketch / 'Other.h', self.compiled / 'Other.h')
        self.add_source('FlightBoardHub75.cpp', b'#define NO_GFX\n#include "assets/vendor/waveshare-hub75/driver.cpp"\n')
        self.add_source('assets/vendor/waveshare-hub75/driver.cpp', b'#include "platforms/esp32s3/dma.hpp"\n')
        self.add_source('assets/vendor/waveshare-hub75/platforms/esp32s3/dma.hpp', b'// public nested header\n')
        self.add_source('assets/vendor/waveshare-hub75/platforms/esp32s3/dma.cpp', b'void drawPixel() {}\n')
        self.add_source('assets/vendor/waveshare-hub75/LICENSE', b'Public fixture licence\n', directive=False)
        self.add_source('assets/vendor/waveshare-hub75/LICENSE-upstream-MIT.txt', b'Public MIT fixture licence\n', directive=False)
        self.add_source('assets/vendor/waveshare-hub75/LICENSE-LovyanGFX-FreeBSD.txt', b'Public FreeBSD fixture licence\n', directive=False)
        self.vendor_manifest = {'repository': 'https://example.com/public', 'commit': 'public-fixture',
                                'files': [{'path': path.name, 'size': (self.sketch / path).stat().st_size,
                                           'sha256': hashlib.sha256((self.sketch / path).read_bytes()).hexdigest()}
                                          for path in sorted(release.WAVESHARE_LICENSES)]}
        self.save_vendor_manifest()
        # Compiler output is not a copied source file; Arduino adds prototypes.
        (self.compiled / 'FlightBoard_V1_4.ino.cpp').write_text('// generated Arduino sketch\n')
        (self.compiled / 'FlightBoardHub75.cpp.o').write_bytes(b'Object fixture')
        (self.compiled / 'FlightBoardHub75.cpp.d').write_text('Object dependency fixture\n')
        self.elf.write_bytes(b'Public ELF test fixture')
        self.binary.write_bytes(image())
        self.save_options()

    def tearDown(self):
        self.temp.cleanup()

    def save_options(self):
        (self.build / 'build.options.json').write_text(json.dumps(self.options))

    def set_version(self, version):
        text = 'void sendSystem(){doc["version"] = "' + version + '";}\n'
        for directory in (self.sketch, self.compiled):
            (directory / 'FlightBoardUpdate.h').write_text(text)

    def add_source(self, relative, content, directive=True):
        source = self.sketch / relative
        copied = self.compiled / relative
        source.parent.mkdir(parents=True, exist_ok=True)
        copied.parent.mkdir(parents=True, exist_ok=True)
        source.write_bytes(content)
        prefix = f'#line 1 "{source.resolve()}"\n'.encode() if directive else b''
        copied.write_bytes(prefix + content)

    def save_vendor_manifest(self):
        self.add_source('assets/vendor/waveshare-hub75/UPSTREAM.json',
                        json.dumps(self.vendor_manifest).encode(), directive=False)

    def package(self, version='2.0.8'):
        return release.package(self.binary, self.sketch, version,
                               'https://updates.example/flightboard/', self.output)

    def snapshot(self):
        return {str(p.relative_to(self.output)): (p.read_bytes(), p.stat().st_mtime_ns)
                for p in self.output.rglob('*') if p.is_file()}

    def test_valid_deterministic_package_and_manifest(self):
        manifest = self.package()
        data = self.binary.read_bytes()
        self.assertEqual(manifest['version'], '2.0.8')
        self.assertEqual(manifest['size'], len(data))
        self.assertEqual(manifest['sha256'], hashlib.sha256(data).hexdigest())
        self.assertEqual(manifest['firmwareFamily'], release.FAMILY)
        self.assertEqual(manifest['url'], 'https://updates.example/flightboard/releases/FlightBoard-2.0.8-ESP32S3.ino.bin')
        saved = self.snapshot()
        self.assertEqual(len(saved), 3)
        self.assertEqual(self.package(), manifest)
        self.assertEqual(self.snapshot(), saved, 'Identical packaging must preserve existing bytes and mtimes')

    def test_new_version_preserves_all_older_binaries(self):
        self.package()
        old = self.snapshot()
        self.set_version('2.0.9')
        self.binary.write_bytes(image(payload=b'\x05\x06\x07\x08'))
        self.package('2.0.9')
        new = self.snapshot()
        self.assertEqual(len(new), 5)
        for name in old:
            if name != 'latest.json':
                self.assertEqual(new[name], old[name])
        self.assertEqual(json.loads(new['latest.json'][0])['version'], '2.0.9')

    def test_existing_version_cannot_be_replaced(self):
        self.package()
        before = self.snapshot()
        self.binary.write_bytes(image(payload=b'\x05\x06\x07\x08'))
        with self.assertRaisesRegex(ValueError, 'replace an existing release'):
            self.package()
        self.assertEqual(self.snapshot(), before)

    def test_all_conflicts_checked_before_any_output_write(self):
        self.output.mkdir()
        folder = self.output / 'releases'
        folder.mkdir()
        checksum = folder / 'FlightBoard-2.0.8-ESP32S3.ino.bin.sha256'
        checksum.write_text('pre-existing different checksum\n')
        before = self.snapshot()
        with self.assertRaisesRegex(ValueError, 'replace an existing release'):
            self.package()
        self.assertEqual(self.snapshot(), before)
        self.assertFalse((folder / 'FlightBoard-2.0.8-ESP32S3.ino.bin').exists())

    def test_secrets_rejected_without_reading_contents(self):
        original = Path.read_bytes
        def guarded(path):
            if path.name.lower() == 'secrets.h':
                raise AssertionError('Secrets content must never be read')
            return original(path)
        for directory in (self.sketch, self.compiled):
            with self.subTest(directory=directory.name):
                secret = directory / 'Secrets.h'
                secret.touch()  # Empty sentinel, never real credentials.
                with patch.object(Path, 'read_bytes', guarded):
                    with self.assertRaisesRegex(ValueError, 'containing Secrets.h'):
                        self.package()
                secret.unlink()
                self.assertFalse(self.output.exists())

    def test_provenance_metadata_headers_version_and_elf(self):
        original = self.options.copy()
        for key, value in [('sketchLocation', str(self.root / 'different-sketch')),
                           ('fqbn', 'esp32:esp32:esp32:'),
                           ('customBuildProperties', 'test.build.flag=1')]:
            with self.subTest(key=key):
                self.options = dict(original, **{key: value})
                self.save_options()
                with self.assertRaises(ValueError): self.package()
                self.assertFalse(self.output.exists())
        self.options = original
        self.save_options()
        (self.compiled / 'Other.h').write_text('// stale compiled source\n')
        with self.assertRaisesRegex(ValueError, 'Compiled source differs'): self.package()
        shutil.copy2(self.sketch / 'Other.h', self.compiled / 'Other.h')
        (self.compiled / 'Extra.h').touch()
        with self.assertRaisesRegex(ValueError, 'source sets differ'): self.package()
        (self.compiled / 'Extra.h').unlink()
        with self.assertRaisesRegex(ValueError, 'source version differs'): self.package('2.0.9')
        self.elf.write_bytes(b'Different public ELF test fixture')
        with self.assertRaisesRegex(ValueError, 'does not match its compiled ELF'): self.package()
        self.assertFalse(self.output.exists())

    def test_arduino_line_directive_must_match_exact_public_source(self):
        for source in self.sketch.glob('*.h'):
            directive = f'#line 1 "{source.resolve()}"\n'.encode()
            (self.compiled / source.name).write_bytes(directive + source.read_bytes())
        self.package()
        before = self.snapshot()
        source = self.sketch / 'Other.h'
        (self.compiled / source.name).write_bytes(
            b'#line 1 "/different/private/Other.h"\n' + source.read_bytes())
        with self.assertRaisesRegex(ValueError, 'Compiled source differs'):
            self.package()
        self.assertEqual(self.snapshot(), before)

    def test_wrapper_and_nested_vendor_sources_must_match(self):
        self.package()
        before = self.snapshot()
        targets = ['FlightBoardHub75.cpp',
                   'assets/vendor/waveshare-hub75/driver.cpp',
                   'assets/vendor/waveshare-hub75/platforms/esp32s3/dma.cpp',
                   'assets/vendor/waveshare-hub75/platforms/esp32s3/dma.hpp',
                   'assets/vendor/waveshare-hub75/UPSTREAM.json']
        for relative in targets:
            with self.subTest(path=relative):
                copied = self.compiled / relative
                original = copied.read_bytes()
                copied.write_bytes(original + b'// stale source\n')
                with self.assertRaisesRegex(ValueError, 'Compiled source differs'):
                    self.package()
                self.assertEqual(self.snapshot(), before)
                copied.unlink()
                with self.assertRaisesRegex(ValueError, 'source sets differ'):
                    self.package()
                self.assertEqual(self.snapshot(), before)
                copied.write_bytes(original)
        for relative in ['Unexpected.ino.cpp', 'assets/vendor/waveshare-hub75/extra.cpp']:
            extra = self.compiled / relative
            extra.write_text('// stale extra source\n')
            with self.assertRaisesRegex(ValueError, 'source sets differ'):
                self.package()
            self.assertEqual(self.snapshot(), before)
            extra.unlink()

    def test_nested_cpp_prefix_and_vendor_metadata_are_strict(self):
        relative = Path('assets/vendor/waveshare-hub75/platforms/esp32s3/dma.cpp')
        source, copied = self.sketch / relative, self.compiled / relative
        original = copied.read_bytes()
        # Unprefixed copies and the exact Arduino prefix are both valid.
        copied.write_bytes(source.read_bytes())
        self.package()
        before = self.snapshot()
        copied.write_bytes(b'#line 1 "/different/private/dma.cpp"\n' + source.read_bytes())
        with self.assertRaisesRegex(ValueError, 'Compiled source differs'):
            self.package()
        self.assertEqual(self.snapshot(), before)
        copied.write_bytes(original)
        for name in ['LICENSE', 'LICENSE-upstream-MIT.txt', 'LICENSE-LovyanGFX-FreeBSD.txt']:
            source = self.sketch / 'assets/vendor/waveshare-hub75' / name
            copied = self.compiled / source.relative_to(self.sketch)
            copied.write_bytes(f'#line 1 "{source.resolve()}"\n'.encode() + source.read_bytes())
            with self.assertRaisesRegex(ValueError, 'Compiled source differs'):
                self.package()
            self.assertEqual(self.snapshot(), before)
            copied.write_bytes(source.read_bytes())

    def test_nested_secret_is_rejected_before_source_contents_are_read(self):
        for directory in (self.sketch, self.compiled):
            secret = directory / 'assets/vendor/waveshare-hub75/platforms/Secrets.h'
            secret.touch()  # Empty sentinel; never real credentials.
            with patch.object(Path, 'read_bytes', side_effect=AssertionError('Must reject by filename first')):
                with self.assertRaisesRegex(ValueError, 'containing Secrets.h'):
                    self.package()
            secret.unlink()
            self.assertFalse(self.output.exists())

    def test_arduino_omitted_licences_are_bound_to_copied_manifest(self):
        for relative in release.WAVESHARE_LICENSES:
            (self.compiled / relative).unlink()
        manifest = self.sketch / release.WAVESHARE_DIRECTORY / 'UPSTREAM.json'
        copied_manifest = self.compiled / release.WAVESHARE_DIRECTORY / 'UPSTREAM.json'
        prefix = f'#line 1 "{manifest.resolve()}"\n'.encode()
        copied_manifest.write_bytes(prefix + manifest.read_bytes())
        self.package()
        before = self.snapshot()
        copied_manifest.write_bytes(b'#line 1 "/different/private/UPSTREAM.json"\n' + manifest.read_bytes())
        with self.assertRaisesRegex(ValueError, 'Compiled source differs'):
            self.package()
        self.assertEqual(self.snapshot(), before)
        copied_manifest.write_bytes(prefix + manifest.read_bytes())
        for relative in release.WAVESHARE_LICENSES:
            source = self.sketch / relative
            original = source.read_bytes()
            source.write_bytes(original + b'changed')
            with self.assertRaisesRegex(ValueError, 'licence differs from its compiled manifest'):
                self.package()
            self.assertEqual(self.snapshot(), before)
            source.unlink()
            with self.assertRaisesRegex(ValueError, 'missing a required licence'):
                self.package()
            self.assertEqual(self.snapshot(), before)
            source.write_bytes(original)
        # A licence-like name cannot excuse a missing compiled C++ input.
        self.add_source('assets/vendor/waveshare-hub75/LICENSE.cpp', b'void driver() {}\n')
        (self.compiled / 'assets/vendor/waveshare-hub75/LICENSE.cpp').unlink()
        with self.assertRaisesRegex(ValueError, 'source sets differ'):
            self.package()
        self.assertEqual(self.snapshot(), before)

    def test_licence_manifest_requires_exact_unique_safe_paths_and_fingerprints(self):
        original = json.loads(json.dumps(self.vendor_manifest))
        mutations = []
        for field, value in [('path', '../LICENSE'), ('path', '/LICENSE'),
                             ('path', './LICENSE'), ('path', 'nested/../LICENSE'),
                             ('path', 'nested\\LICENSE'), ('path', 'LICENSE.cpp'),
                             ('size', -1), ('size', True), ('size', 9999),
                             ('sha256', '0' * 64), ('sha256', 'invalid')]:
            changed = json.loads(json.dumps(original))
            changed['files'][0][field] = value
            mutations.append(changed)
        duplicate = json.loads(json.dumps(original))
        duplicate['files'].append(duplicate['files'][0].copy())
        mutations.extend([duplicate, {'files': []}, {'files': {}}, {'files': [None]}])
        for manifest in mutations:
            with self.subTest(manifest=manifest):
                self.vendor_manifest = manifest
                self.save_vendor_manifest()
                with self.assertRaises(ValueError): self.package()
                self.assertFalse(self.output.exists())
        self.vendor_manifest = original
        self.save_vendor_manifest()
        # Different upstream origins in individual entries are preserved and allowed.
        for index, entry in enumerate(self.vendor_manifest['files']):
            entry.update(repository=f'https://example.com/origin-{index}', commit=str(index) * 40)
        self.save_vendor_manifest()
        self.package()

    def test_symlinks_and_unrelated_output_files_are_rejected(self):
        (self.sketch / 'linked.h').symlink_to(self.compiled / 'Other.h')
        with self.assertRaisesRegex(ValueError, 'symlinks'): self.package()
        (self.sketch / 'linked.h').unlink()
        self.output.mkdir()
        (self.output / 'unexpected.txt').write_text('Do not publish this file')
        before = self.snapshot()
        with self.assertRaisesRegex(ValueError, 'Unexpected file'): self.package()
        self.assertEqual(self.snapshot(), before)
        (self.output / 'unexpected.txt').unlink()
        (self.output / 'releases').symlink_to(self.build, target_is_directory=True)
        with self.assertRaisesRegex(ValueError, 'Unexpected file'): self.package()

    def test_complete_esp_image_layout_checksum_and_hash(self):
        good = image()
        release.validate_image(good)
        release.validate_image(image(appended=False))
        mutations = [good[:-16], good[:-1], good+b'extra', bytes(1310721)]
        for offset, value in [(0, 0), (1, 0), (1, 17), (12, 0), (23, 2),
                              (32, 0), (50, 1), (len(good)-1, good[-1]^1)]:
            changed = bytearray(good)
            changed[offset] = value
            mutations.append(bytes(changed))
        too_long = bytearray(good)
        struct.pack_into('<I', too_long, 28, len(good)+1)
        mutations.append(bytes(too_long))
        for data in mutations:
            with self.subTest(length=len(data), prefix=data[:2]):
                with self.assertRaises(ValueError): release.validate_image(data)
        # Corrupt zero padding, repairing the hash so padding validation is decisive.
        padded_end = len(good)-32
        padding = bytearray(good)
        padding[padded_end-2] = 1
        padding[padded_end:] = hashlib.sha256(padding[:padded_end]).digest()
        with self.assertRaisesRegex(ValueError, 'padding or checksum'): release.validate_image(bytes(padding))
        self.binary.write_bytes(good[:-16])
        with self.assertRaises(ValueError): self.package()
        self.assertFalse(self.output.exists())

    def test_invalid_version_and_url_cannot_create_output(self):
        for version in ('2.0', '02.0.8', '2.0.8-beta', '../2.0.8'):
            with self.assertRaises(ValueError): self.package(version)
        for url in ('http://example.com/', 'https://user:secret@example.com/',
                    'https://example.com/no-slash', 'https://example.com/?token=x',
                    'https://example.com/#fragment', 'https://example.com:abc/'):
            with self.assertRaises(ValueError):
                release.package(self.binary, self.sketch, '2.0.8', url, self.output)
        self.assertFalse(self.output.exists())


if __name__ == '__main__':
    unittest.main()
