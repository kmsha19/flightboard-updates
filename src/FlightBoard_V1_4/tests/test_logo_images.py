#!/usr/bin/env python3
"""Decode embedded tablet images and compare every RGBA pixel to the PNG assets."""
from io import BytesIO
from pathlib import Path
import re
from PIL import Image

root = Path(__file__).resolve().parent.parent
header = (root / 'AirlineLogos.h').read_text()
images = re.findall(r'const uint8_t IMAGE_([A-Z0-9_]+)\[\] PROGMEM = \{(.*?)\};', header, re.S)
assert len(images) == 37, len(images)
for name, encoded in images:
    raw = bytes(int(value, 16) for value in re.findall(r'0x([A-Fa-f0-9]{2})', encoded))
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
print('PASS all 37 embedded WebP logos: alpha and every visible RGBA pixel equal the source PNG')
