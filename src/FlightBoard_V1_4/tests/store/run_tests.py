#!/usr/bin/env python3
"""Exercise the real raw store with NOR flash failure injection and real SHA-256.

Uses system CommonCrypto on macOS or OpenSSL headers/libcrypto elsewhere.
No hardware or network access. All compilation output is temporary.
"""
from pathlib import Path
import platform
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
compiler = shutil.which('clang++') or shutil.which('g++')
if not compiler:
    raise SystemExit('Install clang++ or g++ to run storage tests')
with tempfile.TemporaryDirectory(prefix='zcs-store-test-') as folder:
    binary = Path(folder) / 'test_store'
    command = [compiler, '-std=c++17', '-Os', '-Wall', '-Wextra', '-Werror',
               '-Wno-deprecated-declarations', '-I' + str(root / 'tests/store/mocks'),
               '-I' + str(root), str(root / 'tests/store/test_store.cpp'),
               str(root / 'ZcsContentStore.cpp'), '-o', str(binary)]
    if platform.system() != 'Darwin':
        command += ['-lcrypto']
    subprocess.run(command, check=True)
    subprocess.run([str(binary)], check=True)
