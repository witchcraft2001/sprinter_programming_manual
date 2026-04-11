# 7.3 FDD — дисковод

> **Навигация:** [← 7.2 IDE](02_ide.md) | [Оглавление](../README.md) | [7.4 Разметка →](04_partition.md)

---

## О контроллере FDD

FDD контроллер Sprinter совместим с **WD1793** — стандартным контроллером ZX Spectrum (TR-DOS) и Beta Disk Interface. Он реализован в **MAX7000** CPLD, что отличает его от основного FPGA ACEX.

| Параметр | Значение |
|---------|---------|
| Контроллер | WD1793-совместимый |
| Поддерживаемые форматы | 720 КБ (80 tracks × 9 sectors × 512) и 1.44 МБ (80 × 18 × 512) |
| FS | FAT (DSS) или TR-DOS |
| Физика | 3.5" flexible disk |

---

## Порты WG93 (WD1793)

Доступ через MAX7000 с адресами, закодированными в DCP:

| Z80 порт | Функция |
|---------|---------|
| `#1F` / `#0F` | WG93 Status (чтение) / Command (запись) |
| `#3F` | Track register |
| `#5F` | Sector register |
| `#7F` | Data register |
| `#FF` | System (`WR_PDOS`) |

---

## Типичная последовательность чтения сектора

```
1. Выбрать диск и сторону через системный порт
2. SEEK на нужный трек (команда 0x1X)
3. Ждать !BSY
4. Установить Sector register
5. READ SECTOR (команда 0x8X)
6. Ждать DRQ, читать байт из #7F
7. Повторять до конца сектора
8. Ждать !BSY, проверить статус
```

---

## Команды WD1793

Типы:
- **Type I** (Restore/Seek/Step/Step In/Out) — 0x0X–0x7X
- **Type II** (Read/Write Sector) — 0x8X–0xBX
- **Type III** (Read Address/Read Track/Write Track) — 0xCX–0xFX
- **Type IV** (Force Interrupt) — 0xDX

### Примеры команд

| Код | Команда |
|-----|---------|
| `#0B` | RESTORE (к треку 0) |
| `#1B` | SEEK |
| `#88` | READ SECTOR (single) |
| `#A8` | WRITE SECTOR (single) |
| `#F4` | WRITE TRACK |
| `#D0` | FORCE INTERRUPT |

---

## Биты статуса

Регистр статуса (`IN A, (#1F)`):

**Type I команды:**
| Бит | Имя | Описание |
|-----|-----|---------|
| 7 | NOT READY | Диск не готов |
| 6 | PROT | Write protect |
| 5 | HDLD | Head loaded |
| 4 | SEEK_ERR | Ошибка seek |
| 3 | CRC_ERR | CRC error |
| 2 | TRACK0 | Голова на треке 0 |
| 1 | INDEX | Index hole detected |
| 0 | BSY | Busy |

**Type II/III команды:**
| Бит | Имя |
|-----|-----|
| 7 | NOT READY |
| 6 | WRITE PROT |
| 5 | RTYPE | Record Type / Write Fault |
| 4 | RNF | Record Not Found |
| 3 | CRC_ERR |
| 2 | LOST_DATA |
| 1 | DRQ | Data Request |
| 0 | BSY |

---

## Базовый пример: рестор на трек 0

```asm
; Установить голову на трек 0

RestoreToTrack0:
        ; Сначала выбрать диск (через #FF или другой системный порт)
        ld      a, 0            ; диск A, сторона 0
        out     (#FF), a

        ; Команда RESTORE
        ld      a, #0B          ; RESTORE со скоростью 0
        out     (#1F), a

        ; Ждать !BSY
.wait:
        in      a, (#1F)
        bit     0, a
        jr      nz, .wait
        ret
```

---

## SEEK на трек

```asm
; Seek на трек A
SeekTrack:
        ; Записать новый трек в data register
        out     (#7F), a        ; целевой трек

        ; Команда SEEK
        ld      a, #1B
        out     (#1F), a

        ; Ждать !BSY
.wait:
        in      a, (#1F)
        bit     0, a
        jr      nz, .wait
        ret
```

---

## Чтение сектора (упрощённо)

```asm
; Прочитать один сектор в buffer (512 байт)
; Вход: A = номер сектора (1..9 или 1..18)
ReadFDDSector:
        out     (#5F), a        ; установить номер сектора

        ; Команда READ SECTOR
        ld      a, #88
        out     (#1F), a

        ld      hl, fdd_buf
        ld      bc, 512
.read_loop:
        in      a, (#1F)        ; статус
        bit     1, a            ; DRQ?
        jr      z, .check_bsy

        in      a, (#7F)        ; читать байт данных
        ld      (hl), a
        inc     hl
        dec     bc
        ld      a, b : or c
        jr      nz, .read_loop
        jr      .done

.check_bsy:
        bit     0, a            ; BSY?
        jr      nz, .read_loop  ; ещё занят
        ; иначе — ошибка или конец
.done:
        ret

fdd_buf:    ds 512
```

---

## Запись сектора

```asm
; Записать один сектор
; Вход: A = номер сектора, HL = буфер источника
WriteFDDSector:
        out     (#5F), a

        ld      a, #A8          ; WRITE SECTOR
        out     (#1F), a

        ld      bc, 512
.write:
        in      a, (#1F)
        bit     1, a            ; DRQ?
        jr      z, .write

        ld      a, (hl)
        out     (#7F), a
        inc     hl
        dec     bc
        ld      a, b : or c
        jr      nz, .write

.done:
        in      a, (#1F)
        bit     0, a            ; BSY
        jr      nz, .done
        ret
```

---

## Формат TR-DOS

Традиционный формат дискет ZX Spectrum — TR-DOS:
- 160 секторов (80 × 2 × 1 сторона) или 640 (80 × 2 × 2 стороны)
- Сектор размером 256 байт (необычно!)
- Каталог на треке 0
- До 128 файлов в каталоге

DSS **не поддерживает TR-DOS** напрямую как основную ФС, но BIOS содержит драйвер TR-DOS для загрузки игр.

---

## Формат FAT (DSS)

DSS использует стандартные **FAT12/FAT16** дискеты:
- 1.44 МБ: 2880 секторов × 512 байт
- Кластер 1 сектор
- Совместимо с PC

---

## Через BIOS / DSS

Прямая работа с WD1793 сложна и подвержена ошибкам. Практически всегда используйте BIOS:

```asm
; Чтение через BIOS (работает для FDD и HDD одинаково)
        ld      a, 0                ; диск 0 = FDD A
        ld      hl, 0               ; LBA младшие
        ld      de, 0               ; LBA старшие
        ld      ix, buf
        ld      b, 1                ; 1 сектор
        ld      c, #55              ; DRV_READ
        rst     #08
```

Для FDD LBA вычисляется из CHS:
```
LBA = (cyl * 2 + side) * sectors_per_track + (sector - 1)
```

BIOS выполняет это автоматически.

---

## Ограничения FDD

- **Медленный**: ~350 КБ/с максимум.
- **Ненадёжный**: CRC ошибки, изношенные дискеты.
- **Спиндл мотор**: должен раскрутиться (~500 мс при первом доступе).
- **Один физический диск за раз**: нельзя работать с A: и B: одновременно.

---

## Ключевые моменты

> - FDD контроллер = WD1793 в MAX7000.
> - Порты: `#1F` (статус/команда), `#3F` (track), `#5F` (sector), `#7F` (data), `#FF` (system).
> - Команды: Type I (seek), Type II (r/w sector), Type III (track), Type IV (interrupt).
> - DSS использует FAT16 на 1.44 МБ дискетах; BIOS знает TR-DOS для совместимости.
> - Для прикладных задач — всегда через BIOS (`DRV_READ #55`).
> - Прямая работа с WG93 — только в загрузчиках и формат-утилитах.

---

## Ссылки и источники {#ссылки}

| Источник | Путь / URL |
|---------|-----------|
| DCP порты FDD | `sp-altera-src-2025-11-30/altera/acex/K30/DCP.MIF` (диапазон `#10`) |
| MAX7000 FDD | `sp-altera-src-2025-11-30/altera/max/SP2_MAX.TDF` |
| WD1793 datasheet | Western Digital |
