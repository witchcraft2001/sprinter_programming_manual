# 7.4 Разделы и файловая система

> **Навигация:** [← 7.3 FDD](03_fdd.md) | [Оглавление](../README.md) | [7.5 Примеры диска →](05_examples.md)

---

## MBR — Master Boot Record

Первый сектор диска (LBA 0) содержит **MBR** — загрузочную запись с таблицей разделов.

### Структура MBR (512 байт)

```
Смещение  Размер  Поле
──────────────────────────────────────
0x000     446    Boot code (до кода загрузчика)
0x1BE     16     Partition 1 (primary)
0x1CE     16     Partition 2
0x1DE     16     Partition 3
0x1EE     16     Partition 4
0x1FE     2      Signature #55AA
```

### Запись раздела (16 байт)

```
Смещение  Размер  Поле
──────────────────────────────
0         1      Boot indicator (#80 = bootable)
1         3      Starting CHS
4         1      Partition type (#06 = FAT16, #0B = FAT32)
5         3      Ending CHS
8         4      Starting LBA
12        4      Number of sectors
```

---

## Чтение MBR

```asm
ReadMBR:
        ; Сбросить диск
        ld      a, #80              ; IDE 0 master
        ld      c, #51              ; DRV_RESET
        rst     #08

        ; Прочитать LBA 0
        ld      a, #80
        ld      hl, 0
        ld      de, 0
        ld      ix, mbr_buf
        ld      b, 1
        ld      c, #55              ; DRV_READ
        rst     #08
        jr      c, .err

        ; Проверить сигнатуру
        ld      hl, mbr_buf + 510
        ld      a, (hl)
        cp      #55
        jr      nz, .bad_mbr
        inc     hl
        ld      a, (hl)
        cp      #AA
        jr      nz, .bad_mbr

        ; MBR валиден — извлечь первый раздел
        ld      hl, mbr_buf + 0x1BE
        ld      a, (hl + 4)         ; partition type
        ld      (part_type), a

        ; Starting LBA (4 байта, little-endian)
        ld      hl, mbr_buf + 0x1BE + 8
        ld      e, (hl) : inc hl
        ld      d, (hl) : inc hl
        ld      c, (hl) : inc hl
        ld      b, (hl)
        ld      (part_start), de
        ld      (part_start + 2), bc

        ret

.err:
.bad_mbr:
        scf
        ret

mbr_buf:    ds 512
part_type:  db 0
part_start: dd 0
```

---

## Boot sector (VBR) — FAT16

После MBR идёт загрузочный сектор раздела — VBR (Volume Boot Record). Для FAT16 это также 512 байт.

### Структура FAT16 boot sector

```
Смещение  Размер  Поле
──────────────────────────────────────
0         3       JMP instruction
3         8       OEM name
11        2       Bytes per sector (обычно 512)
13        1       Sectors per cluster
14        2       Reserved sectors (обычно 1)
16        1       Number of FATs (обычно 2)
17        2       Root directory entries (обычно 512)
19        2       Total sectors (16-bit)
21        1       Media descriptor
22        2       Sectors per FAT
24        2       Sectors per track
26        2       Number of heads
28        4       Hidden sectors
32        4       Total sectors (32-bit)
36        1       Drive number
37        1       Reserved
38        1       Extended boot signature (#29)
39        4       Serial number
43        11      Volume label
54        8       File system type ("FAT16   ")
62        448     Boot code
510       2       Signature #55AA
```

### Константы из SDK

```asm
SEC_SIZE        EQU 11      ; смещение Bytes per sector
CLAST_SIZE      EQU 13      ; Sectors per cluster
RESERV_SECS     EQU 14      ; Reserved sectors
FATS_NUM        EQU 16      ; Number of FATs
FLS_NUM         EQU 17      ; Root entries
S_P_D           EQU 19      ; Total sectors 16
FORM_CODE       EQU 21      ; Media descriptor
S_P_F           EQU 22      ; Sectors per FAT
S_P_T           EQU 24      ; Sectors per track
H_P_S           EQU 26      ; Number of heads
SPECIAL_SECS    EQU 28      ; Hidden sectors
FAT_ID          EQU #36     ; FAT ID
```

*Источник: `zx-sprinter-sdk/sdk/src/sprinter/sprint00.asm`*

---

## Layout FAT16 тома

```
Сектор 0       Boot sector (VBR)
Сектор 1..N    Reserved (обычно только 0)
Сектор N+1..M  FAT 1
Сектор M+1..K  FAT 2
Сектор K+1..R  Root directory (фиксированный размер)
Сектор R+1..   Data area (кластеры)
```

### Вычисление адресов

```
FAT_start    = reserved_sectors
FAT_size     = sectors_per_FAT
ROOT_start   = FAT_start + FAT_size * num_FATs
ROOT_size    = (root_entries * 32) / 512
DATA_start   = ROOT_start + ROOT_size
Cluster_to_LBA(N) = DATA_start + (N - 2) * sectors_per_cluster
```

---

## Запись каталога FAT16 (32 байта)

```
Смещение  Размер  Поле
──────────────────────────────────
0         8       Имя файла
8         3       Расширение
11        1       Атрибуты
12        1       Reserved
13        1       Create time (tenths)
14        2       Create time
16        2       Create date
18        2       Last access date
20        2       Starting cluster high (FAT32, в FAT16 = 0)
22        2       Last modified time
24        2       Last modified date
26        2       Starting cluster
28        4       File size
```

---

## Чтение FAT

```asm
; Прочитать FAT таблицу в память
; part_start = LBA начала раздела
; reserved_sectors = оффсет до FAT

ReadFAT:
        ld      hl, (part_start)
        ld      de, (part_start + 2)
        ; LBA = part_start + reserved_sectors
        ld      bc, 1               ; обычно 1 reserved sector
        add     hl, bc
        jr      nc, .no_carry
        inc     de
.no_carry:

        ld      a, #80
        ld      ix, fat_buf
        ld      b, 1
        ld      c, #55              ; DRV_READ
        rst     #08
        ret

fat_buf:    ds 512
```

---

## Получение цепочки кластеров

В FAT16 каждая запись — 16 бит:

```asm
; Получить следующий кластер в цепочке
; Вход: HL = номер текущего кластера
; Выход: HL = следующий кластер (или #FFFF если конец)

GetNextCluster:
        ; Адрес в FAT = cluster * 2
        add     hl, hl              ; HL = offset in FAT (байт)

        ; Определить сектор и смещение внутри сектора
        ; sector_in_fat = offset / 512
        ; ... обычно кэшировать FAT в памяти ...

        ; Для простоты — предполагается, что FAT уже в fat_buf (<= 512 байт)
        ld      bc, fat_buf
        add     hl, bc              ; HL = адрес в RAM
        ld      e, (hl)
        inc     hl
        ld      d, (hl)
        ex      de, hl              ; HL = value
        ret
```

---

## Типы разделов

| Код | Имя |
|-----|-----|
| `#00` | Empty |
| `#01` | FAT12 |
| `#04` | FAT16 (< 32 MB) |
| `#06` | FAT16 (< 2 GB) |
| `#0B` | FAT32 |
| `#0C` | FAT32 LBA |
| `#0E` | FAT16 LBA |

DSS поддерживает **только FAT16** (`#06` или `#0E`).

---

## Создание файла через DSS

DSS прячет всю сложность FAT16 за API. Программе достаточно:

```asm
; Создать файл "NEW.TXT" и записать в него
        ld      hl, filename
        ld      c, #0A              ; Create
        rst     #10
        ld      (h), a

        ld      a, (h)
        ld      hl, data
        ld      de, data_len
        ld      c, #14              ; Write
        rst     #10

        ld      a, (h)
        ld      c, #12              ; Close
        rst     #10

filename:   db "NEW.TXT", 0
data:       db "Hello", 13, 10
data_len    equ $-data
h:          db 0
```

---

## Атрибуты FAT

| Бит | Значение | Имя |
|-----|---------|-----|
| 0 | `#01` | Read-only |
| 1 | `#02` | Hidden |
| 2 | `#04` | System |
| 3 | `#08` | Volume label |
| 4 | `#10` | Directory |
| 5 | `#20` | Archive |

---

## Удаление файла в FAT16

DSS делает это через `Dss.Delete #0E`. Физически:
1. Первый байт имени заменяется на `#E5` (removed).
2. Цепочка кластеров в FAT освобождается (значения `#0000`).

Восстановление возможно специальными утилитами, если файл был только что удалён.

---

## Ключевые моменты

> - MBR в LBA 0 содержит таблицу 4 разделов.
> - Каждый раздел FAT16 начинается с VBR.
> - FAT16 layout: reserved → FAT1 → FAT2 → root dir → data.
> - Запись каталога 32 байта: имя 8.3, атрибуты, стартовый кластер, размер.
> - DSS работает с FAT16 (тип `#06`), используйте API не прямой доступ.
> - Только FAT12/FAT16, без FAT32 и NTFS.

---

## Ссылки и источники {#ссылки}

| Источник | Путь / URL |
|---------|-----------|
| FAT константы SDK | `zx-sprinter-sdk/sdk/src/sprinter/sprint00.asm` |
| DSS файловые функции | `sprintem/bios.cpp` — Estex case `0x0A`–`0x1E` |
| FAT16 спецификация | Microsoft / общедоступная |
