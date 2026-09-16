"""Check the public package, metadata, and local documentation links."""
import json
from pathlib import Path
import re
from urllib.parse import unquote

from package_release import ROOT, inspect_nro
from release_assets import asset_inventory
from nro_branding import check_branding


def main():
    manifest = json.loads((ROOT / 'release/manifest.json').read_text())
    version = json.loads((ROOT / 'version.json').read_text())
    path = ROOT / 'release' / manifest['file']
    assert path.resolve().is_relative_to((ROOT / 'release').resolve())
    actual = inspect_nro(path)
    for field in ('size', 'sha256', 'romfs_size'):
        assert manifest[field] == actual[field], f'Manifest mismatch: {field}'
    assert manifest['version'] == version['version']
    icon = check_branding(path, ROOT / 'branding/icon.jpg', version)
    assert manifest['icon'] == {'file': 'branding/icon.jpg', **icon}
    assert manifest['validation'] == json.loads((ROOT / 'validation.json').read_text())
    assert not manifest['includes_rom'] and manifest['includes_extracted_asset_cache']
    files = [{'file': manifest['file'], **actual}] + asset_inventory(ROOT / 'release/switch/tmc/assets')
    assert manifest['files'] == files, 'Asset inventory or hashes mismatch'
    checksums = (ROOT / 'release/SHA256SUMS.txt').read_text().strip()
    assert checksums == '\n'.join(f"{item['sha256']}  {item['file']}" for item in files)
    allowed = {'README.md', 'manifest.json', 'SHA256SUMS.txt'} | {item['file'] for item in files}
    for file in (ROOT / 'release').rglob('*'):
        if file.is_file():
            assert file.relative_to(ROOT / 'release').as_posix() in allowed, f'Unexpected release file: {file}'
    documents = list(ROOT.glob('*.md')) + list((ROOT / 'docs').glob('*.md')) + [ROOT / 'release/README.md']
    count = 0
    for document in documents:
        content = re.sub(r'```.*?```', '', document.read_text(encoding='utf-8'), flags=re.S)
        for href in re.findall(r'\]\(([^)]+)\)', content):
            if href.startswith(('https://', 'http://', '#', 'mailto:')):
                continue
            target = unquote(href.split('#', 1)[0])
            assert (document.parent / target).exists(), f'Broken link in {document.name}: {href}'
            count += 1
    print(f'PASS: NRO + {len(files) - 1} supplied assets; exact allowlist and SHA-256 match; no ROM file; {count} local documentation links resolve.')


if __name__ == '__main__':
    main()
