#!/usr/bin/env python3
"""Stage a reviewed ZCS release in an existing Git clone, without committing/pushing.

Reads only explicitly selected project files; excludes Secrets.h by filename.
Requires the exact verified public Arduino build and a separate public README.
"""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import re
import shutil
import tempfile
import os
import subprocess

SOURCE_EXTENSIONS = {'.h', '.hpp', '.cpp', '.ino'}
DIRECTORIES = ('assets', 'tests', 'tools')
IGNORED_NAMES = {'secrets.h', '.ds_store', '__pycache__', '.git', '.env', '.env.local'}


def selected(project):
    result = []
    for path in project.iterdir():
        if path.name.lower() in IGNORED_NAMES:
            continue
        if path.is_symlink():
            raise ValueError(f'Symlink rejected: {path.name}')
        if path.is_file() and (path.suffix.lower() in SOURCE_EXTENSIONS or path.name in {'build_assets.py', 'package.json', 'package-lock.json'}):
            result.append(path)
    for directory in DIRECTORIES:
        root = project / directory
        for path in root.rglob('*'):
            rel = path.relative_to(project)
            if any(part.lower() in IGNORED_NAMES for part in rel.parts) or path.suffix == '.pyc':
                continue
            if path.is_symlink():
                raise ValueError(f'Symlink rejected: {rel}')
            if path.is_file():
                result.append(path)
    return sorted(result)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--project', type=Path, required=True)
    parser.add_argument('--public-sketch', type=Path, required=True)
    parser.add_argument('--binary', type=Path, required=True)
    parser.add_argument('--repo', type=Path, required=True)
    parser.add_argument('--version', required=True)
    parser.add_argument('--readme', type=Path, required=True)
    parser.add_argument('--notes', required=True)
    parser.add_argument('--preview', type=Path)
    args = parser.parse_args()
    if not re.fullmatch(r'\d+\.\d+\.\d+', args.version):
        raise ValueError('Stable numeric version required')
    project, public, repo = (p.resolve() for p in (args.project, args.public_sketch, args.repo))
    if not (repo / '.git').is_dir():
        raise ValueError('Expected an existing Git checkout')
    if subprocess.check_output(['git', 'status', '--porcelain'], cwd=repo).strip():
        raise ValueError('Publication checkout must be clean before staging')
    source_paths = selected(project)
    spec = importlib.util.spec_from_file_location('prepare_release', project / 'tools/prepare_release.py')
    release = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(release)
    firmware = release.validate_build(args.binary.resolve(), public, args.version)
    # The compiled public stage intentionally excludes tests and generation input.
    # Bind every real firmware header/source and vendor file to that exact stage.
    verified_files = release.public_sources(public)
    for rel, verified in verified_files.items():
        candidate = project / rel
        if candidate not in source_paths or candidate.read_bytes() != verified.read_bytes():
            raise ValueError(f'Current project differs from compiled firmware: {rel}')
    for candidate in source_paths:
        rel = candidate.relative_to(project)
        if len(rel.parts) == 1 and candidate.suffix.lower() in SOURCE_EXTENSIONS and candidate.suffix != '.ino':
            if rel not in verified_files:
                raise ValueError(f'Project source missing from compiled stage: {rel}')
    ino = project / 'FlightBoard_V1_4.ino'
    if ino.read_bytes() != (public / ino.name).read_bytes():
        raise ValueError('Current Arduino sketch differs from compiled firmware')
    source_target = repo / 'src/FlightBoard_V1_4'
    if source_target.is_symlink() or (source_target.exists() and not source_target.is_dir()):
        raise ValueError('Source destination must be a regular directory')
    readme = args.readme.read_text()
    if args.version not in readme:
        raise ValueError('README must describe the release version')
    web_metadata = json.loads((project / 'assets/zcs-web.json').read_text())
    binding = f"ZCSWEB1:{web_metadata['size']}:{web_metadata['sha256']}:".encode()
    if firmware.count(binding) != 1:
        raise ValueError('Compiled firmware is not uniquely bound to the interface being published')
    digest = hashlib.sha256(firmware).hexdigest()
    binary_name = f'ZCS-Control-{args.version}-Waveshare-K716.ino.bin'
    binary_path = repo / 'releases' / binary_name
    if binary_path.exists():
        raise ValueError('Release already exists; immutable versions cannot be replaced')
    release.validate_web_assets(project, args.version)
    extras = {
        Path('store') / Path(json.loads((project / 'assets/zcs-web.json').read_text())['url']).name: (project / 'assets/zcs-web.bin').read_bytes(),
        Path(f'store/zcs-web-{args.version}.json'): (project / 'assets/zcs-web.json').read_bytes(),
        Path('store/zcs-logos.bin'): (project / 'assets/zcs-logos.bin').read_bytes(),
        Path('store/zcs-logos.json'): (project / 'assets/zcs-logos.json').read_bytes(),
    }
    for relative, content in extras.items():
        target = repo / relative
        if target.is_symlink() or (target.exists() and target.read_bytes() != content):
            raise ValueError(f'Refusing to replace an existing store asset: {relative}')
    if args.preview and (args.preview.is_symlink() or not args.preview.is_file() or
                         args.preview.suffix.lower() not in {'.jpg', '.png', '.webp'}):
        raise ValueError('Preview must be a reviewed JPEG, PNG, or WebP')
    # Prepare the complete source mirror before touching the clean Git checkout.
    inventory = []
    with tempfile.TemporaryDirectory(prefix='zcs-publication-', dir=repo.parent) as scratch:
        prepared = Path(scratch) / 'FlightBoard_V1_4'
        for path in source_paths:
            relative = path.relative_to(project)
            target = prepared / relative
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(path, target)
            inventory.append({'path': relative.as_posix(), 'size': target.stat().st_size,
                              'sha256': hashlib.sha256(target.read_bytes()).hexdigest()})
        source_target.parent.mkdir(parents=True, exist_ok=True)
        previous = Path(scratch) / 'previous-source'
        if source_target.exists():
            os.replace(source_target, previous)
        try:
            os.replace(prepared, source_target)
        except BaseException:
            if previous.exists():
                os.replace(previous, source_target)
            raise
    for relative, content in extras.items():
        target = repo / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        if not target.exists():
            target.write_bytes(content)
    (repo / 'README.md').write_text(readme)
    (repo / '.gitignore').write_text('Secrets.h\n**/Secrets.h\n.env\n.env.*\n.DS_Store\n__pycache__/\n*.pyc\nnode_modules/\nbuild/\n.pio/\n')
    binary_path.parent.mkdir(exist_ok=True)
    binary_path.write_bytes(firmware)
    Path(str(binary_path) + '.sha256').write_text(f'{digest}  {binary_name}\n')
    manifest = {'schema': 1, 'version': args.version, 'firmwareFamily': release.FAMILY,
                'url': 'https://raw.githubusercontent.com/kmsha19/flightboard-updates/main/releases/' + binary_name,
                'size': len(firmware), 'sha256': digest, 'notes': args.notes,
                'interface': {key:web_metadata[key] for key in ('size','sha256','url')}}
    (repo / 'latest.json').write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + '\n')
    if args.preview:
        (repo / 'docs').mkdir(exist_ok=True)
        shutil.copyfile(args.preview, repo / 'docs' / ('zcs-control-v4' + args.preview.suffix.lower()))
    inventory_path = repo / 'source-manifest.json'
    inventory_path.write_text(json.dumps({'version': args.version, 'root': 'src/FlightBoard_V1_4',
        'firmwareSha256': digest, 'files': inventory}, ensure_ascii=False, indent=2) + '\n')
    # Git tracks only publication files selected above, never the private project.
    forbidden = [p for p in repo.rglob('*') if '.git' not in p.parts and p.name.lower() == 'secrets.h']
    if forbidden:
        raise ValueError('Secret filename in publication checkout')
    print(json.dumps({'version': args.version, 'sourceFiles': len(inventory),
                      'binaryBytes': len(firmware), 'binarySha256': digest,
                      'repository': str(repo), 'status': 'staged-on-disk-only'}))


if __name__ == '__main__':
    main()
