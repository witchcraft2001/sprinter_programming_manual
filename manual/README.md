# ZX Sprinter (Peters Plus Sp2000) — Руководство программиста на Z80 ASM

**Версия:** 1.0
**Целевая аудитория:** Программисты, знакомые с Z80 ASM и базовой архитектурой ZX Spectrum
**Синтаксис:** sjasmplus (hex через `#xxxx`, команды в нижнем регистре)

---

## Оглавление

### 01. Архитектура

| Раздел | Описание |
|--------|---------|
| [1.1 Обзор ZX Sprinter](01_architecture/01_overview.md) | Аппаратные варианты, компоненты, DSS, BIOS, история прошивок |
| [1.2 Процессор Z84C15](01_architecture/02_cpu_z84c15.md) | Z84C15 vs Z80, тактирование 7/21 МГц, SIO, CTC, IM2 |
| [1.3 Карта памяти](01_architecture/03_memory_map.md) | 4 окна по 16 КБ, физические страницы, маппинг при загрузке |
| [1.4 Карта портов](01_architecture/04_port_map.md) | Полная таблица портов I/O: AY, COVOX, IDE, FDD, CMOS, ISA |
| [1.5 Последовательность загрузки](01_architecture/05_boot_sequence.md) | BIOS, CMOS, DSS, EXE-формат, системные адреса |

### 02. Память

| Раздел | Описание |
|--------|---------|
| [2.1 Окна памяти](02_memory/01_windows.md) | EmmWin.P0–P3, банкинг, страницы, SavePage/RestorePage |
| [2.2 DCP маппер](02_memory/02_dcp_mapper.md) | 256×16-бит LUT, TYPE поле, wait states, системные регистры |
| [2.3 Кэш-ОЗУ (SRAM) и wait states](02_memory/03_cache.md) | SRAM 128 КБ, 0 waits в турбо, I/O и memory waits |
| [2.4 Flash / ROM](02_memory/04_flash.md) | Страницы ROM, порт #8F, JEDEC программирование |
| [2.5 Примеры](02_memory/05_examples.md) | SavePage, CopyPage, ShowBitmap, шаблон EXE |

### 03. BIOS

| Раздел | Описание |
|--------|---------|
| [3.1 Обзор BIOS](03_bios/01_bios_overview.md) | RST #08, RST #30, группы функций |
| [3.2 BIOS API](03_bios/02_bios_api.md) | Полный справочник: EMM, печать, графика, диски, мышь |
| [3.3 BIOS дисковые операции](03_bios/03_bios_disk.md) | IDE, FDD через BIOS (DRV_READ, DRV_WRITE) |
| [3.4 Конфигурация](03_bios/04_bios_config.md) | CMOS регистры, SETUP, контрольная сумма |
| [3.5 Примеры BIOS](03_bios/05_examples.md) | Hello, выделение RAM, чтение сектора, графика, мышь |

### 04. DSS — операционная система

| Раздел | Описание |
|--------|---------|
| [4.1 Обзор DSS](04_dss/01_dss_overview.md) | Архитектура, системные страницы, EXE формат |
| [4.2 DSS API](04_dss/02_dss_api.md) | Полный справочник функций #00–#5F |
| [4.3 Файловая система](04_dss/03_dss_filesystem.md) | FAT16, 8.3 имена, чтение/запись, каталоги |
| [4.4 Управление памятью](04_dss/04_dss_memory.md) | GetMem, SetWin, блоки страниц |
| [4.5 EXE формат](04_dss/05_dss_exe.md) | Заголовок, загрузка, EXEC, аргументы |
| [4.6 Примеры DSS](04_dss/06_examples.md) | Hello, файлы, листинг, лог, копирование |

### 05. Графика

| Раздел | Описание |
|--------|---------|
| [5.1 Обзор видео](05_graphics/01_video_overview.md) | VRAM, режимы, PORT_Y безопасность, основы |
| [5.2 ZX режим](05_graphics/02_zx_mode.md) | 256×192, атрибуты, бордюр |
| [5.3 Режим 320×256](05_graphics/03_mode1_320.md) | 8bpp линейный, основной для игр |
| [5.4 Режим 640×256](05_graphics/04_mode2_640.md) | 4bpp для GUI в высоком разрешении |
| [5.5 Палитра](05_graphics/05_palette.md) | 256 цветов из 16млн., порты #89/#A4, эффекты |
| [5.6 Акселератор](05_graphics/06_accelerator.md) | LD r,r команды, блочные операции |
| [5.7 Double buffering](05_graphics/07_double_buffer.md) | RGMOD flip, VSync синхронизация |
| [5.8 Текстовый режим 80×32](05_graphics/08_text_mode.md) | DSS text API, псевдографика, рамки |
| [5.9 Примеры графики](05_graphics/09_examples.md) | Пиксели, линии, спрайты, анимация |

### 06. Звук

| Раздел | Описание |
|--------|---------|
| [6.1 AY-3-8910](06_audio/01_ay_3_8910.md) | Регистры, каналы, огибающие |
| [6.2 COVOX](06_audio/02_covox.md) | DAC, порты #88/#89, частоты |
| [6.3 COVOX DMA](06_audio/03_covox_dma.md) | Стриминг, двойной буфер |
| [6.4 Примеры звука](06_audio/04_examples.md) | Тоны, мелодии, WAV, эффекты |

### 07. Диски

| Раздел | Описание |
|--------|---------|
| [7.1 Обзор дисков](07_disk/01_disk_overview.md) | IDE, FDD, дескрипторы, уровни доступа |
| [7.2 IDE](07_disk/02_ide.md) | Прямые порты, LBA, команды ATA |
| [7.3 FDD](07_disk/03_fdd.md) | WD1793, TR-DOS, форматы |
| [7.4 Разметка](07_disk/04_partition.md) | MBR, FAT16 структура, VBR |
| [7.5 Примеры диска](07_disk/05_examples.md) | Детектор, hex-дамп, копирование, MBR |

### 08. Периферия

| Раздел | Описание |
|--------|---------|
| [8.1 Клавиатура](08_peripherals/01_keyboard.md) | ZX матрица, PS/2 через SIO, коды клавиш |
| [8.2 Мышь](08_peripherals/02_mouse.md) | BIOS RST #30, Kempston Mouse |
| [8.3 ISA шина](08_peripherals/03_isa_bus.md) | Слот, порт #9FBD, доступ к ISA-устройствам |
| [8.4 CMOS/RTC](08_peripherals/04_cmos_rtc.md) | Часы, настройки, BCD |
| [8.5 SIO/PIO](08_peripherals/05_serial_parallel.md) | Z84C15 встроенная периферия, CTC таймеры |

### 09. Продвинутые темы

| Раздел | Описание |
|--------|---------|
| [9.1 Прерывания](09_advanced/01_interrupts.md) | Источники INT, формула INT_X, IM1/IM2, CTC, CBL, NMI |
| [9.2 Turbo](09_advanced/02_turbo.md) | 7/21 МГц, wait states, оптимизация |
| [9.3 Скрытые возможности](09_advanced/03_hidden_features.md) | SC, CNF, PN, FN_ACC, NMI |
| [9.4 ZX совместимость](09_advanced/04_zx_compat.md) | Spectrum, Pentagon, Scorpion, TRD/SNA |
| [9.5 VSync практика](09_advanced/05_vsync_practice.md) | Джиттер клавиатуры, tearing, waitVsync, SDK обработчик, CTC+VSync |

### 10. Уроки (туториалы)

| Раздел | Описание |
|--------|---------|
| [10.1 Hello World](10_tutorials/01_hello_world.md) | Первая программа, сборка, запуск |
| [10.2 Файловый браузер](10_tutorials/02_file_browser.md) | Листинг каталога, навигация |
| [10.3 Sprite engine](10_tutorials/03_sprite_engine.md) | 16×16 спрайты, double buffer, акселератор |
| [10.4 Музыкальный плеер](10_tutorials/04_music_player.md) | AY, прерывания, PT3 |

---

## Быстрый старт

### Минимальная программа

```asm
; hello.asm — вывод сообщения на экран
; Сборка: sjasmplus --raw=hello.exe hello.asm

        org     #8100 - 512     ; EXE заголовок
        db      "EXE", 0
        dw      #0200           ; версия
        dw      #0200           ; offset
        dw      0               ; loader
        dw      #8100           ; load addr
        dw      main            ; entry
        dw      #BFFF           ; stack
        ds      512 - 16

        org     #8100
main:
        ld      hl, msg
        ld      c, #5C          ; DSS PChars
        rst     #10

        ld      c, #30          ; WaitKey
        rst     #10

        ld      c, #41          ; Exit
        rst     #10

msg:    db      "Hello, Sprinter!", 13, 10, 0
```

### Ключевые порты (шпаргалка)

```
#82, #A2, #C2, #E2  — переключение окон памяти (WIN0–WIN3)
#10–#13             — CTC таймеры
#18, #19            — SIO-A (PS/2 клавиатура)
#58                 — Kempston Mouse (кнопки)
#88, #89            — COVOX DAC / режим
#8C, #8D, #8E       — AY-3-8910 (чтение, адрес, данные)
#8F                 — ROM_RG (Flash управление)
#89                 — PORT_Y / RGADR (палитра)
#C3                 — ALL_MODE (видеорежим)
#C9                 — RGMOD (double buffer)
#FE                 — ZX клавиатура + бордюр

RST #08             — BIOS API (C = функция)
RST #10             — DSS API (C = функция)
RST #30             — Mouse API
```

### Частые функции

| Функция | RST | C | Назначение |
|---------|-----|---|-----------|
| Print string | #10 | #5C | HL = ASCIIZ строка |
| Wait key | #10 | #30 | → A = код |
| Scan key | #10 | #31 | неблок, Z=1 → нет |
| Exit | #10 | #41 | выход из программы |
| Open file | #10 | #11 | HL = имя, A = mode → A = handle |
| Read file | #10 | #13 | A = handle, HL = buf, DE = len |
| Get mem | #10 | #3D | B = pages → A = block ID |
| Set video | #10 | #50 | A = mode (0/1/2) |
| Alloc mem | #08 | #C2 | B = pages → A = ID |
| Read sector | #08 | #55 | A = диск, HL:DE = LBA |
| Mouse state | #30 | #03 | → A = buttons, HL = X, DE = Y |

---

## Источники

| Репозиторий | URL |
|-------------|-----|
| BIOS исходники | https://gitlab.com/mikhaltchenkov/bios |
| DSS исходники | https://gitlab.com/mikhaltchenkov/dos |
| Эмулятор SprintEm | https://gitlab.com/nedopc/sprintem |
| SDK (ENgineE777) | https://github.com/ENgineE777/zx-sprinter-sdk |
| Flappybird пример | https://github.com/witchcraft2001/flappybird |
| FlexNavigator (файловый менеджер) | https://github.com/witchcraft2001/flexnavigator |
| Sprinter TextEditor | https://github.com/witchcraft2001/sprinter-texteditor |
| GfxView | https://github.com/witchcraft2001/sprinter-gfxview |
| Неофициальный сайт | http://sprinter.nedopc.org |
| FPGA исходники | `sprinter_ai_doc/sp-altera-src-2025-11-30/` |

---

## Об этом руководстве

Это руководство создано на основе анализа открытых исходников:
- **FPGA**: ACEX 1K30 top-level (SP2_ACEX.TDF), DCP (DCP.TDF/MIF), акселератор (ACCELER.TDF), видео (VIDEO2.TDF), AY (AY.TDF)
- **BIOS**: сборка 2.17/3.00, драйвер IDE (HDRIVER6.ASM), SETUP (DSETUP.ASM)
- **DSS**: DSS.INC + эмулятор в sprintem/bios.cpp
- **Примеры приложений**: flappybird (полная игра), flexnavigator (GUI), texteditor, gfxview
- **SDK**: zx-sprinter-sdk — шаблоны и библиотеки

Документация покрывает **практическое программирование** на Z80 ASM для Sprinter: от Hello World до полноценных игр со спрайтами, звуком и двойной буферизацией.

---

## Условные обозначения

- Hex значения: `#xxxx` или `0xXXXX`
- Бинарные: `%10101010` или `B"10101010"`
- RST вызовы: `RST #08` (BIOS), `RST #10` (DSS), `RST #30` (Mouse)
- Регистры Z80: стандартные (A, B, C, D, E, H, L, IX, IY, AF', BC', DE', HL')
- Комментарии: `;`
- Метки с точкой (`.local`) — локальные внутри функции
