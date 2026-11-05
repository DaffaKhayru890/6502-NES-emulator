#!/usr/bin/env python3
"""Membuat ROM NROM sintetis minimal untuk menguji TinyNES.

ROM ini hanya berisi program loop `JMP $8000` dan vektor reset/NMI/IRQ yang
menunjuk ke alamat yang sama. Layar akan menampilkan warna latar universal
karena CHR RAM kosong dan rendering tidak diaktifkan.

Pemakaian:
    python3 examples/make_synthetic_rom.py contoh.nes
    python3 examples/make_synthetic_rom.py --nes2 contoh-nes2.nes
    python3 examples/make_synthetic_rom.py --pal contoh-pal.nes
"""

import struct
import sys


def main() -> int:
    args = [arg for arg in sys.argv[1:] if not arg.startswith("--")]
    nes2 = "--nes2" in sys.argv[1:]
    pal = "--pal" in sys.argv[1:]

    if pal:
        nes2 = True

    if len(args) != 1:
        print(
            "Pemakaian: make_synthetic_rom.py [--nes2|--pal] <keluaran.nes>",
            file=sys.stderr,
        )
        return 1

    # 16 KiB PRG, tanpa CHR ROM (CHR RAM 8 KiB dibuat oleh emulator).
    prg = bytearray([0xEA]) * 0x4000  # NOP

    # $8000: JMP $8000
    prg[0x0000:0x0003] = bytes([0x4C, 0x00, 0x80])

    # Vektor NMI, reset, dan IRQ semuanya menunjuk ke $8000.
    prg[0x3FFA:0x3FFC] = struct.pack("<H", 0x8000)
    prg[0x3FFC:0x3FFE] = struct.pack("<H", 0x8000)
    prg[0x3FFE:0x4000] = struct.pack("<H", 0x8000)

    flags7 = 0x08 if nes2 else 0x00
    tail = bytearray(8)
    if pal:
        tail[4] = 0x01  # byte 12: timing PAL
    header = b"NES\x1a" + bytes([1, 0, 0, flags7]) + bytes(tail)

    with open(args[0], "wb") as fp:
        fp.write(header)
        fp.write(prg)

    format_name = "NES 2.0 PAL" if pal else ("NES 2.0" if nes2 else "iNES 1.0")
    print(f"ROM {format_name} dibuat: {args[0]} ({len(header) + len(prg)} byte)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
