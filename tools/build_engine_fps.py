#!/usr/bin/env python3
"""Build an unsigned FPS variant without modifying the recovered upstream tree."""
import hashlib
import json
from pathlib import Path
import struct
import zipfile

ROOT = Path(__file__).resolve().parents[1]
VARIANT = ROOT / "variants/engine-fps"


def pack_pbo(files, prefix):
    header = bytearray(b"\0" + struct.pack("<5I", 0x56657273, 0, 0, 0, 0))
    header += b"prefix\0" + prefix.encode() + b"\0\0"
    payload = bytearray()
    for name, data in sorted(files.items()):
        header += name.replace("/", "\\").encode() + b"\0"
        header += struct.pack("<5I", 0, len(data), 0, 0, len(data))
        payload += data
    header += b"\0" + bytes(20)
    content = bytes(header + payload)
    return content + b"\0" + hashlib.sha1(content).digest()


def build():
    reference = ROOT / "reference/v0.9.91"
    manifest = json.loads((reference / "pbo-manifest.json").read_text())
    base = {}
    for entry in manifest["entries"]:
        data = (reference / "game-mod" / entry["path"]).read_bytes()
        if hashlib.sha256(data).hexdigest() != entry["sha256"]:
            raise ValueError(f"Reference changed: {entry['path']}")
        base[entry["path"]] = data
    overlay = VARIANT / "overlay"
    for path in overlay.rglob("*"):
        if path.is_file():
            key = path.relative_to(overlay).as_posix()
            if key not in base:
                raise ValueError(f"Overlay is not a replacement: {key}")
            base[key] = path.read_bytes()
    addon_dir = VARIANT / "addon"
    addon = {p.relative_to(addon_dir).as_posix(): p.read_bytes()
             for p in addon_dir.rglob("*") if p.is_file()}
    output = ROOT / "build/engine-fps"
    mod = output / "@ADD_Server_Manager_FPS"
    addons = mod / "addons"
    addons.mkdir(parents=True, exist_ok=True)
    (addons / "ADD_Server_Manager.pbo").write_bytes(pack_pbo(base, "ADD_Server_Manager"))
    (addons / "ADD_Server_Manager_FPS.pbo").write_bytes(pack_pbo(addon, "ADD_Server_Manager_FPS"))
    (mod / "README.md").write_bytes((VARIANT / "README.md").read_bytes())
    (mod / "LICENSE").write_bytes((ROOT / "LICENSE").read_bytes())
    (mod / "REVIEW.md").write_bytes((ROOT / "docs/GAME_MOD_REVIEW.md").read_bytes())
    hashes = {p.relative_to(mod).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
              for p in sorted(mod.rglob("*")) if p.is_file() and p.name != "SHA256.json"}
    (mod / "SHA256.json").write_text(json.dumps(hashes, indent=2) + "\n")
    archive = output / "ADD_Server_Manager_FPS-v0.9.91.zip"
    with zipfile.ZipFile(archive, "w", compression=zipfile.ZIP_DEFLATED) as z:
        for p in sorted(mod.rglob("*")):
            if p.is_file():
                info = zipfile.ZipInfo(p.relative_to(output).as_posix(), (2026, 1, 1, 0, 0, 0))
                info.compress_type = zipfile.ZIP_DEFLATED
                info.external_attr = 0o100644 << 16
                z.writestr(info, p.read_bytes())
    return archive


if __name__ == "__main__":
    print(build())
