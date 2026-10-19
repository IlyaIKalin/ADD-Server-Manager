#!/usr/bin/env python3
"""Extract reference material from the verified upstream v0.9.91 assets; does not run them."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import re
import struct

HASHES = {
    'ADDServerManager.exe': '30b210ec80e5733e84ebd05a0cc694eaafedc53289221a0765605113639533c0',
    'ADD_Server_Manager.pbo': '212d5b3607d3ca0b7e6b15e9ddff1997e1ca631be627a54b85c6c5b65b207ff8',
}


def write_json(path, value):
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')


def extract_pbo(data, output):
    position = 0
    properties, entries = {}, []

    def string():
        nonlocal position
        end = data.index(b'\0', position)
        value = data[position:end].decode('utf-8')
        position = end + 1
        return value

    while True:
        name = string()
        method, original, reserved, timestamp, size = struct.unpack_from('<5I', data, position)
        position += 20
        if not name and method == 0x56657273:
            while True:
                key = string()
                if not key:
                    break
                properties[key] = string()
            continue
        if not name:
            break
        if method != 0:
            raise ValueError('Compressed PBO entry is not supported')
        parts = name.replace('\\', '/').split('/')
        if any(part in ('', '.', '..') for part in parts) or ':' in name:
            raise ValueError('Invalid PBO path')
        entries.append({'path': '/'.join(parts), 'size': size, 'original': original})
    for entry in entries:
        content = data[position:position + entry['size']]
        if len(content) != entry['size']:
            raise ValueError('Truncated PBO')
        position += entry['size']
        path = output / 'game-mod' / entry['path']
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(content)
        entry['sha256'] = hashlib.sha256(content).hexdigest()
    write_json(output / 'pbo-manifest.json', {'properties': properties, 'entries': entries})
    return len(entries)


def extract_exe(data, output):
    pe = struct.unpack_from('<I', data, 60)[0]
    if data[:2] != b'MZ' or data[pe:pe + 4] != b'PE\0\0':
        raise ValueError('Expected Windows PE')
    count = struct.unpack_from('<H', data, pe + 6)[0]
    optional = pe + 24
    optional_size = struct.unpack_from('<H', data, pe + 20)[0]
    image_base = struct.unpack_from('<Q', data, optional + 24)[0]
    sections = []
    for i in range(count):
        virtual_size, address, size, pointer = struct.unpack_from('<IIII', data, optional + optional_size + 40 * i + 8)
        sections.append((address, size, pointer))

    def address(offset):
        for va, size, pointer in sections:
            if pointer <= offset < pointer + size:
                return image_base + va + offset - pointer
        raise ValueError('Invalid file offset')

    def offset(address):
        for va, size, pointer in sections:
            if image_base + va <= address < image_base + va + size:
                return pointer + address - image_base - va
        raise ValueError('Invalid virtual address')

    def varint(position):
        value = shift = 0
        for _ in range(10):
            byte = data[position]
            position += 1
            value |= (byte & 127) << shift
            if byte < 128:
                return value, position
            shift += 7
        raise ValueError('Invalid varint')

    def name(position):
        flags = data[position]
        size, current = varint(position + 1)
        text = data[current:current + size].decode('utf-8')
        current += size
        tag = ''
        if flags & 2:
            size, current = varint(current)
            tag = data[current:current + size].decode('utf-8')
        return text, tag

    position = data.index(b'\xff Go buildinf:') + 32
    size, position = varint(position)
    version = data[position:position + size].decode()
    position += size
    size, position = varint(position)
    info = data[position:position + size][16:-16].decode()
    (output / 'go-build-info.txt').write_text(version + '\n' + info, encoding='utf-8')
    paths = sorted(set(m.group().decode() for m in re.finditer(rb'addsm/[A-Za-z0-9_./-]+\.go', data)))
    (output / 'go-source-paths.txt').write_text('\n'.join(paths) + '\n', encoding='utf-8')

    files = {}
    for match in re.finditer(rb'dist/assets/[A-Za-z0-9_-]+\.(?:js|css)', data):
        filename = match.group().decode()
        reference = struct.pack('<QQ', address(match.start()), len(filename))
        for ref in re.finditer(re.escape(reference), data):
            try:
                pointer, size = struct.unpack_from('<QQ', data, ref.start() + 16)
                start = offset(pointer)
                if not 0 < size < 10_000_000 or start + size > len(data):
                    continue
                content = data[start:start + size]
                if filename.endswith('.js') and not content.startswith((b'import', b'var ', b'function', b'const', b'let', b'/**', b'//', b'export')):
                    continue
                relative = filename.removeprefix('dist/')
                path = output / 'web-assets' / relative
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(content)
                files[relative] = {'size': size, 'sha256': hashlib.sha256(content).hexdigest()}
            except (ValueError, struct.error):
                continue
    write_json(output / 'web-manifest.json', files)

    # Recover Go struct names, fields and tags, not executable Go source or defaults.
    names = [m.start() - 2 for m in re.finditer(rb'\*(?:settings\.[A-Za-z]+|config.Config)(?![A-Za-z])', data)]
    candidates = []
    for match in re.finditer(re.escape(struct.pack('<QQ', 8, 8)), data):
        start = match.start()
        if start % 8 or data[start + 23] & 31 != 22:
            continue
        relative = struct.unpack_from('<i', data, start + 40)[0]
        if relative > 0:
            candidates.append((start, relative))
    bases = Counter(n - relative for _, relative in candidates for n in names if 0 <= n - relative < len(data))
    types_base = bases.most_common(1)[0][0]

    def type_name(position):
        relative = struct.unpack_from('<i', data, position + 40)[0]
        result = name(types_base + relative)[0]
        return result[1:] if data[position + 20] & 2 else result

    types = {}
    for start, relative in candidates:
        try:
            typename = name(types_base + relative)[0]
            if not typename.startswith(('*config.Config', '*config.TLS', '*settings.', '*app.SettingsView')):
                continue
            element = offset(struct.unpack_from('<Q', data, start + 48)[0])
            if data[element + 23] & 31 != 25:
                continue
            array = offset(struct.unpack_from('<Q', data, element + 56)[0])
            fields = struct.unpack_from('<Q', data, element + 64)[0]
            recovered = []
            for i in range(fields):
                np, tp, field_offset = struct.unpack_from('<QQQ', data, array + 24 * i)
                field, tag = name(offset(np))
                recovered.append({'name': field, 'type': type_name(offset(tp)), 'tag': tag})
            types[typename] = recovered
        except (ValueError, UnicodeError, struct.error):
            continue
    if '*config.Config' not in types or '*settings.All' not in types:
        raise ValueError('Could not recover configuration metadata')
    write_json(output / 'go-types.json', types)
    return len(files)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('assets', type=Path, help='Directory containing the two downloaded release files')
    parser.add_argument('output', type=Path, help='Reference output directory')
    args = parser.parse_args()
    contents = {}
    for filename, expected in HASHES.items():
        content = (args.assets / filename).read_bytes()
        if hashlib.sha256(content).hexdigest() != expected:
            raise ValueError('Unexpected release asset hash: ' + filename)
        contents[filename] = content
    args.output.mkdir(parents=True, exist_ok=True)
    pbo_count = extract_pbo(contents['ADD_Server_Manager.pbo'], args.output)
    web_count = extract_exe(contents['ADDServerManager.exe'], args.output)
    write_json(args.output / 'release-assets.json', {
        'release': 'https://github.com/K-aBa-N/ADD-Server-Manager/releases/tag/v0.9.91',
        'tag_commit': '6fb0459ede231306facfbf10a4cbd11b197a7462',
        'assets': {filename: {'size': len(contents[filename]), 'sha256': digest} for filename, digest in HASHES.items()},
        'method': 'Static extraction only; original executable was not run; backend source was not recovered.',
    })
    print(f'Extracted {pbo_count} PBO entries, {web_count} built web assets and Go metadata.')


if __name__ == '__main__':
    main()
