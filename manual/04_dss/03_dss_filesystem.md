# 4.3 Файловая система DSS

> **Навигация:** [← 4.2 DSS API](02_dss_api.md) | [Оглавление](../README.md) | [4.4 Память DSS →](04_dss_memory.md)

---

## Организация дисков

DSS использует **FAT16** файловую систему на всех типах носителей:

| Тип диска | Буквы | Максимальный размер |
|-----------|-------|---------------------|
| FDD | A:, B: | 1.44 МБ (720 КБ или 1.44 МБ) |
| HDD IDE | C:, D:, ... | 2 ГБ (FAT16 лимит) |
| CD-ROM | зависит от подключения | read-only |
| RAM-диск | R: (или другой) | зависит от RAM |

Текущий диск хранится в системной переменной DSS. Смена через функцию `CHDISK #01`.

---

## Имена файлов

DSS использует классический **8.3** формат:

```
NAME    .EXT
^^^^^^^^    ^^^
 8 симв    3 симв
```

Регистр игнорируется (имена приводятся к верхнему регистру). Пути используют обратный слеш `\`:

```
C:\DOCS\REPORT.TXT
A:\PROGS\GAME.EXE
```

---

## Атрибуты файлов

| Атрибут | Значение | Описание |
|---------|---------|----------|
| `FileAttrib.Normal` | `#00` | Обычный файл |
| `FileAttrib.RDOnly` | `#01` | Только чтение |
| `FileAttrib.Hidden` | `#02` | Скрытый |
| `FileAttrib.System` | `#04` | Системный |
| `FileAttrib.Label` | `#08` | Метка тома |
| `FileAttrib.Direc` | `#10` | Каталог |
| `FileAttrib.Arch` | `#20` | Архивный (изменялся) |

Атрибуты можно комбинировать через OR: `FileAttrib.System OR FileAttrib.Hidden` = `#06`.

---

## Чтение файла целиком

```asm
DssRst          EQU #10
Dss.Open        EQU #11
Dss.Read        EQU #13
Dss.Close       EQU #12

; Прочитать файл целиком в буфер (до 16 КБ)
ReadFullFile:
        ; Открыть файл
        ld      hl, filename
        ld      a, #01              ; FileMode.Read
        ld      c, Dss.Open
        rst     DssRst
        jr      c, .error
        ld      (handle), a

        ; Прочитать максимум 16 КБ
        ld      a, (handle)
        ld      hl, buffer
        ld      de, #4000           ; 16 КБ
        ld      c, Dss.Read
        rst     DssRst
        ; DE = реально прочитано байт
        ld      (bytes_read), de

        ; Закрыть файл
        ld      a, (handle)
        ld      c, Dss.Close
        rst     DssRst

        or      a                   ; CF=0
        ret

.error:
        scf
        ret

filename:       db "DATA.BIN", 0
handle:         db 0
bytes_read:     dw 0
buffer:         ds #4000
```

---

## Запись файла

```asm
Dss.Create      EQU #0A
Dss.Write       EQU #14

WriteFile:
        ; Создать (или перезаписать) файл
        ld      hl, filename
        ld      c, Dss.Create
        rst     DssRst
        jr      c, .error
        ld      (handle), a

        ; Записать данные
        ld      a, (handle)
        ld      hl, data
        ld      de, data_len
        ld      c, Dss.Write
        rst     DssRst

        ; Закрыть
        ld      a, (handle)
        ld      c, Dss.Close
        rst     DssRst

        or      a
        ret

.error:
        scf
        ret

filename:   db  "OUTPUT.TXT", 0
data:       db  "Sprinter DSS example", 13, 10
data_len    equ $-data
handle:     db  0
```

---

## Чтение файла блоками

Для файлов больше 16 КБ нужно читать частями:

```asm
Dss.Move_FP     EQU #15

; Прочитать файл размером >16 КБ в несколько буферов
ReadLargeFile:
        ld      hl, filename
        ld      a, #01
        ld      c, Dss.Open
        rst     DssRst
        jr      c, .error
        ld      (handle), a

        ld      bc, 0               ; счётчик блоков
.read_loop:
        push    bc

        ld      a, (handle)
        ld      hl, chunk_buf
        ld      de, #1000           ; 4 КБ за раз
        ld      c, Dss.Read
        rst     DssRst

        ; DE = реально прочитано
        ld      a, d
        or      e
        jr      z, .eof             ; 0 байт = конец файла

        ; ... обработать chunk_buf ...

        pop     bc
        inc     bc
        jr      .read_loop

.eof:
        pop     bc
        ld      a, (handle)
        ld      c, Dss.Close
        rst     DssRst
        ret

.error:
        scf
        ret

chunk_buf:  ds #1000                ; 4 КБ
handle:     db 0
```

---

## Перемещение файлового указателя (Seek)

```asm
; Переместиться на 100 байт от начала файла
        ld      a, (handle)
        ld      b, 0                ; SEEK_SET
        ld      hl, 0               ; старшие 16 бит смещения
        ld      ix, 100             ; младшие 16 бит
        ld      c, Dss.Move_FP
        rst     DssRst

; К концу файла
        ld      a, (handle)
        ld      b, 2                ; SEEK_END
        ld      hl, 0
        ld      ix, 0
        ld      c, Dss.Move_FP
        rst     DssRst
        ; HL:IX = размер файла
```

---

## Обход каталога (F_First / F_Next)

Структура записи каталога FAT (32 байта):

```
Смещение  Размер  Поле
──────────────────────────────────────
0         11     Имя файла 8.3 (padded)
11        1      Атрибуты
12        10     Резерв
22        2      Время (FAT формат)
24        2      Дата (FAT формат)
26        2      Стартовый кластер
28        4      Размер файла (32 бита)
```

### Листинг файлов каталога

```asm
Dss.F_First     EQU #19
Dss.F_Next      EQU #1A

; Показать все файлы в текущем каталоге
ListFiles:
        ld      de, find_buf
        ld      hl, mask
        ld      a, #FF              ; все атрибуты
        ld      b, 0
        ld      c, Dss.F_First
        rst     DssRst
        jr      c, .done

.next:
        ; Напечатать имя файла (первые 11 байт записи)
        ld      hl, find_buf
        ld      b, 11
.print_name:
        ld      a, (hl)
        push    bc
        push    hl
        ld      c, #5B              ; DSS PUTCHAR
        rst     DssRst
        pop     hl
        pop     bc
        inc     hl
        djnz    .print_name

        ; Перевод строки
        ld      a, 13
        ld      c, #5B
        rst     DssRst
        ld      a, 10
        ld      c, #5B
        rst     DssRst

        ; Следующий файл
        ld      de, find_buf
        ld      c, Dss.F_Next
        rst     DssRst
        jr      nc, .next

.done:
        ret

mask:       db  "*.*", 0
find_buf:   ds  32
```

### Фильтрация по атрибутам

```asm
; Только каталоги
        ld      a, FileAttrib.Direc     ; #10
        ld      c, Dss.F_First
        ; ...
```

---

## Размер файла

```asm
; Получить размер файла из структуры F_First
; find_buf[28..31] = 32-битный размер

GetFileSize:
        ld      de, find_buf
        ld      hl, filename
        ld      a, #FF
        ld      b, 0
        ld      c, Dss.F_First
        rst     DssRst
        jr      c, .not_found

        ; Размер файла в find_buf + 28
        ld      hl, find_buf + 28
        ld      e, (hl) : inc hl
        ld      d, (hl) : inc hl    ; DE = младшие 16 бит
        ld      c, (hl) : inc hl
        ld      b, (hl)             ; BC = старшие 16 бит
        or      a                   ; CF=0
        ret

.not_found:
        scf
        ret
```

---

## Навигация по каталогам

```asm
Dss.ChDir       EQU #1D
Dss.CurDir      EQU #1E
Dss.MkDir       EQU #1B
Dss.RmDir       EQU #1C

; Перейти в подкаталог DOCS
        ld      hl, docs_path
        ld      c, Dss.ChDir
        rst     DssRst

; Получить текущий путь
        ld      hl, cwd
        ld      c, Dss.CurDir
        rst     DssRst
        ; cwd содержит строку "\CURRENT\PATH"

; Создать каталог
        ld      hl, new_dir
        ld      c, Dss.MkDir
        rst     DssRst

; Вернуться в родительский каталог
        ld      hl, parent
        ld      c, Dss.ChDir
        rst     DssRst

docs_path:  db  "DOCS", 0
parent:     db  "..", 0
new_dir:    db  "BACKUP", 0
cwd:        ds  128
```

---

## Удаление и переименование файлов

```asm
Dss.Delete      EQU #0E
Dss.Rename      EQU #10

; Удалить файл
        ld      hl, filename
        ld      c, Dss.Delete
        rst     DssRst

; Переименовать
        ld      hl, old_name
        ld      de, new_name
        ld      c, Dss.Rename
        rst     DssRst

old_name:   db  "OLD.TXT", 0
new_name:   db  "NEW.TXT", 0
```

---

## Управление атрибутами

```asm
Dss.Attrib      EQU #16

; Получить атрибуты файла
        ld      hl, filename
        ld      a, #FF              ; #FF = прочитать, не изменять
        ld      c, Dss.Attrib
        rst     DssRst
        ; A = текущие атрибуты

; Установить "Read-only" + "Hidden"
        ld      hl, filename
        ld      a, FileAttrib.RDOnly OR FileAttrib.Hidden     ; #03
        ld      c, Dss.Attrib
        rst     DssRst
```

---

## Форматы дат и времени FAT

Стандартное FAT16 кодирование:

**Дата (16 бит):**
- биты [15:9] — год минус 1980
- биты [8:5] — месяц (1–12)
- биты [4:0] — день (1–31)

**Время (16 бит):**
- биты [15:11] — часы (0–23)
- биты [10:5] — минуты (0–59)
- биты [4:0] — секунды / 2 (0–29)

```asm
; Декодировать FAT дату из регистра HL
FatDateDecode:
        ; День = L & 0x1F
        ld      a, l
        and     #1F
        ld      (day), a

        ; Месяц = ((H<<8)|L) >> 5 & 0x0F
        ld      a, l
        rlca : rlca : rlca      ; сдвиг на 5
        ld      b, a
        ld      a, h
        rla
        rla
        rla
        and     #F0
        or      b
        and     #0F
        ld      (month), a

        ; Год = (H>>1) + 1980
        ld      a, h
        srl     a
        add     a, 1980 / 256   ; ... (упрощённо)
        ld      (year), a
        ret

day:    db 0
month:  db 0
year:   db 0
```

---

## Ключевые моменты

> - FAT16 с 8.3 именами, поддержка каталогов.
> - Открытие — `Open #11`, режимы: Read, Write, RW.
> - Чтение/запись порциями до 16 КБ (0 = максимум).
> - Каталог: F_First + F_Next, структура записи 32 байта.
> - Размер файла в байтах 28–31 записи F_First.
> - Навигация: ChDir, CurDir, MkDir/RmDir.
> - Атрибуты: комбинация Normal/RDOnly/Hidden/System/Dir/Arch.

---

## Ссылки и источники {#ссылки}

| Источник | Путь / URL |
|---------|-----------|
| DSS файловые функции | `sprintem/bios.cpp` — `Estex()` case `0x0A`–`0x1E` |
| Константы атрибутов | `espprobe/dss_equ.asm` |
