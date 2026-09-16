"""Read-only verification of the supported USA cartridge dump (Python 3.10+)."""
import argparse
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def inspect_rom(path):
    with path.open('rb') as stream:
        header = stream.read(0xC0)
        stream.seek(0)
        digest = hashlib.sha1()
        size = 0
        for block in iter(lambda: stream.read(1024 * 1024), b''):
            digest.update(block)
            size += len(block)
    return {'size': size, 'header': header[0xAC:0xB0].decode('ascii', errors='replace'),
            'sha1': digest.hexdigest()}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('rom', type=Path)
    args = parser.parse_args()
    version = json.loads((ROOT / 'version.json').read_text())
    try:
        result = inspect_rom(args.rom)
    except OSError as error:
        parser.exit(1, f'Não foi possível ler a ROM: {error}\n')
    print(json.dumps(result, indent=2))
    if result['header'] != version['rom_header'] or result['sha1'] != version['rom_sha1']:
        parser.exit(1, 'O arquivo não corresponde ao dump USA de referência desta edição.\n')
    print('OK: dump USA corresponde à base de referência. Nenhum arquivo foi alterado.')


if __name__ == '__main__':
    main()
