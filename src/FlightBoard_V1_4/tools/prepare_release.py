#!/usr/bin/env python3
"""Package a verified public Arduino application and HTTPS manifest (no upload).

Build metadata and mirrored sources guard against accidentally selecting a private
or stale build. They are provenance checks, not a publisher signature.
"""
import argparse
import hashlib
import gzip
import json
import os
from pathlib import Path
import re
import struct
import tempfile
from urllib.parse import urlsplit

FAMILY = 'FlightBoard.ESP32S3.Application.v2'
MAX_FIRMWARE_BYTES = 1310720
VERSION = r'(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)'
PUBLIC_BINARY = re.compile(r'FlightBoard-' + VERSION + r'-ESP32S3\.ino\.bin(?:\.sha256)?')
SOURCE_SUFFIXES = {'.h', '.hh', '.hpp', '.hxx', '.c', '.cc', '.cpp', '.cxx',
                   '.c++', '.inc', '.ipp', '.tpp', '.s'}
VENDOR_DIRECTORY = Path('assets/vendor')
WAVESHARE_DIRECTORY = VENDOR_DIRECTORY / 'waveshare-hub75'
WAVESHARE_LICENSES = {WAVESHARE_DIRECTORY / name for name in
                     ('LICENSE', 'LICENSE-upstream-MIT.txt', 'LICENSE-LovyanGFX-FreeBSD.txt')}


def regular_file(path):
    if path.is_symlink() or not path.is_file():
        raise ValueError(f'Expected a regular, non-symlink file: {path.name}')


def public_sources(directory, generated_ino=None):
    if not directory.is_dir() or directory.is_symlink():
        raise ValueError('Expected a regular isolated sketch directory')
    entries = list(directory.rglob('*'))
    # Detect by filename before reading any content, including broken symlinks.
    if any(path.name.lower() == 'secrets.h' for path in entries):
        raise ValueError('Refusing a source or compiled sketch containing Secrets.h')
    if any(path.is_symlink() for path in entries):
        raise ValueError('Public sketch sources must not contain symlinks')
    result = {}
    for path in entries:
        relative = path.relative_to(directory)
        # Arduino synthesizes this one .cpp from the sketch, adding prototypes.
        # Other root and nested C++ files are copied inputs and must match.
        if not path.is_file() or relative == generated_ino:
            continue
        if path.suffix.lower() in SOURCE_SUFFIXES or VENDOR_DIRECTORY in relative.parents:
            # Keep vendor licence/provenance files too, not just code extensions.
            result[relative] = path
    return result


def validate_vendor_licenses(source_files, compiled_files):
    """Arduino omits these licence files; bind them to its copied manifest."""
    if not any(WAVESHARE_DIRECTORY in path.parents for path in source_files.keys() | compiled_files.keys()):
        return set()
    manifest_path = WAVESHARE_DIRECTORY / 'UPSTREAM.json'
    if not WAVESHARE_LICENSES.issubset(source_files):
        raise ValueError('Public Waveshare stage is missing a required licence')
    if manifest_path not in source_files or manifest_path not in compiled_files:
        raise ValueError('Compiled and public sketch source sets differ: missing vendor manifest')
    original = source_files[manifest_path].read_bytes()
    copied = compiled_files[manifest_path].read_bytes()
    # Arduino also adds its exact source directive to the copied UPSTREAM.json.
    directive = f'#line 1 "{source_files[manifest_path].resolve()}"\n'.encode()
    if copied != original and copied != directive + original:
        raise ValueError(f'Compiled source differs from the public source: {manifest_path}')
    manifest = json.loads(original)
    if not isinstance(manifest, dict) or not isinstance(manifest.get('files'), list):
        raise ValueError('Invalid vendor provenance manifest')
    entries = {}
    for entry in manifest['files']:
        name = entry.get('path') if isinstance(entry, dict) else None
        # Never resolve arbitrary manifest paths or follow them outside the stage.
        if (not isinstance(name, str) or not name or '\\' in name or '\x00' in name or
                Path(name).is_absolute() or '..' in Path(name).parts or
                Path(name).as_posix() != name or name == '.' or name in entries):
            raise ValueError('Invalid or duplicate vendor manifest path')
        entries[name] = entry
    for relative in WAVESHARE_LICENSES:
        entry = entries.get(relative.relative_to(WAVESHARE_DIRECTORY).as_posix(), {})
        size, digest = entry.get('size'), entry.get('sha256')
        if (type(size) is not int or size < 0 or not isinstance(digest, str) or
                not re.fullmatch('[0-9a-f]{64}', digest)):
            raise ValueError(f'Missing or invalid vendor licence fingerprint: {relative.name}')
        content = source_files[relative].read_bytes()
        if len(content) != size or hashlib.sha256(content).hexdigest() != digest:
            raise ValueError(f'Vendor licence differs from its compiled manifest: {relative.name}')
    return WAVESHARE_LICENSES


def validate_sources(sketch, compiled):
    source_files = public_sources(sketch)
    compiled_files = public_sources(compiled, Path(sketch.name + '.ino.cpp'))
    optional_copies = validate_vendor_licenses(source_files, compiled_files)
    missing = source_files.keys() - compiled_files.keys()
    if not source_files or missing - optional_copies or compiled_files.keys() - source_files.keys():
        raise ValueError('Compiled and public sketch source sets differ; rebuild the public stage')
    for relative, source in source_files.items():
        if relative not in compiled_files:
            continue  # Only the exact, fingerprint-verified licences can be absent.
        original = source.read_bytes()
        copied = compiled_files[relative].read_bytes()
        # Arduino may prepend its own source-location directive to copied code,
        # including nested vendor .cpp files. Match the exact expected path.
        directive = f'#line 1 "{source.resolve()}"\n'.encode()
        supports_directive = (source.suffix.lower() in SOURCE_SUFFIXES or
                              relative == WAVESHARE_DIRECTORY / 'UPSTREAM.json')
        with_directive = supports_directive and copied == directive + original
        if copied != original and not with_directive:
            raise ValueError(f'Compiled source differs from the public source: {relative}')


def validate_web_assets(sketch, version):
    """Bind the external interface and embedded rescue page to the compiled header."""
    header_path = sketch / 'FlightBoardWebGzip.h'
    if not header_path.exists():
        return  # Legacy releases have no generated web asset header.
    regular_file(header_path)
    header = header_path.read_text()
    if 'ZCS_WEB_SIZE' not in header:
        return  # Legacy releases keep the full interface inside their application.
    required = ['assets/zcs-web.bin', 'assets/zcs-web.json', 'assets/bootstrap.html',
                'FlightBoardWeb.h', 'ZcsBootstrap.h']
    for relative in required:
        regular_file(sketch / relative)
    payload = (sketch / required[0]).read_bytes()
    metadata = json.loads((sketch / required[1]).read_text())
    digest = hashlib.sha256(payload).hexdigest()
    asset_id = digest if tuple(map(int,version.split('.'))) >= (4,1,3) else version
    url = f'https://raw.githubusercontent.com/kmsha19/flightboard-updates/main/store/zcs-web-{asset_id}.bin'
    source = re.search(r'R"FBPAGE\((.*?)\)FBPAGE"', (sketch / 'FlightBoardWeb.h').read_text(), re.S)
    if not source:
        raise ValueError('Missing readable web source')
    digest = hashlib.sha256(payload).hexdigest()
    expected = {'schema': 1, 'id': '__web', 'version': version, 'size': len(payload),
                'sha256': digest, 'url': url,
                'sourceSha256': hashlib.sha256(source.group(1).encode()).hexdigest(),
                'minifiedSha256': hashlib.sha256(gzip.decompress(payload)).hexdigest()}
    if not 0 < len(payload) <= 98304 or metadata != expected:
        raise ValueError('Web interface metadata differs from its source or content')
    if tuple(map(int,version.split('.'))) >= (4,1,3):
        marker = f'ZCSWEB1:{len(payload)}:{digest}:'
        if f'ZCS_WEB_BINDING[] PROGMEM = "{marker}"' not in header:
            raise ValueError('Missing firmware/interface binding')
    patterns = {'SIZE': r'constexpr uint32_t ZCS_WEB_SIZE = ([0-9]+);',
                'SHA256': r'const char ZCS_WEB_SHA256\[\] PROGMEM = "([^"]+)";',
                'URL': r'const char ZCS_WEB_URL\[\] PROGMEM = "([^"]+)";'}
    for key, value in {'SIZE': str(len(payload)), 'SHA256': digest, 'URL': url}.items():
        if re.findall(patterns[key], header) != [value]:
            raise ValueError('Web content differs from the compiled fingerprint')
    template = (sketch / 'assets/bootstrap.html').read_text()
    for key, value in {'VERSION': version, 'SIZE': str(len(payload)), 'SHA256': digest, 'URL': url}.items():
        template = template.replace('__WEB_' + key + '__', value)
    rescue_header = (sketch / 'ZcsBootstrap.h').read_text()
    rescue = bytes(int(value, 16) for value in re.findall(r'0x([A-F0-9]{2})', rescue_header))
    if len(rescue) > 4096 or gzip.decompress(rescue) != template.encode():
        raise ValueError('Embedded rescue interface differs from its source or content fingerprint')


def validate_build(binary, sketch, version):
    binary, sketch = Path(binary), Path(sketch)
    regular_file(binary)
    build = binary.parent
    validate_sources(sketch, build / 'sketch')
    validate_web_assets(sketch, version)
    options_path = build / 'build.options.json'
    regular_file(options_path)
    options = json.loads(options_path.read_text())
    if Path(options.get('sketchLocation', '')).resolve() != sketch.resolve():
        raise ValueError('Build metadata does not belong to the supplied public sketch')
    if not str(options.get('fqbn', '')).startswith('esp32:esp32:esp32s3:'):
        raise ValueError('Build target must be ESP32-S3')
    if options.get('customBuildProperties', ''):
        raise ValueError('Public builds must not inject custom compiler properties')
    if binary.name != sketch.name + '.ino.bin':
        raise ValueError('Use the original application binary from its Arduino build directory')
    update = sketch / 'FlightBoardUpdate.h'
    regular_file(update)
    versions = re.findall(r'doc\["version"\]\s*=\s*"([^"\n]+)"\s*;', update.read_text())
    if versions != [version]:
        raise ValueError('The compiled application source version differs from the release version')
    firmware = binary.read_bytes()
    validate_image(firmware)
    # esp_app_desc_t embeds the ELF SHA-256 at offset 144 in the first segment.
    # Tie the selected .bin to its sibling compiler output, not just its filename.
    elf = binary.with_suffix('.elf')
    regular_file(elf)
    if firmware[176:208] != hashlib.sha256(elf.read_bytes()).digest():
        raise ValueError('Application binary does not match its compiled ELF; rebuild the public stage')
    return firmware


def validate_image(firmware):
    # Espressif's documented ESP32 image: 24-byte header, segments, zero padding,
    # XOR checksum and optional appended SHA-256. No bootloader/merged/signature.
    # https://docs.espressif.com/projects/esptool/en/latest/esp32/advanced-topics/firmware-image-format.html
    if not 288 < len(firmware) <= MAX_FIRMWARE_BYTES or firmware[0] != 0xe9:
        raise ValueError('Invalid or oversized application image')
    count, appended = firmware[1], firmware[23]
    if not 1 <= count <= 16 or firmware[12:14] != b'\x09\x00' or appended not in (0, 1):
        raise ValueError('Expected an ESP32-S3 application image')
    offset, checksum = 24, 0xef
    for index in range(count):
        if offset + 8 > len(firmware):
            raise ValueError('Truncated image segment header')
        _, size = struct.unpack_from('<II', firmware, offset)
        offset += 8
        if size % 4 or size > len(firmware) - offset:
            raise ValueError('Invalid or truncated image segment')
        if index == 0 and (size < 256 or firmware[offset:offset+4] != b'\x32\x54\xcd\xab'):
            raise ValueError('Expected an application descriptor, not a bootloader')
        for value in firmware[offset:offset+size]:
            checksum ^= value
        offset += size
    padded_end = (offset + 16) & ~15
    if len(firmware) != padded_end + 32 * appended:
        raise ValueError('Image is truncated or contains trailing/merged data')
    if any(firmware[offset:padded_end-1]) or firmware[padded_end-1] != checksum:
        raise ValueError('Application image padding or checksum is invalid')
    if appended and hashlib.sha256(firmware[:padded_end]).digest() != firmware[padded_end:]:
        raise ValueError('Application image SHA-256 is invalid')
    if FAMILY.encode() not in firmware:
        raise ValueError('Image does not belong to Flight Board V2')


def preflight_output(output, immutable):
    if output.is_symlink() or (output.exists() and not output.is_dir()):
        raise ValueError('Output must be a regular public distribution directory')
    if output.exists():
        for path in output.rglob('*'):
            relative = path.relative_to(output)
            allowed = (relative == Path('releases') and path.is_dir()) or (
                path.is_file() and (relative == Path('latest.json') or
                len(relative.parts) == 2 and relative.parts[0] == 'releases' and
                PUBLIC_BINARY.fullmatch(relative.name)))
            if path.is_symlink() or not allowed:
                raise ValueError(f'Unexpected file in public output: {relative}')
    # Check every immutable destination before creating directories or files.
    for relative, content in immutable.items():
        target = output / relative
        if target.exists() and target.read_bytes() != content:
            raise ValueError(f'Refusing to replace an existing release: {relative}')


def package(binary, sketch, version, base_url, output):
    if not re.fullmatch(VERSION, version):
        raise ValueError('A stable numeric version is required')
    url = urlsplit(base_url)
    if (url.scheme != 'https' or not url.hostname or url.username or url.password or
            url.query or url.fragment or not url.path.endswith('/')):
        raise ValueError('base-url must be an HTTPS directory without credentials, query or fragment')
    _ = url.port  # Reject malformed ports before preparing output.
    firmware = validate_build(Path(binary), Path(sketch), version)
    filename = f'FlightBoard-{version}-ESP32S3.ino.bin'
    relative = Path('releases') / filename
    digest = hashlib.sha256(firmware).hexdigest()
    immutable = {relative: firmware,
                 Path(str(relative) + '.sha256'): f'{digest}  {filename}\n'.encode()}
    output = Path(output)
    preflight_output(output, immutable)
    manifest = {'schema': 1, 'version': version, 'firmwareFamily': FAMILY,
        'url': base_url + relative.as_posix(), 'size': len(firmware), 'sha256': digest,
        'notes': 'Le radar retrouve son rythme après redémarrage. Recherche et installation des mises à jour depuis la tablette.'}
    manifest_bytes = (json.dumps(manifest, ensure_ascii=False, indent=2) + '\n').encode()
    (output / 'releases').mkdir(parents=True, exist_ok=True)
    for destination, content in immutable.items():
        target = output / destination
        if target.exists():
            continue  # Identical existing versions retain both bytes and mtime.
        with target.open('xb') as stream:
            stream.write(content)
    latest = output / 'latest.json'
    if not latest.exists() or latest.read_bytes() != manifest_bytes:
        temporary = None
        try:
            with tempfile.NamedTemporaryFile(dir=output, prefix='.latest-', delete=False) as stream:
                temporary = Path(stream.name)
                stream.write(manifest_bytes)
            os.replace(temporary, latest)
        finally:
            if temporary and temporary.exists():
                temporary.unlink()
    return manifest


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('binary', type=Path)
    parser.add_argument('--sketch', required=True, type=Path, help='Isolated public sketch used for this build')
    parser.add_argument('--version', required=True)
    parser.add_argument('--base-url', default='https://raw.githubusercontent.com/kmsha19/flightboard-updates/main/')
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    try:
        manifest = package(args.binary, args.sketch, args.version, args.base_url, args.output)
    except (ValueError, OSError, KeyError, TypeError) as error:
        parser.error(str(error))
    print(f'Public package ready: {args.output} ({manifest["size"]} bytes, SHA-256 {manifest["sha256"]})')


if __name__ == '__main__':
    main()
