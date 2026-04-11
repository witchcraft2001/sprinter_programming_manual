# 7.2 IDE — низкоуровневый доступ

> **Навигация:** [← 7.1 Обзор дисков](01_disk_overview.md) | [Оглавление](../README.md) | [7.3 FDD →](03_fdd.md)

---

## Порты IDE

IDE контроллер Sprinter использует нестандартные 16-битные Z80 порты с кодированием командных/данных регистров в старших битах адреса.

### Порты записи

| Порт | Имя | ATA регистр |
|------|-----|-------------|
| `#0150` | `HDW_DAT` | 1F0h Data |
| `#0151` | `HDW_ERR` | 1F1h Features |
| `#0152` | `HDW_CNT` | 1F2h Sector Count |
| `#0153` | `HDW_SEC` | 1F3h Sector Number (LBA 0–7) |
| `#0154` | `HDW_CLL` | 1F4h Cylinder Low (LBA 8–15) |
| `#0155` | `HDW_CLH` | 1F5h Cylinder High (LBA 16–23) |
| `#4152` | `HDW_DRV` | 1F6h Drive/Head (биты LBA 24–27) |
| `#4153` | `HDW_COM` | 1F7h Command |
| `#4154` | `P_HD3F6` | 3F6h Device Control |

### Порты чтения

| Порт | Имя | ATA регистр |
|------|-----|-------------|
| `#0050` | `HDR_DAT` | 1F0h Data |
| `#0051` | `HDR_ERR` | 1F1h Error |
| `#0052` | `HDR_CNT` | 1F2h Sector Count |
| `#0053` | `HDR_SEC` | 1F3h Sector Number |
| `#0054` | `HDR_CLL` | 1F4h Cylinder Low |
| `#0055` | `HDR_CLH` | 1F5h Cylinder High |
| `#4052` | `HDR_DRV` | 1F6h Drive/Head |
| `#4053` | `HDR_CTL` | 1F7h Status |
| `#4055` | `P_HD3F7` | 3F7h Alt Status |

### Константы (HDRIVER6.ASM)

```asm
HDW_DAT EQU #0150
HDW_ERR EQU #0151
HDW_CNT EQU #0152
HDW_SEC EQU #0153
HDW_CLL EQU #0154
HDW_CLH EQU #0155
HDW_DRV EQU #4152
HDW_COM EQU #4153

HDR_DAT EQU #0050
HDR_ERR EQU #0051
HDR_CNT EQU #0052
HDR_SEC EQU #0053
HDR_CLL EQU #0054
HDR_CLH EQU #0055
HDR_DRV EQU #4052
HDR_CTL EQU #4053

BSY     EQU 7
RDY     EQU 6
DRQ     EQU 3
ERR     EQU 0
```

---

## Чтение статуса

Поскольку порты 16-битные, для доступа к ним используется пара `BC` (16 бит) + `OUT (C), reg` или `IN reg, (C)`:

```asm
; Прочитать регистр статуса IDE
ReadStatus:
        ld      bc, HDR_CTL         ; BC = #4053
        in      a, (c)              ; A = статус
        ret
```

---

## Ожидание готовности

### Ожидание !BSY

```asm
WaitNotBusy:
        ld      bc, HDR_CTL
.loop:
        in      a, (c)
        bit     BSY, a
        jr      nz, .loop
        ret
```

### Ожидание DRQ

```asm
WaitDRQ:
        ld      bc, HDR_CTL
.loop:
        in      a, (c)
        bit     BSY, a
        jr      nz, .loop
        bit     DRQ, a
        jr      z, .loop
        ret
```

---

## Выбор диска

Диск выбирается битом 4 регистра `HDW_DRV`:
- bit 4 = 0 → master
- bit 4 = 1 → slave
- bits [3:0] = верхние биты LBA (27–24) в LBA режиме

```asm
SelectMaster:
        ld      bc, HDW_DRV
        ld      a, #E0              ; 11100000: LBA mode, master, heads=0
        out     (c), a
        ret

SelectSlave:
        ld      bc, HDW_DRV
        ld      a, #F0              ; 11110000: LBA mode, slave
        out     (c), a
        ret
```

---

## LBA — установка адреса сектора

```asm
; Установить LBA адрес: DEHL = 32-битный LBA
; (здесь упрощённо: только младшие 28 бит)
SetLBA:
        ; LBA [0..7] → HDW_SEC
        ld      bc, HDW_SEC
        ld      a, l
        out     (c), a

        ; LBA [8..15] → HDW_CLL
        ld      bc, HDW_CLL
        ld      a, h
        out     (c), a

        ; LBA [16..23] → HDW_CLH
        ld      bc, HDW_CLH
        ld      a, e
        out     (c), a

        ; LBA [24..27] → HDW_DRV (+ mode bits)
        ld      bc, HDW_DRV
        ld      a, d
        and     #0F
        or      #E0                 ; LBA mode, master
        out     (c), a
        ret
```

---

## Команды IDE

| Код | Команда |
|-----|---------|
| `#20` | READ SECTOR(S) |
| `#30` | WRITE SECTOR(S) |
| `#91` | INITIALIZE DEVICE PARAMETERS |
| `#EC` | IDENTIFY DEVICE |
| `#E7` | FLUSH CACHE |
| `#08` | RESET |

---

## Чтение одного сектора

```asm
; Прочитать сектор LBA (в DEHL) в буфер HL'
; Предполагается, что выбран диск и LBA установлен

ReadOneSector:
        call    WaitNotBusy

        ; Sector Count = 1
        ld      bc, HDW_CNT
        ld      a, 1
        out     (c), a

        ; Установить LBA
        ; ... (см. SetLBA выше) ...

        ; Команда READ
        ld      bc, HDW_COM
        ld      a, #20
        out     (c), a

        ; Ждать DRQ
        call    WaitDRQ

        ; Прочитать 256 слов = 512 байт
        ld      bc, HDR_DAT
        ld      hl, sector_buf
        ld      d, 0                ; счётчик (256)
.read_loop:
        ini                         ; читает (C), пишет в (HL), HL++, B--
        ini                         ; каждый IDE байт = один ini, 512 раз
        dec     d
        jr      nz, .read_loop

        ret

sector_buf: ds 512
```

> **Замечание:** используется инструкция `INI` (Z80 block input), которая читает из порта `(C)`, записывает по `(HL)`, увеличивает `HL` и декрементирует `B`. Для 512 байт нужно 512 `INI` или цикл.

---

## Запись сектора

```asm
WriteOneSector:
        call    WaitNotBusy

        ld      bc, HDW_CNT
        ld      a, 1
        out     (c), a

        ; ... установить LBA ...

        ld      bc, HDW_COM
        ld      a, #30              ; WRITE SECTOR
        out     (c), a

        call    WaitDRQ

        ; Записать 512 байт
        ld      bc, HDW_DAT
        ld      hl, sector_buf
        ld      e, 0
.write_loop:
        outi
        outi
        dec     e
        jr      nz, .write_loop

        call    WaitNotBusy
        ret
```

---

## IDENTIFY DEVICE

Команда `#EC` возвращает 512 байт информации об устройстве:

```asm
IdentifyDevice:
        call    SelectMaster

        call    WaitNotBusy

        ld      bc, HDW_COM
        ld      a, #EC
        out     (c), a

        call    WaitDRQ

        ; Читать 512 байт в буфер
        ld      bc, HDR_DAT
        ld      hl, id_buf
        ld      d, 0
.loop:  ini : ini : dec d : jr nz, .loop
        ret

id_buf:     ds 512

; Структура ID buffer (частично):
; [0]    = флаги конфигурации
; [2]    = количество цилиндров
; [6]    = количество головок
; [12]   = секторов на трек
; [20]   = серийный номер (20 байт, ASCII)
; [46]   = прошивка (8 байт)
; [54]   = модель (40 байт)
; [120]  = общее количество LBA секторов (32 бит)
```

---

## Ошибки

При `ERR=1` в статусе детали в регистре `HDR_ERR`:

| Бит | Имя | Описание |
|-----|-----|---------|
| 7 | BBK | Bad Block |
| 6 | UNC | Uncorrectable Error |
| 5 | MC | Media Changed |
| 4 | IDNF | Sector ID Not Found |
| 3 | MCR | Media Change Requested |
| 2 | ABRT | Aborted command |
| 1 | TK0NF | Track 0 Not Found |
| 0 | AMNF | Address Mark Not Found |

```asm
CheckError:
        ld      bc, HDR_CTL
        in      a, (c)
        bit     ERR, a
        jr      z, .ok              ; нет ошибки

        ld      bc, HDR_ERR
        in      a, (c)              ; A = код ошибки
        scf
        ret

.ok:
        or      a                   ; CF=0
        ret
```

---

## Soft Reset

```asm
SoftReset:
        ld      bc, #4154           ; P_HD3F6
        ld      a, #04              ; SRST bit
        out     (c), a
        ; небольшая задержка
        ld      b, 0
.d:     djnz    .d

        xor     a
        out     (c), a
        call    WaitNotBusy
        ret
```

---

## Практический совет

**Не используйте прямой доступ к IDE** в обычных программах. Это имеет смысл только в:
- Загрузчиках (до инициализации BIOS).
- Утилитах низкого уровня (форматирование, prep диска).
- Оптимизированных игровых движках с огромным потоком данных.

Для типичного приложения используйте `BIOS.Drv_Read #55` и `BIOS.Drv_Write #56` — они обрабатывают все частные случаи и ошибки.

---

## Ключевые моменты

> - IDE порты 16-битные, доступ через `OUT (C), reg` и `IN reg, (C)`.
> - Запись и чтение — разные порты: `HDW_xxx #015x` vs `HDR_xxx #005x`.
> - Статусные биты: BSY=7, RDY=6, DRQ=3, ERR=0.
> - LBA mode: `HDW_DRV` бит 6 = 1, LBA распределён по 4 регистрам.
> - Данные передаются через `INI`/`OUTI` (256 слов = 512 байт).
> - Для прикладных задач — BIOS API, для низкого уровня — прямой доступ.

---

## Ссылки и источники {#ссылки}

| Источник | Путь / URL |
|---------|-----------|
| HDRIVER6.ASM | `sprinter_bios/SETUP/HDRIVER6.ASM` |
| ATA спецификация | стандарт ATA-2 |
