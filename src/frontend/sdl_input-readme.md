# sdl_input.c

**Peran:** Pemetaan keyboard dan game controller ke tombol NES.

## Keyboard

`Z`/`X` = A/B, `Right Shift` = Select, `Enter` = Start, panah = D-pad,
`Escape` = keluar.

## Game controller

Controller dibuka otomatis saat terhubung dan ditutup saat dilepas.
Controller pertama mengisi port 1, kedua port 2. Mapping memakai
`SDL_GameController` standar.

## Fungsi

- `sdl_input_create()` / `sdl_input_destroy()`.
- `sdl_input_process_event()`.
- `sdl_input_buttons()`: hasilkan bitmask `ControllerButton`.
