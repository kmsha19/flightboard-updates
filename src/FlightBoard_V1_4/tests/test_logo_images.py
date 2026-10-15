#!/usr/bin/env python3
"""Decode downloadable WebP pack and compare every RGBA pixel to PNG assets."""
import hashlib
import json
from io import BytesIO
from pathlib import Path
import re
from PIL import Image

root = Path(__file__).resolve().parent.parent
header = (root / 'AirlineLogos.h').read_text()
bundle = (root / 'assets/zcs-logos.bin').read_bytes()
manifest = json.loads((root / 'assets/zcs-logos.json').read_text())
images = manifest['images']
assert len(images) == 37, len(images)
assert len(bundle) == manifest['size'] == int(re.search(r'AIRLINE_IMAGES_SIZE = (\d+)', header)[1])
assert hashlib.sha256(bundle).hexdigest() == manifest['sha256'] == re.search(r'AIRLINE_IMAGES_SHA256\[\] PROGMEM = "([a-f0-9]+)"', header)[1]
assert 'const uint8_t IMAGE_' not in header, 'WebP data must not consume firmware space'
cursor = 0
for image in images:
    name, offset, size = image['code'], image['offset'], image['size']
    assert offset == cursor and size > 0 and size <= len(bundle) - offset
    cursor += size
    raw = bundle[offset:offset + size]
    assert re.search(r'LOGO_' + name + r', ' + str(offset) + r', ' + str(size) + r', INDEX_' + name, header), name
    decoded = Image.open(BytesIO(raw))
    assert decoded.format == 'WEBP', name
    actual = decoded.convert('RGBA')
    expected = Image.open(root / 'assets' / 'tablet' / (name + '.png')).convert('RGBA')
    assert actual.size == expected.size == (72, 48), name
    # Fully transparent pixels have no visible RGB. Compare alpha exactly and
    # every color channel where any alpha is present; preserve white emblems.
    actual_bytes, expected_bytes = actual.tobytes(), expected.tobytes()
    for offset in range(0, len(actual_bytes), 4):
        target, source = actual_bytes[offset:offset + 4], expected_bytes[offset:offset + 4]
        assert target[3] == source[3], name
        if source[3]:
            assert target == source, name
    assert actual.getextrema()[3][0] == 0, name
assert cursor == len(bundle)
print('PASS all 37 downloadable WebP logos: verified offsets/SHA-256, alpha and visible RGBA pixels equal source PNG')
