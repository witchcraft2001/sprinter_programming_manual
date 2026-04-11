# 3.3 BIOS: работа с дисками

> **Навигация:** [← 3.2 BIOS API](02_bios_api.md) | [Оглавление](../README.md) | [3.4 Конфигурация →](04_bios_config.md)

---

## Модель дисков в BIOS

BIOS Sprinter поддерживает следующие типы устройств:

| Номер | Тип | Описание |
|-------|-----|----------|
| `#00` | FDD 0 | Дисковод A: (первый) |
| `#01` | FDD 1 | Дисковод B: (второй) |
| `#80` | IDE 0 Master | HDD/CD-ROM основной |
| `#81` | IDE 0 Slave | HDD/CD-ROM подчинённый |
| `#82` | IDE 1 Master | (если второй IDE контроллер) |
| `#83` | IDE 1 Slave | — |

Диски определяются при старте (функция `DRV_DETECT #57`) и их параметры хранятся в BIOS дескрипторах RAM.

### Дескрипторы дисков в памяти

| Адрес | Дескриптор |
|-------|-----------|
| `#C1C0` | IDE 0 (master) |
| `#C1C8` | IDE 1 (slave) |
| `#C1E0` | FDD 0 |
| `#C1E8` | FDD 1 |

Структура дескриптора (8 байт):
```
offset 0: DRVHD_H  — Drive/Head (старший бит = slave)
offset 1: SC_PT_H  — секторов на трек
offset 2: HEADS_H  — количество головок
offset 3: CYL_L_H  — цилиндры (младший)
offset 4: CYL_H_H  — цилиндры (старший)
offset 5: SPCLL_H  — write precomp (младший)
offset 6: SPCLH_H  — write precomp (старший)
offset 7: TYPE_H   — тип: 0=нет, 1=HDD, 2=CD-ROM
```

---

## Обнаружение дисков

```asm
Bios.Drv_Detect EQU     #57

; Обнаружить все устройства (вызывается BIOS при старте)
        ld      c, Bios.Drv_Detect
        rst     #08
        ; После вызова дескрипторы C1C0/C1C8/C1E0/C1E8 заполнены
```

---

## Получение списка доступных дисков

```asm
Bios.Drv_List   EQU     #5F

; Получить список номеров активных дисков
        ld      hl, disk_list
        ld      c, Bios.Drv_List
        rst     #08
        ; disk_list содержит список номеров, терминирован #FF

disk_list:  ds  16          ; буфер на 16 дисков
```

---

## Параметры диска

```asm
Bios.Drv_Get_Par EQU    #58

; Получить геометрию диска #80 (IDE master)
        ld      a, #80
        ld      c, Bios.Drv_Get_Par
        rst     #08
        ; BC, DE, HL — геометрия (точный формат зависит от прошивки BIOS)
```

Для прямого чтения из дескриптора в памяти:

```asm
; Прочитать геометрию IDE 0 master из дескриптора C1C0
        in      a, (EmmWin.P3)
        push    af
        ld      a, #FE              ; системная страница
        out     (EmmWin.P3), a

        ld      hl, #C1C0 - #C000 + #C000   ; смещение в системной странице
        ; эквивалент: ld hl, #C1C0 (системная страница уже WIN3)
        ld      a, (hl)             ; DRVHD_H
        inc     hl
        ld      a, (hl)             ; SC_PT_H
        inc     hl
        ld      a, (hl)             ; HEADS_H
        ; ...

        pop     af
        out     (EmmWin.P3), a
```

---

## Чтение сектора (DRV_READ)

```asm
Bios.Drv_Read   EQU     #55

; Прочитать 1 сектор с LBA=100 диска #80 в буфер
ReadSector:
        ld      a, #80              ; диск IDE 0 master
        ld      hl, 100             ; LBA (младшие 16 бит)
        ld      de, 0               ; LBA (старшие 16 бит)
        ld      ix, sector_buf      ; буфер приёмник (512 байт)
        ld      b, 1                ; количество секторов
        ld      c, Bios.Drv_Read
        rst     #08
        jr      c, .error

        ; Успех: sector_buf содержит данные
        ret

.error:
        ; A = код ошибки
        ret

sector_buf:     ds  512
```

### Чтение нескольких секторов

```asm
; Прочитать 8 секторов (4 КБ) начиная с LBA=0
        ld      a, #80
        ld      hl, 0
        ld      de, 0
        ld      ix, bigbuf
        ld      b, 8                ; 8 секторов = 4096 байт
        ld      c, Bios.Drv_Read
        rst     #08

bigbuf:     ds  4096
```

> **Ограничение:** буфер приёмник должен целиком помещаться в одно окно 16 КБ. Для больших чтений используйте несколько вызовов с меньшими блоками.

---

## Запись сектора (DRV_WRITE)

```asm
Bios.Drv_Write  EQU     #56

; Записать 1 сектор на диск
WriteSector:
        ld      a, #80
        ld      hl, 100             ; LBA
        ld      de, 0
        ld      ix, source_buf
        ld      b, 1
        ld      c, Bios.Drv_Write
        rst     #08
        ret     nc                  ; успех
        ; ошибка
        ret
```

---

## Проверка сектора (DRV_VERIFY)

```asm
Bios.Drv_Verify EQU     #54

; Проверить, что сектор читается без ошибок (без загрузки в память)
        ld      a, #80
        ld      hl, 100
        ld      de, 0
        ld      b, 1
        ld      c, Bios.Drv_Verify
        rst     #08
```

---

## Сброс диска (DRV_RESET)

После критических ошибок IDE нужно сбросить устройство:

```asm
Bios.Drv_Reset  EQU     #51

        ld      a, #80              ; номер диска
        ld      c, Bios.Drv_Reset
        rst     #08
```

---

## Версия дисковой подсистемы

```asm
Bios.Ext_Version EQU    #5A

; Получить версию BIOS дисковой системы
        ld      c, Bios.Ext_Version
        rst     #08
        ; DE = версия в BCD формате (например #0232 → 2.32)
```

---

## Пример: полная процедура чтения загрузочного сектора

```asm
; Прочитать MBR (LBA=0) с IDE 0 master и проверить сигнатуру

ReadMBR:
        ; Сначала сбросить диск
        ld      a, #80
        ld      c, #51              ; DRV_RESET
        rst     #08
        jr      c, .disk_error

        ; Прочитать сектор 0
        ld      a, #80
        ld      hl, 0
        ld      de, 0
        ld      ix, mbr_buf
        ld      b, 1
        ld      c, #55              ; DRV_READ
        rst     #08
        jr      c, .disk_error

        ; Проверить MBR сигнатуру (0xAA55 в конце сектора)
        ld      hl, mbr_buf + 510
        ld      a, (hl)
        cp      #55
        jr      nz, .not_mbr
        inc     hl
        ld      a, (hl)
        cp      #AA
        jr      nz, .not_mbr

        ; Успех: MBR валидный
        or      a                   ; CF=0
        ret

.disk_error:
.not_mbr:
        scf                         ; CF=1 — ошибка
        ret

mbr_buf:    ds  512
```

---

## Низкоуровневый доступ к IDE

BIOS внутренне использует порты IDE напрямую (см. раздел `01_architecture/04_port_map.md`):

- Запись: `HDW_DAT #0150`, `HDW_ERR #0151`, ..., `HDW_COM #4153`
- Чтение: `HDR_DAT #0050`, ..., `HDR_CTL #4053`

Для программы всегда предпочтительнее вызывать BIOS, а не работать с портами напрямую. Прямой доступ оправдан только в загрузчиках и оптимизированном коде.

---

## Обработка ошибок диска

Коды ошибок в `A` после `CF=1`:

| Код | Описание |
|-----|----------|
| `#01` | Неверные параметры |
| `#02` | CRC ошибка (FDD) |
| `#04` | Сектор не найден |
| `#10` | Ошибка ECC (HDD) |
| `#20` | Контроллер занят |
| `#40` | Seek ошибка |
| `#80` | Таймаут устройства |
| `#FF` | Общая ошибка |

---

## Ключевые моменты

> - Диски: `#00`–`#03` FDD, `#80`–`#83` IDE.
> - Основные функции: `DRV_READ #55`, `DRV_WRITE #56`, `DRV_VERIFY #54`, `DRV_RESET #51`.
> - LBA через HL (младшие) и DE (старшие).
> - `IX` — буфер приёмник, `B` — количество секторов.
> - Дескрипторы дисков хранятся по адресам `#C1C0`–`#C1E8` в системной странице.
> - Всегда сбрасывайте диск после критических ошибок.

---

## Ссылки и источники {#ссылки}

| Источник | Путь / URL |
|---------|-----------|
| HDRIVER6 — драйвер IDE | `sprinter_bios/SETUP/HDRIVER6.ASM` |
| Константы диска | `sprinter_bios/SETUP/HDRIVER6.ASM` (HDW_*, HDR_*, BSY, RDY) |
| Дескрипторы (C1C0, C1C8) | `sprinter_bios/SETUP/HDRIVER6.ASM` |
| BIOS функции диска | `espprobe/bios_equ.asm` (Bios.Drv_*) |
