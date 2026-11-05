# Modul Input

`controller.c` meniru shift register controller NES.

## Cara Kerja

Controller menyimpan status 8 tombol pada `buttons`. Menulis `$4016`:

- bit 0 = 1: `shift = buttons` (strobe aktif);
- bit 0 = 0: strobe dilepas, pembacaan berikutnya menggeser register.

Setiap pembacaan mengembalikan bit 0 `shift`, lalu menggeser. Setelah 8
pembacaan, bit yang dikembalikan bernilai 1 (open bus).

Urutan bit:

```
A, B, Select, Start, Up, Down, Left, Right
```

## API

| Fungsi | Keterangan |
|---|---|
| `controller_init` | Reset status |
| `controller_set_buttons` | Set tombol aktif |
| `controller_get_buttons` | Ambil status mentah |
| `controller_write_strobe` | Proses tulisan `$4016` |
| `controller_read` | Baca satu bit serial |

Frontend mengisi tombol melalui `nes_set_buttons()`, yang memanggil
`controller_set_buttons()` pada port yang sesuai.
