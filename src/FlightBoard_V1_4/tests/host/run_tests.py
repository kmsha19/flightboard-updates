#!/usr/bin/env python3
"""Compile/run real Flight Board logic with host-only peripheral mocks.

Requires Python3, clang++ or g++, and an installed ArduinoJson source tree.
No network requests, credentials, ESP32 writes, or actual peripheral operations.
"""
import argparse
import gzip
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile

parser = argparse.ArgumentParser()
parser.add_argument('project', nargs='?', default='.')
parser.add_argument('--arduino-json', type=Path)
args = parser.parse_args()
project = Path(args.project).resolve()
tests = Path(__file__).resolve().parent
arduino_json = args.arduino_json or project.parent / 'libraries/ArduinoJson/src'
compiler = shutil.which('clang++') or shutil.which('g++')
if not compiler:
    parser.error('clang++ or g++ is required.')
if not (arduino_json / 'ArduinoJson.h').is_file():
    parser.error('ArduinoJson.h not found; use --arduino-json /path/to/ArduinoJson/src')
if not (project / 'FlightBoard_V1_4.ino').is_file():
    parser.error('Pass the Flight Board project directory.')

# The ESP32 serves the generated gzip; stale output would ship an older UI.
html = re.search(r'R"FBPAGE\((.*?)\)FBPAGE"',
    (project / 'FlightBoardWeb.h').read_text(), re.S).group(1).encode()
node = shutil.which('node')
if not node:
    parser.error('Node.js >=18 and npm ci are required to verify the embedded UI build.')
minified = subprocess.run([node, str(project / 'tools/minify_web.cjs')],
                         input=html, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
if minified.returncode:
    parser.error(minified.stderr.decode('utf-8', errors='replace').strip())
compressed = bytes(int(value, 16) for value in re.findall(r'0x([A-F0-9]{2})',
    (project / 'FlightBoardWebGzip.h').read_text()))
if gzip.decompress(compressed) != minified.stdout:
    parser.error('Compressed UI is stale: run python3 build_assets.py first.')
print('PASS compressed UI exactly matches the pinned build of its readable source', flush=True)

with tempfile.TemporaryDirectory(prefix='flightboard-v20-host-') as name:
    build = Path(name)
    subprocess.run([sys.executable, str(tests / 'extract_settings.py'), str(project), '--output', str(build / 'settings_under_test.inc')], check=True)
    # The Wi-Fi guard suite includes the original Wi-Fi scenarios, then tests OTA.
    shutil.copy2(tests / 'test_wifi.cpp', build / 'test_wifi.cpp')
    for test in ('test_features', 'test_radar', 'test_routes', 'test_logos', 'test_settings', 'test_brightness', 'test_zcs_filters', 'test_zcs_weather', 'test_wifi_busy', 'test_update'):
        directory = tests.parent / 'ota' if test == 'test_update' else tests
        source = build / (test + '.cpp')
        shutil.copy2(directory / source.name, source)
        output = build / test
        subprocess.run([compiler, '-std=c++17', '-Wall', '-Wextra', '-Werror',
            '-I' + str(directory / 'mocks'), '-I' + str(project), '-I' + str(arduino_json),
            str(source), '-o', str(output)], check=True)
        subprocess.run([str(output)], check=True)
