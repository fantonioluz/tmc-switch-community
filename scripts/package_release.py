"""Package the Switch executable and supplied runtime assets; never include a ROM."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess
import tempfile
import zipfile
from release_assets import asset_inventory
from nro_branding import check_branding

ROOT = Path(__file__).resolve().parents[1]


def inspect_nro(path):
    data = path.read_bytes()
    if len(data) < 0x80 or data[0x10:0x14] != b'NRO0':
        raise ValueError('Invalid NRO header')
    executable_size = struct.unpack_from('<I', data, 0x18)[0]
    if executable_size < 0x80 or executable_size + 56 > len(data):
        raise ValueError('Invalid NRO size or missing asset header')
    if data[executable_size:executable_size + 4] != b'ASET':
        raise ValueError('Missing NRO ASET header')
    for offset in (8, 24, 40):
        start, length = struct.unpack_from('<QQ', data, executable_size + offset)
        if length and (start < 56 or executable_size + start + length > len(data)):
            raise ValueError('NRO asset section extends outside the file')
    romfs_offset, romfs_size = struct.unpack_from('<QQ', data, executable_size + 40)
    if romfs_offset or romfs_size:
        raise ValueError('This package expects external assets, not an embedded RomFS')
    return {'size': len(data), 'sha256': hashlib.sha256(data).hexdigest(),
            'romfs_size': romfs_size}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    inputs = parser.add_mutually_exclusive_group()
    inputs.add_argument('--elf', type=Path, default=ROOT / 'source/platforms/switch/tmc_switch_usa.elf')
    inputs.add_argument('--nro', type=Path, help='Repackage an existing NRO without rebuilding')
    args = parser.parse_args()
    version = json.loads((ROOT / 'version.json').read_text())
    if not re.fullmatch(r'\d+\.\d+\.\d+(?:-[A-Za-z0-9.-]+)?', version['version']):
        parser.error('version.json must contain a semantic version')
    icon = ROOT / 'branding/icon.jpg'
    if not icon.is_file():
        parser.error('Missing project icon: branding/icon.jpg')
    assets = asset_inventory(ROOT / 'release/switch/tmc/assets')
    destination = ROOT / 'release/switch/tmc/tmc.nro'
    destination.parent.mkdir(parents=True, exist_ok=True)
    if args.nro:
        metadata = inspect_nro(args.nro)
        check_branding(args.nro, icon, version)
        if args.nro.resolve() != destination.resolve():
            shutil.copyfile(args.nro, destination)
    else:
        elf = args.elf.resolve()
        if not elf.is_file():
            parser.error(f'ELF not found: {elf}')
        for tool in ('elf2nro', 'nacptool'):
            if not shutil.which(tool):
                parser.error(f'{tool} must be on PATH (devkitPro tools)')
        if not os.environ.get('DEVKITPRO'):
            parser.error('Set DEVKITPRO to your devkitPro installation')
        work = ROOT / '.local'
        work.mkdir(exist_ok=True)
        with tempfile.TemporaryDirectory(prefix='package-', dir=work) as temporary:
            tmp = Path(temporary)
            nacp = tmp / 'control.nacp'
            nro = tmp / 'tmc.nro'
            subprocess.run(['nacptool', '--create', version['name'], version['author'], version['version'], str(nacp)], check=True)
            # Runtime assets ship alongside the NRO in switch/tmc/assets/.
            subprocess.run(['elf2nro', str(elf), str(nro), f'--nacp={nacp}', f'--icon={icon}'], check=True)
            metadata = inspect_nro(nro)
            check_branding(nro, icon, version)
            shutil.copyfile(nro, destination)
    icon_metadata = check_branding(destination, icon, version)
    validation = json.loads((ROOT / 'validation.json').read_text(encoding='utf-8'))
    provenance = json.loads((ROOT / 'source/UPSTREAM.json').read_text())
    manifest = {**version, 'file': 'switch/tmc/tmc.nro', **metadata,
                'source_base_commit': provenance['base_commit'],
                'includes_rom': False, 'includes_extracted_asset_cache': True,
                'asset_source': 'Runtime asset files supplied by the maintainer',
                'files': [{'file': 'switch/tmc/tmc.nro', **metadata}] + assets,
                'icon': {'file': 'branding/icon.jpg', **icon_metadata},
                'validation': validation,
                'first_install_hardware_validation': validation['fresh_install']['status']}
    (ROOT / 'release/manifest.json').write_text(json.dumps(manifest, indent=2) + '\n', encoding='utf-8')
    (ROOT / 'release/SHA256SUMS.txt').write_text(''.join(f"{item['sha256']}  {item['file']}\n" for item in manifest['files']), encoding='utf-8')
    dist = ROOT / 'dist'
    dist.mkdir(exist_ok=True)
    archive = dist / f"tmc-switch-community-{version['version']}-usa.zip"
    with zipfile.ZipFile(archive, 'w', compression=zipfile.ZIP_DEFLATED) as package:
        package.write(destination, 'switch/tmc/tmc.nro')
        package.write(icon, 'branding/icon.jpg')
        for item in assets:
            package.write(ROOT / 'release' / item['file'], item['file'])
        for file, name in [('release/README.md', 'README.md'),
                           ('release/SHA256SUMS.txt', 'SHA256SUMS.txt'),
                           ('release/manifest.json', 'manifest.json'),
                           ('CONTRIBUTING.md', 'CONTRIBUTING.md'),
                           ('CREDITS.md', 'CREDITS.md'), ('LICENSE.md', 'LICENSE.md'),
                           ('THIRD_PARTY_NOTICES.md', 'THIRD_PARTY_NOTICES.md')]:
            package.write(ROOT / file, name)
        for name in ('INSTALLATION.md', 'ICON.md', 'ASSETS.md', 'KNOWN_ISSUES.md', 'UPDATING.md', 'HYRULE_FIXES.md', 'NPC_FIXES.md', 'STABILITY.md', 'DEVELOPMENT.md', 'RELEASING.md'):
            package.write(ROOT / 'docs' / name, 'docs/' + name)
        for file in sorted((ROOT / 'LICENSES').rglob('*')):
            if file.is_file():
                package.write(file, file.relative_to(ROOT).as_posix())
    print(json.dumps({'nro': str(destination), 'zip': str(archive), **metadata}, indent=2))


if __name__ == '__main__':
    main()
