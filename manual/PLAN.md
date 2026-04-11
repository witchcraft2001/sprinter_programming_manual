# ZX Sprinter ASM Programming Manual — Implementation Plan

**Target audience:** Programmer familiar with Z80 ASM and basic ZX Spectrum architecture.
**Goal:** A sequential, practice-oriented guide through all Sprinter subsystems with commented ASM code examples.

---

## Directory Structure

```
manual/
├── PLAN.md                    ← this file
├── README.md                  ← manual index / navigation
│
├── 01_architecture/
│   ├── 01_overview.md         ← what is Sprinter, variants, board layout
│   ├── 02_cpu_z84c15.md       ← Z84C15 vs Z80: differences, new instructions, timing
│   ├── 03_memory_map.md       ← full address space: RAM, ROM, VRAM, Flash, cache
│   ├── 04_port_map.md         ← complete port reference table
│   └── 05_boot_sequence.md    ← BIOS boot, Flash loading, DSS startup
│
├── 02_memory/
│   ├── 01_windows.md          ← 4 × 16KB memory windows, EmmWin ports
│   ├── 02_dcp_mapper.md       ← DCP port mapper, table structure
│   ├── 03_cache.md            ← L1/L2 cache, wait states, cache control
│   ├── 04_flash.md            ← Flash read/write/erase procedures, ROM pages
│   └── 05_examples.md         ← ASM: bank switching, copying between pages
│
├── 03_bios/
│   ├── 01_bios_overview.md    ← BIOS role, initialization, RST vectors
│   ├── 02_bios_api.md         ← RST #08 / RST #30 full function reference
│   ├── 03_bios_disk.md        ← BIOS disk routines (IDE, FDD)
│   ├── 04_bios_config.md      ← configuration, hardware switching
│   └── 05_examples.md         ← ASM: calling BIOS, disk read, mouse
│
├── 04_dss/
│   ├── 01_dss_overview.md     ← DSS architecture, memory model, EXE format
│   ├── 02_dss_api.md          ← RST #10 full function reference (00h–5Fh)
│   ├── 03_dss_filesystem.md   ← disk layout, directories, file operations
│   ├── 04_dss_memory.md       ← DSS memory management, windows, Emm_Fn5
│   ├── 05_dss_exe.md          ← EXE header, loading, CallDss wrapper
│   └── 06_examples.md         ← ASM: open/read/write file, list directory
│
├── 05_graphics/
│   ├── 01_video_overview.md   ← video subsystem, modes, VRAM layout
│   ├── 02_zx_mode.md          ← ZX Spectrum compatible mode, attributes
│   ├── 03_mode1_320.md        ← 320×256 8bpp mode, pixel addressing
│   ├── 04_mode2_640.md        ← 640×512 4bpp mode, pixel addressing
│   ├── 05_palette.md          ← color pipeline, palette ports, registers
│   ├── 06_accelerator.md      ← hardware accelerator (LD r,r), block ops
│   ├── 07_double_buffer.md    ← double buffering technique
│   └── 08_examples.md         ← ASM: draw pixel, blit sprite, clear screen
│
├── 06_audio/
│   ├── 01_ay_3_8910.md        ← AY chip: registers, channels, envelopes
│   ├── 02_covox.md            ← Covox DAC: ports, DMA, stereo
│   ├── 03_covox_dma.md        ← DMA audio: CBL buffer, frequencies, I²S
│   └── 04_examples.md         ← ASM: play tone, DMA playback
│
├── 07_disk/
│   ├── 01_disk_overview.md    ← IDE and FDD controllers, disk detection
│   ├── 02_ide.md              ← IDE registers, PIO read/write, LBA addressing
│   ├── 03_fdd.md              ← FDD controller, TRD format, sector access
│   ├── 04_partition.md        ← FAT/DSS partition layout, MBR
│   └── 05_examples.md         ← ASM: read sector, detect drives, boot sector
│
├── 08_peripherals/
│   ├── 01_keyboard.md         ← ZX keyboard matrix + AT keyboard via Z84C15
│   ├── 02_mouse.md            ← mouse via BIOS, port reading
│   ├── 03_isa_bus.md          ← ISA slot: signals, addressing, enable, examples
│   ├── 04_cmos_rtc.md         ← CMOS/RTC access
│   └── 05_serial_parallel.md  ← Z80-SIO, Z80-PIO usage
│
├── 09_advanced/
│   ├── 01_interrupts.md       ← IM0/IM1/IM2, interrupt vectors, VBlank
│   ├── 02_turbo.md            ← turbo mode, wait states, timing
│   ├── 03_hidden_features.md  ← VGA_SWITCH, ALT_ACC, FN_ACC, soft reset
│   └── 04_zx_compat.md        ← ZX Spectrum software compatibility layer
│
├── 10_tutorials/
│   ├── 01_hello_world.md      ← minimal program: hello world to screen
│   ├── 02_file_browser.md     ← DSS file browser step-by-step
│   ├── 03_sprite_engine.md    ← sprite engine with accelerator
│   └── 04_music_player.md     ← AY music player (PT3 format)
│
└── include/                   ← shared ASM include files (symlink or copy)
    ├── dss_equ.inc
    ├── bios_equ.inc
    ├── ports.inc
    ├── kbd_matrix.inc
    └── at_scancodes.inc
```

---

## Implementation Phases

### Phase 1 — Foundation (Chapters 01–02)
**Goal:** Reader understands hardware, memory model, can write/run first program.

| Step | File | Key content |
|------|------|-------------|
| 1.1 | `01_architecture/01_overview.md` | Board variants, FPGA, specs table |
| 1.2 | `01_architecture/02_cpu_z84c15.md` | Z84C15 specifics, instruction timing, BUSRQ |
| 1.3 | `01_architecture/03_memory_map.md` | Full 64KB map, paging windows, shadow ROM |
| 1.4 | `01_architecture/04_port_map.md` | All I/O ports with hex addresses |
| 1.5 | `01_architecture/05_boot_sequence.md` | BIOS init, Flash, DSS loader |
| 1.6 | `02_memory/01_windows.md` | EmmWin.P0–P3, 16KB banks |
| 1.7 | `02_memory/02_dcp_mapper.md` | DCP table, port decoding algorithm |
| 1.8 | `02_memory/03_cache.md` | Cache registers, wait state control |
| 1.9 | `02_memory/04_flash.md` | Flash page layout, write/erase |
| 1.10 | `02_memory/05_examples.md` | Working ASM: bank switch, page copy |

### Phase 2 — System Software (Chapters 03–04)
**Goal:** Reader can call BIOS and DSS, build and run a real EXE program.

| Step | File | Key content |
|------|------|-------------|
| 2.1 | `03_bios/01_bios_overview.md` | BIOS role, RST vectors, init sequence |
| 2.2 | `03_bios/02_bios_api.md` | All RST #08/#30 calls documented |
| 2.3 | `03_bios/03_bios_disk.md` | IDE/FDD disk routines |
| 2.4 | `03_bios/04_bios_config.md` | Hardware switches, config ports |
| 2.5 | `03_bios/05_examples.md` | ASM: read sector, get mouse |
| 2.6 | `04_dss/01_dss_overview.md` | DSS architecture, process model |
| 2.7 | `04_dss/02_dss_api.md` | RST #10 full reference |
| 2.8 | `04_dss/03_dss_filesystem.md` | File system, directories |
| 2.9 | `04_dss/04_dss_memory.md` | Memory allocation, windows |
| 2.10 | `04_dss/05_dss_exe.md` | EXE header format, template |
| 2.11 | `04_dss/06_examples.md` | ASM: open file, read, write, close |

### Phase 3 — Graphics (Chapter 05)
**Goal:** Reader can draw graphics in all modes, use palette and accelerator.

| Step | File | Key content |
|------|------|-------------|
| 3.1 | `05_graphics/01_video_overview.md` | Mode register, VRAM layout |
| 3.2 | `05_graphics/02_zx_mode.md` | ZX attribute system |
| 3.3 | `05_graphics/03_mode1_320.md` | 320×256 linear addressing |
| 3.4 | `05_graphics/04_mode2_640.md` | 640×512 4bpp |
| 3.5 | `05_graphics/05_palette.md` | Palette ports, color pipeline |
| 3.6 | `05_graphics/06_accelerator.md` | LD r,r accelerated opcodes |
| 3.7 | `05_graphics/07_double_buffer.md` | Double buffer technique |
| 3.8 | `05_graphics/08_examples.md` | ASM: pixel, line, sprite, clear |

### Phase 4 — Audio & Disk (Chapters 06–07)
**Goal:** Reader can make sound and access disk storage directly.

| Step | File | Key content |
|------|------|-------------|
| 4.1 | `06_audio/01_ay_3_8910.md` | AY registers, PSG programming |
| 4.2 | `06_audio/02_covox.md` | Covox DAC |
| 4.3 | `06_audio/03_covox_dma.md` | DMA buffer, CBL, I²S |
| 4.4 | `06_audio/04_examples.md` | ASM: beep, play sample |
| 4.5 | `07_disk/01_disk_overview.md` | IDE/FDD detection |
| 4.6 | `07_disk/02_ide.md` | IDE PIO, LBA mode |
| 4.7 | `07_disk/03_fdd.md` | FDD TRD format |
| 4.8 | `07_disk/04_partition.md` | Partition table, DSS format |
| 4.9 | `07_disk/05_examples.md` | ASM: read IDE sector |

### Phase 5 — Peripherals & Advanced (Chapters 08–09)
**Goal:** Reader can use all I/O, interrupts, and advanced CPU features.

| Step | File | Key content |
|------|------|-------------|
| 5.1 | `08_peripherals/01_keyboard.md` | Matrix scan + AT keyboard |
| 5.2 | `08_peripherals/02_mouse.md` | Mouse via BIOS |
| 5.3 | `08_peripherals/03_isa_bus.md` | ISA slot programming |
| 5.4 | `08_peripherals/04_cmos_rtc.md` | CMOS/RTC |
| 5.5 | `08_peripherals/05_serial_parallel.md` | Z80-SIO/PIO |
| 5.6 | `09_advanced/01_interrupts.md` | IM2, VBlank handler |
| 5.7 | `09_advanced/02_turbo.md` | Turbo mode, timing |
| 5.8 | `09_advanced/03_hidden_features.md` | Hidden registers |
| 5.9 | `09_advanced/04_zx_compat.md` | ZX Spectrum compat layer |

### Phase 6 — Tutorials (Chapter 10)
**Goal:** End-to-end worked examples pulling everything together.

| Step | File | Key content |
|------|------|-------------|
| 6.1 | `10_tutorials/01_hello_world.md` | Minimal working program |
| 6.2 | `10_tutorials/02_file_browser.md` | File browser with DSS |
| 6.3 | `10_tutorials/03_sprite_engine.md` | Sprite engine + accelerator |
| 6.4 | `10_tutorials/04_music_player.md` | PT3 AY player |

### Phase 7 — Polish
- `README.md` — manual index with navigation
- Cross-links between chapters
- Review all code examples compile with sjasmplus
- Verify register/port tables against hardware sources

---

## Source Material Mapping

| Manual chapter | Primary sources |
|---------------|-----------------|
| Architecture | `generated_doc/memory.md`, `accelerator.md`, `peripherals.md`; FIDO-архив `other/07BA-90CE/DOCS/` |
| Memory | `generated_doc/memory.md`, `port-mapper.md`, `flash.md`; FPGA-исходники `other/Sprinter/sprinter/SP2000/dcp.inc` |
| BIOS | `programming_guide/bios_api.md`, `bios_equ.inc`, `espprobe/bios_equ.asm`; `other/Sprinter/sprinter/LAST/BIOS/EXP_HDD.ASZ` |
| DSS | `programming_guide/dss_api.md`, `dss_equ.inc`, `espprobe/dss_equ.asm`, `docs/DSS 1.60 rst 10.docx`; **`other/07BA-90CE/DOCS/Manual.txt`** (DSS v1.55, cp866) |
| Graphics | `generated_doc/video-memory.md`, `video-timing.md`, `color-palette.md`, `programming_guide/video.md`; FPGA `video.inc`, `video2.inc` |
| Accelerator | `programming_guide/accelerator.md`; **`other/07BA-90CE/DOCS/accel_r.txt`** (подробное описание LD r,r команд, тайминги, примеры) |
| Audio | `generated_doc/covox-dma.md`, `programming_guide/audio/*.md`; `other/Sprinter/sprinter/SP2000/ay.inc` |
| Disk | `generated_doc/peripherals.md`, `programming_guide/bios_api.md`; **`other/07BA-90CE/DOCS/ide.txt`** (схемы, порты IDE); `other/Sprinter/sprinter/LAST/BIOS/EXP_HDD.ASZ` |
| Peripherals | `generated_doc/peripherals.md`, `programming_guide/keyboard.md`, `isa.md`; `other/Sprinter/sprinter/SP2000/kbd.inc`, `mouse.inc` |
| Advanced | `generated_doc/hidden-features.md`, `accelerator.md`; FIDO-архив (обсуждения FPGA, FORTH-CPU, UTF-8) |
| Tutorials | `espprobe/` source files; **`other/07BA-90CE/DEMOS/PLASMA2/plasma2.asm`** (plasma effect — реальный EXE с page switching и палитрой) |

---

## Дополнительные находки из `other/`

### Из `other/07BA-90CE/DOCS/`

**`Manual.txt`** (DSS v1.55, cp866, ~объёмный)
- Полный справочник функций RST 10h (00h–0Eh и далее)
- Атрибуты файлов: бит 0=read-only, 1=hidden, 2=system, 3=volume, 4=dir, 5=archive
- Соглашения регистров: C — номер функции, HL — указатель, CF — флаг ошибки
- FAT16, имена до 8 символов, разделитель `\`
- **→ Первичный источник для `04_dss/02_dss_api.md` и `04_dss/03_dss_filesystem.md`**

**`accel_r.txt`** (cp866)
- Детальное описание всех accelerator-команд `LD r,r`:
  - `LD B,B` — выключить акселератор
  - `LD D,D` + `LD A,N` — задать размер блока (1–256 байт)
  - `LD C,C` — заполнить блок из A (fill)
  - `LD E,E` — заполнение вертикальных линий на графическом экране
  - `LD L,L` — копировать блок из RAM акселератора в VRAM
  - `LD A,A` — копировать вертикальные линии на графический экран
- **Важно:** скорость акселератора НЕ зависит от turbo-режима (ограничена физическим Z80)
- Пример: полная прокрутка экрана ~26 мс
- DI/EI обязательны во время операций акселератора
- **→ Расширить `05_graphics/06_accelerator.md` этими деталями**

**`ide.txt`** (cp866)
- Схема IDE-контроллера на ИС серии 1533
- Полная таблица портов:
  - `xx50h` — данные (16-бит через буфер)
  - `xx51h` — ошибка/precomp
  - `xx52h` — счётчик секторов
  - `xx53h` — номер сектора
  - `xx54h/xx55h` — цилиндр (low/high)
  - `4052h` — drive/head select
  - `4053h` — команда/статус
  - `4054h` → порт 3F6h (alt status/device control)
  - `4055h` → порт 3F7h (drive address)
- Поддержка MASTER/SLAVE, сигнал готовности `/HD_RDY`
- **→ Первичный источник для `07_disk/02_ide.md`**

### Из `other/07BA-90CE/DEMOS/PLASMA2/`

**`plasma2.asm`** — полноценный демо-EXE (plasma effect)
- Показывает реальный паттерн EXE-заголовка: сигнатура `5845h`, версия `00h`, адрес `8100h`
- Переключение страниц памяти: PORT_Y (`#89`), PAGE3 (`#E2`)
- Предвычисленные таблицы синусов (256 значений)
- Прямая запись пикселей в VRAM (`#C000`+)
- Вращение RGB-палитры через порт `#C000+992`
- Опрос клавиатуры через RST 10h функция `33h`
- **→ Основа для `10_tutorials/03_sprite_engine.md` (или новый туториал plasma)**

### Из `other/Sprinter/sprinter/SP2000/` (FPGA-исходники)

| Файл | Содержимое | Применение |
|------|-----------|------------|
| `dcp.inc` | Контроллер памяти, turbo, 18-бит RAM, refresh | `02_memory/02_dcp_mapper.md` |
| `acceler.inc` | Блочные операции, копирование, XOR | `05_graphics/06_accelerator.md` |
| `video.inc` | 20-бит видеоадрес, 42MHz, multi-bank | `05_graphics/01_video_overview.md` |
| `video2.inc` | 640×256 режим, интегрированный курсор мыши | `05_graphics/04_mode2_640.md` |
| `kbd.inc` | Матрица клавиатуры, спец-клавиши, INT | `08_peripherals/01_keyboard.md` |
| `mouse.inc` | X/Y координаты (10-бит), кнопки, INT | `08_peripherals/02_mouse.md` |
| `ay.inc` | 3 канала AY, стерео L/R | `06_audio/01_ay_3_8910.md` |
| `dc_port.inc`, `dc_port2.inc` | Декодер портов, CNF-биты, маски | `01_architecture/04_port_map.md` |

### Из `other/Sprinter/sprinter/LAST/BIOS/`

| Файл | Содержимое | Применение |
|------|-----------|------------|
| `EXP_HDD.ASZ` | Прошивка IDE: HD_CMD_0–6, порты P_HDST/P_S_CNT/P_S_NUM | `07_disk/02_ide.md`, `03_bios/03_bios_disk.md` |
| `EXP_DCP2.ASZ` | Загрузчик FPGA-битстрима, DCP_INIT, RLC-распаковка | `02_memory/02_dcp_mapper.md` |

### FIDO-архив `other/07BA-90CE/` (исторические обсуждения)

Полезные технические факты из переписки разработчиков:
- FPGA-ресурсы: клавиатура 5% LCELL, мышь 4%, AY-чип 8%, FORTH-CPU 33%
- FORTH-CPU прототип: 32-бит, 14MHz, занимал треть FPGA
- Поддержка воспроизведения видео в формате 160×128 (фильм «Матрица», 20 минут)
- Планировалась поддержка UTF-8
- **→ Материал для `09_advanced/03_hidden_features.md` и раздела истории в `01_architecture/01_overview.md`**

---

## Новые/расширенные пункты плана

В результате анализа `other/` добавить:

1. **`05_graphics/06_accelerator.md`** — расширить полной таблицей команд из `accel_r.txt` с тайминг-данными
2. **`07_disk/02_ide.md`** — добавить полную таблицу портов из `ide.txt` со схемой адресации
3. **`10_tutorials/05_plasma_effect.md`** *(новый)* — разобрать `plasma2.asm` по шагам: EXE-заголовок → page switching → sine table → VRAM → palette cycling
4. **`01_architecture/01_overview.md`** — добавить исторический контекст из FIDO-архива (FORTH-CPU, видео-движок, UTF-8)
5. **`11_appendix/`** *(новый раздел)*:
   - `A_port_reference.md` — полная сводная таблица всех портов (из generated_doc + ide.txt + dc_port.inc)
   - `B_fpga_signals.md` — справочник FPGA-сигналов из SP2000/*.inc для понимания low-level

---

---

## Новые находки: sprinter_bios, sprinter_dss, sprintem, flappybird, sdk

### `sprinter_bios/SETUP/` — Исходники BIOS (cp866, AS80)

**`HDRIVER6.ASM`** — Полный IDE-драйвер. Точные порты (Write/Read раздельно):
```
HDW_COM=#4153  HDR_CTL=#4053  ; 1F7h Command/Status
HDW_DRV=#4152  HDR_DRV=#4052  ; 1F6h Drive/Head
HDW_CLH=#0155  HDR_CLH=#0055  ; 1F5h Cylinder High
HDW_CLL=#0154  HDR_CLL=#0054  ; 1F4h Cylinder Low
HDW_SEC=#0153  HDR_SEC=#0053  ; 1F3h Sector
HDW_CNT=#0152  HDR_CNT=#0052  ; 1F2h Count
HDW_ERR=#0151  HDR_ERR=#0051  ; 1F1h Error
HDW_DAT=#0150  HDR_DAT=#0050  ; 1F0h Data
```
- Биты статуса: BSY=bit7, RDY=bit6, DRQ=bit3, ERR=bit0
- IDE-дескрипторы IDE0/IDE1 в памяти по `#C1C0`/`#C1C8` (8 байт: heads, SPT, cylinders, drive/head reg, type)
- Команды GETMEDH/SETMEDH/READH/LREADH/WRITEH (длинное чтение с поддержкой block ID)
- **→ Первичный источник для `07_disk/02_ide.md`**

**`ROM.ASM`** — Карта ROM-образа:
- `#0000`: EXTENDED.BIN (extended BIOS)
- `#1000`: BSETUP.BIN (BIOS setup)
- `#3FD0`–`#3FFF`: RST-векторы и патч-area; порты `#7C` и `#3C` используются в инициализации
- **→ Источник для `03_bios/01_bios_overview.md`, `01_architecture/05_boot_sequence.md`**

**`VIDEO_IO.ASM`** — Текстовые видео-процедуры BIOS:
- Константы цветов: BLACK=#00 … WHITE=#0F
- Процедуры: GET_CUR, LOCAT (C=#84), CRLF, PRINT (C=#82), PRINTZ (C=#8C), IPRINT
- Все вызовы через `IPOINT` = BIOS RST #08 wrapper
- **→ Источник для `03_bios/02_bios_api.md`**

### `sprinter_dss/DSS.INC` + `DOS-MAIN.ASM` — DSS OS

Полная таблица функций RST #10 (подтверждена с кодами):
```
#00 VERSION    #01 CHDISK     #02 CURDISK    #03 DSKINFO
#09 BOOTDSK    #0A CREATE     #0B CREAT_N    #0E DELETE
#10 RENAME     #11 OPEN       #12 CLOSE      #13 READ
#14 WRITE      #15 MOVE_FP    #16 ATTRIB     #17 GET_D_T
#18 PUT_D_T    #19 F_FIRST    #1A F_NEXT     #1B MKDIR
#1C RMDIR      #1D CHDIR      #1E CURDIR     #21 SYSTIME
#22 SETTIME    #30 WAITKEY    #31 SCANKEY    #32 ECHOKEY
#33 CTRLKEY    #35 K_CLEAR    #36 K_SETUP    #37 TESTKEY
#38 SETWIN     #39 SETWIN1    #3A SETWIN2    #3B SETWIN3
#3C INFOMEM    #3D GETMEM     #3E FREEMEM    #3F SETMEM
#40 EXEC       #41 EXIT       #42 WAIT       #43 GSWITCH
#44 DOSNAME    #45 EX_PATH    #46 ENVIRON    #47 APPINFO
#50 SETVMOD    #51 GETVMOD    #52 LOCATE     #53 CURSOR
#54 SELPAGE    #55 SCROLL     #56 CLEAR      #57 RDCHAR
#58 WRCHAR     #59 WINCOPY    #5A WINREST    #5B PUTCHAR
#5C PCHARS     #5F PRINT
```
- Дополнительные функции BIOS RST #08 (через Estex): `#80-#8E` (LP_PRINT_*), `#A1` PIC_POINT, `#A4-#A6` PAL, `#B0` WIN_OPEN, `#C0-#C7` EMM_FN0-7
- **→ Исчерпывающий источник для `04_dss/02_dss_api.md`**

Найдены DSS-утилиты с ASM-исходниками: ATTRIB, BOOT, CDX, CMOS, COPY, FVIEW, IF, INPUT, KEYBOARD, MENU, RAMDRIVE — готовые примеры реального кода!

### `sprintem/bios.cpp` — Эмулятор как спецификация

EMM API (BIOS функции памяти через RST #08):
- `EMM_FN0 (#C0)` — получить инфо о RAM (кол-во страниц)
- `EMM_FN1 (#C1)` — инициализация RAM-менеджера
- `EMM_FN2 (#C2)` — выделить блок памяти (B=кол-во страниц) → A=ID блока
- `EMM_FN3 (#C3)` — освободить блок (A=ID)
- `EMM_FN4 (#C4)` — получить реальный номер страницы (A=ID блока, B=индекс) → A=реальный номер
- `EMM_FN5 (#C5)` — получить список реальных номеров страниц (A=ID, HL=буфер) → [HL..]=номера
- `EMM_FN6 (#C6)` — получить порты окон → DE=P0/P1, IX=P2/P3 (т.е. #82,#A2,#C2,#E2)
- `EMM_FN7 (#C7)` — следующая страница блока (A=ID, B=текущий индекс) → A=следующий
- **→ Расширить `04_dss/04_dss_memory.md`**

### `flappybird/src/` — Реальная игра (полный код ASM)

**Ключевые паттерны:**
1. **EXE-заголовок** (`include/head.asm`):
   ```asm
   org 8100h-512
   db "EXE", 0        ; сигнатура
   dw 200h            ; версия
   dw 0,0,0,0,0
   dw begin           ; точка входа
   dw begin           ; ...
   dw 0BFFFh          ; stack
   ds 490             ; padding до 512 байт
   ```
2. **Переключение страниц для ресурсов:**
   ```asm
   out (EmmWin.P3),a  ; переключаем страницу в окне #3
   out (#89),A        ; PORT_Y — строка видеопамяти
   ```
3. **Двойная буферизация** (чтение/запись `RGMOD=#C9`):
   ```asm
   in  a,(RGMOD)
   xor 1
   out (RGMOD),a      ; flip активного экрана
   ```
4. **Акселератор** (`ShowBitmapAcc`):
   ```asm
   di
   ld d,d             ; enable accel, set size
   ld a,SIZE          ; block size (1-256)
   ld l,l             ; copy block to VRAM
   ei
   ```
5. **Клавиатура через SIO** (`sys_utils.asm`):
   ```asm
   in  a,(SIO_CONTROL_A)  ; #19 — бит 0: байт готов
   in  a,(SIO_DATA_REG_A) ; #18 — PS2 scancode
   cp  #F0               ; key-up prefix
   ```
6. **IM2 setup** (`im2_utils.asm`): вектор I=#80, таблица в #80FF (два байта = адрес обработчика)
7. **DSS API** в действии: `Dss.AppInfo` (#47), `Dss.ChDir`, `Dss.GetMem`, `Bios.Emm_Fn5`, `Dss.Open/Read/Close/Exit`
- **→ Основа для `10_tutorials/` и примеров во всех главах**

### `zx-sprinter-sdk/sdk/src/sprinter/lib_startup.asm` — Production runtime library

**Критичные технические детали:**
- **CTC таймер**: `CTC_CH0=#10`, `CH1=#11`, `CH2=#12`, `CH3=#13`; инициализация: `out (CTC_CH2),0x57 / 112 / out (CTC_CH3),0xD7 / 160`
- **IM2 VSync**: таблица векторов в `#FE00` (257 байт `#FD`), обработчик на `#FDFD`, VSync handler на `#FE06`
- **AY вывод из прерывания**: регистры `#FFFD`/`#FFBF`, функция `ROUT` — 13 регистров за кадр
- **Страницы ресурсов**:
  - `VPAGE_TILES=#50` — тайлы (16KB страница)
  - `VPAGE_SPRITES=#5C` — спрайты
  - Звуковая страница, палитра, картинки — в отдельных страницах из DSS EMM
- **double-buffer flip**: `out (RGMOD),a` — бит 0 переключает экран
- **Палитра** через `PORT_Y=#89`: инкрементный перебор строк + запись через `$43E0`-`$43E4`
- **Акселератор** в sprite engine: `ld d,d` (set size) + `ld l,l` (copy line) внутри DI/EI
- **SDK build**: SDCC (C→Z80) + sjasmplus, `crt0.s`→`sprinter.rel`→`startup.bin`, линкер
- **→ Источник для `09_advanced/01_interrupts.md`, `05_graphics/07_double_buffer.md`, новая глава `10_c_sdk`**

### `espprobe/espprobe.asm` — ISA COM-порт зонд

Демонстрирует доступ к ISA через адресацию:
```asm
isa_adr_base    equ 0xC000   ; ISA window base
base_com1_addr  equ 0x3F8    ; COM1 base address
SER_P = isa_adr_base + 0x3F8
; порты: LCR=SER_P+3, FCR=SER_P+2, DLL=SER_P, DLM=SER_P+1, MCR=SER_P+4, LSR=SER_P+5
Port.ISA = #9FBD             ; ISA enable port
```
- **→ Источник для `08_peripherals/03_isa_bus.md`**

---

## Обновлённая карта источников

| Глава руководства | Первичные источники |
|------------------|---------------------|
| Архитектура | `generated_doc/memory.md`, `accelerator.md`; FIDO-архив; `sprinter_bios/SETUP/ROM.ASM` |
| Память/окна | `generated_doc/memory.md`, `port-mapper.md`; `flappybird/src/include/sp_equ.asm` |
| BIOS API | `sprinter_bios/SETUP/VIDEO_IO.ASM`; `sprintem/bios.cpp` (EMM_FN0-7); `programming_guide/bios_api.md` |
| DSS API | `sprinter_dss/DSS.INC`; `sprintem/bios.cpp` (case-by-case); `docs/DSS 1.60 rst 10.docx` |
| DSS Memory | `sprintem/bios.cpp` EMM_FN0-7; `flappybird/src/fbird.asm` |
| DSS EXE | `flappybird/src/include/head.asm`; `sprintem/bios.cpp` EXEC(#40) |
| DSS Utils | `sprinter_dss/utils/` (ATTRIB, CDX, FVIEW, MENU, KEYBOARD, CMOS, COPY, INPUT) |
| Графика | `generated_doc/video-memory.md`; `flappybird/src/grx_utils.asm`; `sdk/src/sprinter/lib_startup.asm` |
| Акселератор | `other/07BA-90CE/DOCS/accel_r.txt`; `flappybird/src/grx_utils.asm`; `sdk/src/sprinter/lib_sprites.asm` |
| Двойная буферизация | `flappybird/src/fbird.asm`; `sdk/src/sprinter/lib_startup.asm` _swap_screen |
| Аудио/AY | `sdk/src/sprinter/lib_startup.asm` ROUT/im2VShandler; `sdk/src/wyzplayer/` |
| Диск/IDE | `sprinter_bios/SETUP/HDRIVER6.ASM`; `other/07BA-90CE/DOCS/ide.txt`; `sprinter_dss/IDE_DRV0.ASM` |
| Диск/FDD | `sprinter_bios/SETUP/FDRIVER2.ASM`; `sprinter_dss/FDD_DRV0.ASM` |
| Клавиатура | `flappybird/src/sys_utils.asm` (SIO); `sdk/src/sprinter/lib_input.asm`; `sprinter_dss/KEYINTER.ASM` |
| Мышь | `sprinter_dss/INTMOUSE.ASM`; `sprintem/bios.cpp` (BIOS #00-#03) |
| ISA Bus | `espprobe/espprobe.asm`, `espprobe/isa.asm`; `programming_guide/isa.md` |
| CMOS/RTC | `sprinter_dss/utils/CMOS/CMOS.ASM` |
| Прерывания/IM2 | `flappybird/src/im2_utils.asm`; `sdk/src/sprinter/lib_startup.asm` im2VShandler |
| CTC таймер | `sdk/src/sprinter/lib_startup.asm` (CTC_CH0-3 = #10-#13) |
| Туториалы | `flappybird/` (полная игра); `plasma2.asm`; `sprintem/sources/FRACTALS/`; `sprintem/disk/HELLO.ASM` |

---

## Новые/расширенные пункты плана (v2)

Добавить к существующей структуре:

**Новые файлы:**
- `05_graphics/09_palette_hw.md` — аппаратная палитра через PORT_Y; адрес `$43E0-$43E4`; палитра из прерывания
- `09_advanced/05_ctc_timer.md` *(новый)* — Z84C15 CTC каналы (#10-#13), инициализация, использование для таймера/IM2
- `10_tutorials/06_game_template.md` *(новый)* — разбор структуры `flappybird/` как шаблона игры (EXE → init → IM2 → game loop → exit)
- `10_tutorials/07_sdk_c.md` *(новый)* — zx-sprinter-sdk: C+ASM, build pipeline, спрайты, звук

**Расширить существующие:**
- `05_graphics/06_accelerator.md` — три паттерна из реального кода: fill, copy-line (SDK), copy-block (flappybird)
- `07_disk/02_ide.md` — добавить HDRIVER6 порты, биты статуса, IDE-дескриптор структуру
- `08_peripherals/01_keyboard.md` — секция про SIO (#18/#19) + PS2 scancode декодирование из `lib_input.asm`
- `09_advanced/01_interrupts.md` — рабочий пример IM2 VSync handler из SDK + flappybird
- `11_appendix/C_dss_utils.md` *(новый)* — ссылки на исходники DSS-утилит как примеры реального кода

---

## Находки из FPGA-исходников (sp-altera-src-2025-11-30, K30/K50)

Два агента независимо проанализировали TDF/MIF файлы и получили исчерпывающую картину железа.

### Архитектура: два чипа

| Чип | Роль |
|-----|------|
| **ACEX 1K30** (`SP2_ACEX.TDF`) | CPU-интерфейс, управление памятью, видео, AY, акселератор, COVOX, клавиатура, мышь |
| **MAX7000** (`SP2_MAX.TDF`) | Синхронизатор: FDD-контроллер, HDD-интерфейс, CMOS, VGA-sync, reset-логика |

ACEX K30 использует 91% LC (1580 ячеек), 57% RAM.

### Тактирование (из DCP.TDF + SP2_ACEX.TDF)

| Домен | Частота | Источник |
|-------|---------|---------|
| CLK42 | 42 MHz | TG42 вход (главный клок) |
| CLK84 | 84 MHz | CLK42 XOR |
| CLK21 | 21 MHz | CLK42 ÷ 2 (turbo Z80) |
| Z80 normal | 7 MHz | 42 MHz ÷ 6 (CTZ паттерн `!CT2 & CT1`) |
| Z80 turbo | 21 MHz | CLK21 — активируется F12+Ctrl+Shift |
| CLK14 | 14 MHz | MAX chip XCT ÷ 3 |
| CLK_K | 15 kHz | для PS/2 клавиатуры |

### Таблица портов из DCP.MIF (256×16-бит LUT)

Порты декодируются через DCPP[7:0] — сжатый адрес I/O. TYPE[3:0] в старшем нибле:

| DCPP паттерн | Порт | Функция |
|-------------|------|---------|
| `11000010` | C2h | **Border** (биты 2:0 = цвет, бит 3 = BEEPER) |
| `11000000` | C0h | **SC register** (Scorpion paging) |
| `11000001` | C1h | **PN register** (номер страницы) |
| `11000100` | C4h | **Accelerator line** (начальная строка blitter) |
| `11000101` | C5h | **RGMOD** (режим/страница видео) |
| `11000110` | C6h | **CNF register** (config: ROM, turbo, Scorpion) |
| `11000111` | C7h | ALT accelerator mode |
| `11000011` | C3h | **ALL_MODE** (global: acc_enable, kbd int mode) |
| `10001001` | 89h | **COVOX mode** register |
| `10001000` | 88h | **COVOX data** write |
| `10001101` | 8Dh | AY address write |
| `10001110` | 8Eh | AY data write |
| `10001100` | 8Ch | AY data read |
| `10001111` | 8Fh | ROM write enable |
| `11110XXX` | F0h–FFh | ISA slot ports |
| `00011011` | 1Bh | ISA A20 line |

**Kempston Mouse** (порт `0x58`):
- A10=0, A8=0 → кнопки (bit1=right, bit0=left)
- A10=0, A8=1 → X позиция
- A10=1, A8=1 → Y позиция (инвертирована)

### COVOX/PCM — таблица частот дискретизации (из CBL_TAB в SP2_ACEX.TDF)

| Биты [3:0] | Частота |
|-----------|---------|
| 0 | 16 кГц |
| 1 | 22 кГц |
| 8 | 7.8 кГц |
| 9 | 10.9 кГц |
| 10 | 15.6 кГц |
| 11 | 21.9 кГц |
| 12 | 31.25 кГц |
| 13 | 43.75 кГц |
| 14 | 54.7 кГц |
| 15 | 109.4 кГц |

COVOX port `0x89` биты: bit7=enable, bit6=stereo, bit5=16bit, bit4=int enable, bits[3:0]=rate.
I²S DAC: DAC_DATA (serial MSB first), DAC_WS (word select), DAC_BCK (bit clock).

### Акселератор из ACCELER.TDF — точная таблица опкодов

| Опкод | Мнемоника | Функция акселератора |
|-------|-----------|---------------------|
| `40h` | `LD B,B` | Disable (mode 0) |
| `49h` | `LD C,C` | Fill by constant (A) |
| `52h` | `LD D,D` | Load count (block size) |
| `5Bh` | `LD E,E` | Fill vertical lines |
| `64h` | `LD H,H` | Double-byte function |
| `6Dh` | `LD L,L` | Copy horizontal line |
| `7Fh` | `LD A,A` | Copy line vertical |

Перехват: паттерн `DI[] == B"010XX0XX"` или `B"011XX1XX"` на M1-цикле (без префиксов).
ACC_DIR[7:0]: bit0=ACC_ON, bit1=dir, bit2=counter_en, bit4=enable, bit6=DOUBLE_CAS.

### Видеосистема из VIDEO2.TDF

**Timing counters:**
- CTH[5:0] — горизонтальный, 0..55 (56 состояний)
- CTV[8:0] — вертикальный, 0..319 (320 строк)
- HOR_PLACE=0x50, VER_PLACE=0x91 — позиции sync

**MODE0 register bits:**
- bit0: INT enable для этого экрана
- bit3: text/graphic (0=graphic, 1=text)
- bit4: page 0/1 select
- bit5: resolution (0=320px, 1=640px)

**Sprinter mode** (VAI19=1): 20-бит видеоадрес, 512KB VRAM, 4 × 8-бит шины (VD0..VD3) = 32-бит/такт

**Spectrum mode** (VAI19=0): ZX-совместимая адресация, 8K/16K экранные страницы

**INT** (CPU interrupt): срабатывает при `CTV[2:0]==7` — растровое прерывание по фиксированной строке.

### Wait states (из DCP, таблица W_TAB)

| Режим | TYPE[14:12] | Ожидание |
|-------|------------|---------|
| Normal | 0–1 | 2 такта |
| Normal | 2–3 | 1 такт |
| Normal | 6 | 7 тактов |
| Normal | 7 | 10 тактов |
| Turbo | 0–3 | 2 такта |
| Turbo | 4–5 | 4 такта |
| Turbo | 6–7 | 7 тактов |
| HDD порты | — | 10 тактов |
| HDD данные | — | 4 такта |

### История версий прошивки (из orig/readme.txt)

| Файл | Описание |
|------|---------|
| `_213.BIN` | Последняя с встроенными ROM Spectrum и тестами. Старая схема ROM. |
| `_217.BIN` | Последняя ветки 2.xx |
| `_218src.BIN` | Версия, собираемая из сохранившихся исходников (поведение близко к 3.00) |
| `_300.BIN` | Ключевая версия (Sprinter Update 1, SU1). Последний официальный релиз. |
| `_303.BIN` | Не выпускалась официально. Опубликована в 2009 при открытии исходников. |
| `_304.BIN` | Отличается таймингами DRAM под Alliance VRAM chips. |

### DOS-флаг и ROM-маппинг (из DCP.TDF)

`DOS` флаг устанавливается при M1-фетче из `#DD00–#DDFF` (когда `PN4 & A13 & A12 & A[11:8]==0xD`).
- DOS=0: основной BASIC ROM
- DOS=1: DOS ROM

ROM банки выбираются через `RA[17:14]` (4 бита = 16 банков):
- SYS=0: `(!AROM16, 0, 0, 0)` — системный ROM 0/1
- SYS=1: `(!AROM16, 0, SPR_[1:0])` — extended/DOS/BASIC

### Дополнения к плану из FPGA-анализа

**Расширить существующие главы:**
- `01_architecture/04_port_map.md` — добавить полную таблицу DCPP декодирования из DCP.MIF
- `01_architecture/02_cpu_z84c15.md` — тактирование (7/21 MHz), схема turbo через F12
- `02_memory/02_dcp_mapper.md` — внутренняя структура DCP LUT (256×16 бит), TYPE[3:0] значения, wait-state таблица
- `05_graphics/01_video_overview.md` — точные счётчики CTH/CTV, HOR_PLACE/VER_PLACE, INT строка
- `05_graphics/06_accelerator.md` — полная таблица опкодов из ACCELER.TDF с механизмом перехвата M1-цикла
- `06_audio/02_covox.md` — полная таблица частот CBL_TAB, I²S DAC, порт 0x89 биты
- `09_advanced/02_turbo.md` — три режима тактирования, W_TAB wait states, HDD тайминги
- `03_bios/05_boot_sequence.md` — история версий прошивки, два-PLD дизайн, ALTERA конфигурация
- `08_peripherals/02_mouse.md` — Kempston mouse порт 0x58, три адреса (кнопки/X/Y)

**Новые файлы:**
- `11_appendix/D_fpga_port_lut.md` — полная таблица DCP.MIF (256 записей) для разработчиков
- `11_appendix/E_firmware_versions.md` — история версий прошивки K30

---

## Conventions in the Manual

- All port addresses in HEX: `port #xxxx`
- ASM examples use **sjasmplus** syntax
- cp866-файлы цитируются с конвертацией в UTF-8
- Every chapter ends with a "Key Points" summary box
- Code listings have line-by-line comments
- Tables use compact Markdown format
- Links to related chapters at top of each file
