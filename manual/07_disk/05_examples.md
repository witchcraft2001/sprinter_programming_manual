# 7.5 Примеры работы с диском

> **Навигация:** [← 7.4 Разделы](04_partition.md) | [Оглавление](../README.md) | [8.1 Клавиатура →](../08_peripherals/01_keyboard.md)

---

## Пример 1: детектор дисков

```asm
; Показать все доступные диски в системе

        org     #8100
main:
        ; Вызвать обнаружение
        ld      c, #57                  ; Bios.Drv_Detect
        rst     #08

        ; Прочитать дескрипторы IDE и FDD
        in      a, (#E2)
        push    af
        ld      a, #FE                  ; системная страница
        out     (#E2), a

        ; IDE 0 master
        ld      hl, ide0_label
        call    PrintStr
        ld      a, (#C1C0 + 7)          ; тип
        call    PrintDiskType
        call    NewLine

        ; IDE 0 slave
        ld      hl, ide1_label
        call    PrintStr
        ld      a, (#C1C8 + 7)
        call    PrintDiskType
        call    NewLine

        ; FDD 0
        ld      hl, fdd0_label
        call    PrintStr
        ld      a, (#C1E0 + 7)
        call    PrintDiskType
        call    NewLine

        pop     af
        out     (#E2), a                ; восстановить WIN3

        ; Ждать клавишу и выйти
        ld      c, #30
        rst     #10
        ld      c, #41
        rst     #10

PrintStr:
        ld      c, #5C
        rst     #10
        ret

PrintDiskType:
        or      a
        jr      z, .none
        cp      1
        jr      z, .hdd
        cp      2
        jr      z, .cd

        ld      hl, unknown_msg
        jr      PrintStr

.none:  ld      hl, none_msg : jr PrintStr
.hdd:   ld      hl, hdd_msg  : jr PrintStr
.cd:    ld      hl, cd_msg   : jr PrintStr

NewLine:
        ld      a, 13 : ld c, #5B : rst #10
        ld      a, 10 : ld c, #5B : rst #10
        ret

ide0_label: db "IDE 0 master: ", 0
ide1_label: db "IDE 0 slave : ", 0
fdd0_label: db "FDD A       : ", 0
none_msg:   db "not present", 0
hdd_msg:    db "HDD", 0
cd_msg:     db "CD-ROM", 0
unknown_msg:db "unknown", 0
```

---

## Пример 2: hex-дамп сектора

```asm
; Прочитать LBA 0 (MBR) и показать первые 256 байт в hex

DumpSector:
        ld      a, #80
        ld      hl, 0                   ; LBA
        ld      de, 0
        ld      ix, sec_buf
        ld      b, 1
        ld      c, #55                  ; DRV_READ
        rst     #08
        ret     c

        ; Напечатать 16 строк по 16 байт
        ld      hl, sec_buf
        ld      b, 16
.row:
        push    bc

        ; Позиция
        ld      a, b                    ; номер строки (16, 15, ...)
        neg
        add     a, 16
        ; ... упрощённо: пропускаем адрес

        ; 16 байт hex
        ld      b, 16
.byte_loop:
        ld      a, (hl)
        call    PrintHexByte
        ld      a, ' '
        ld      c, #5B
        rst     #10
        inc     hl
        djnz    .byte_loop

        ld      a, 13 : ld c, #5B : rst #10
        ld      a, 10 : ld c, #5B : rst #10

        pop     bc
        djnz    .row
        ret

PrintHexByte:
        push    af
        rra : rra : rra : rra
        and     #0F
        call    PrintHexDigit
        pop     af
        and     #0F
PrintHexDigit:
        cp      10
        jr      c, .dig
        add     a, 'A' - 10
        jr      .out
.dig:   add     a, '0'
.out:   push    af
        ld      c, #5B
        rst     #10
        pop     af
        ret

sec_buf:    ds 512
```

---

## Пример 3: поиск файла с заданным расширением

```asm
; Найти первый .BIN файл в текущем каталоге
; и запомнить его имя

FindFirstBIN:
        ld      de, fbuf
        ld      hl, mask
        ld      a, #FF
        ld      b, 0
        ld      c, #19                  ; F_First
        rst     #10
        ret

mask:       db "*.BIN", 0
fbuf:       ds 32
```

---

## Пример 4: сохранение снапшота RAM в файл

```asm
; Сохранить 48 КБ (страницы 0, 1, 2) в файл

SaveSnapshot:
        ; Создать файл
        ld      hl, snap_name
        ld      c, #0A                  ; Create
        rst     #10
        jr      c, .err
        ld      (h_file), a

        ; Сохранить текущие окна (чтобы переключать)
        in      a, (#E2)
        ld      (save_p3), a

        ld      b, 0                    ; страница 0
.page_loop:
        push    bc
        ld      a, b
        out     (#E2), a                ; подключить страницу в WIN3

        ld      a, (h_file)
        ld      hl, #C000
        ld      de, #4000               ; 16 КБ
        ld      c, #14                  ; Write
        rst     #10

        pop     bc
        inc     b
        ld      a, b
        cp      3
        jr      c, .page_loop

        ; Восстановить WIN3
        ld      a, (save_p3)
        out     (#E2), a

        ; Закрыть
        ld      a, (h_file)
        ld      c, #12
        rst     #10

.err:
        ret

snap_name:  db "SNAP.DAT", 0
h_file:     db 0
save_p3:    db 0
```

---

## Пример 5: копирование файла с прогрессом

```asm
; Копировать файл с выводом прогресса

CopyFile:
        ld      hl, src
        ld      a, 1
        ld      c, #11 : rst #10
        jr      c, .err
        ld      (hsrc), a

        ld      hl, dst
        ld      c, #0A : rst #10
        jr      c, .err1
        ld      (hdst), a

        ld      bc, 0                   ; счётчик копированных блоков
.loop:
        push    bc
        ld      a, (hsrc)
        ld      hl, buf
        ld      de, 1024
        ld      c, #13 : rst #10        ; Read

        ld      a, d : or e
        jr      z, .done

        push    de
        ld      a, (hdst)
        ld      hl, buf
        pop     de
        ld      c, #14 : rst #10        ; Write

        ; Показать прогресс
        ld      a, '.'
        ld      c, #5B : rst #10

        pop     bc
        inc     bc
        jr      .loop

.done:
        pop     bc
.err1:
        ld      a, (hdst)
        ld      c, #12 : rst #10
.err:
        ld      a, (hsrc)
        ld      c, #12 : rst #10
        ret

src:    db "SOURCE.BIN", 0
dst:    db "TARGET.BIN", 0
hsrc:   db 0
hdst:   db 0
buf:    ds 1024
```

---

## Пример 6: определение размера свободного места

```asm
; Получить количество свободного места на текущем диске
; (Через обход FAT или BIOS функции)

GetFreeSpace:
        ld      a, 0                    ; текущий диск
        ld      c, #03                  ; Dss.DskInfo
        rst     #10
        ; Результат в регистрах зависит от реализации DSS
        ret
```

---

## Пример 7: чтение и парсинг MBR

```asm
        org     #8100

main:
        ; Прочитать LBA 0
        ld      a, #80
        ld      hl, 0
        ld      de, 0
        ld      ix, mbr_buf
        ld      b, 1
        ld      c, #55 : rst #08
        jr      c, .err

        ; Проверить сигнатуру
        ld      a, (mbr_buf + 510)
        cp      #55
        jr      nz, .bad
        ld      a, (mbr_buf + 511)
        cp      #AA
        jr      nz, .bad

        ; Показать 4 раздела
        ld      hl, mbr_buf + 0x1BE
        ld      b, 4
        ld      c, 0                    ; номер раздела
.loop:
        push    bc
        push    hl

        ld      a, c
        add     a, '1'
        ld      c, #5B : rst #10
        ld      a, ':'
        ld      c, #5B : rst #10
        ld      a, ' '
        ld      c, #5B : rst #10

        ; Тип раздела (HL+4)
        pop     hl
        push    hl
        inc     hl : inc hl : inc hl : inc hl
        ld      a, (hl)
        call    PrintHex            ; печать как hex

        ld      a, 13 : ld c, #5B : rst #10
        ld      a, 10 : ld c, #5B : rst #10

        pop     hl
        ld      bc, 16              ; размер записи раздела
        add     hl, bc

        pop     bc
        inc     c
        djnz    .loop

        ; Ждать клавишу
        ld      c, #30 : rst #10

.bad:
.err:
        ld      c, #41 : rst #10

; (Реализация PrintHex опущена, см. ранее)
PrintHex:
        push    af
        rra : rra : rra : rra
        and     #0F
        call    PrintHexDig
        pop     af
        and     #0F
PrintHexDig:
        cp      10
        jr      c, .d
        add     a, 'A' - 10 - '0'
.d:     add     a, '0'
        push    af : ld c, #5B : rst #10 : pop af
        ret

mbr_buf:    ds 512
```

---

## Пример 8: чтение BOOT sector раздела

```asm
; Прочитать VBR первого раздела

ReadBootSector:
        ; Сначала прочитать MBR
        ld      a, #80
        ld      hl, 0
        ld      de, 0
        ld      ix, mbr_buf
        ld      b, 1
        ld      c, #55 : rst #08
        jr      c, .err

        ; Получить Starting LBA первого раздела (offset #1BE + 8)
        ld      hl, (mbr_buf + 0x1BE + 8)
        ld      de, (mbr_buf + 0x1BE + 10)

        ; Прочитать этот сектор
        ld      a, #80
        ld      ix, vbr_buf
        ld      b, 1
        ld      c, #55 : rst #08

.err:
        ret

mbr_buf:    ds 512
vbr_buf:    ds 512
```

---

## Пример 9: подсчёт файлов в каталоге

```asm
CountFiles:
        ld      bc, 0                   ; счётчик

        ld      de, fbuf
        ld      hl, mask
        ld      a, #FF
        push    bc
        ld      b, 0
        ld      c, #19                  ; F_First
        rst     #10
        pop     bc
        jr      c, .done
        inc     bc

.next:
        ld      de, fbuf
        push    bc
        ld      c, #1A                  ; F_Next
        rst     #10
        pop     bc
        jr      c, .done
        inc     bc
        jr      .next

.done:
        ; BC = количество файлов
        ret

mask:       db "*.*", 0
fbuf:       ds 32
```

---

## Пример 10: чтение файла в цикле (построчно)

```asm
; Читать текстовый файл и печатать построчно

PrintTextFile:
        ld      hl, filename
        ld      a, 1
        ld      c, #11 : rst #10
        jr      c, .err
        ld      (h), a

.read:
        ld      a, (h)
        ld      hl, buf
        ld      de, 80                  ; 80 байт
        ld      c, #13 : rst #10

        ld      a, d : or e
        jr      z, .done

        ; Напечатать
        push    de
        ld      hl, buf
        pop     de
.print:
        ld      a, (hl)
        push    hl
        push    de
        ld      c, #5B : rst #10
        pop     de
        pop     hl
        inc     hl
        dec     de
        ld      a, d : or e
        jr      nz, .print

        jr      .read

.done:
        ld      a, (h)
        ld      c, #12 : rst #10
.err:
        ret

filename:   db "README.TXT", 0
h:          db 0
buf:        ds 80
```

---

## Ключевые моменты

> - BIOS `DRV_READ #55` — основной способ чтения сектора.
> - Перед чтением MBR или VBR всегда проверяйте сигнатуру `#55AA`.
> - FAT16 тома проще читать через DSS API, чем парсить вручную.
> - Для больших файлов используйте `Move_FP #15` для seek.
> - При копировании файлов лучше буфер 1024+ байт (меньше оверхеда на вызовы).
> - Хекс-дампы и утилиты — хороший способ отладки дисковых операций.

---

## Ссылки и источники {#ссылки}

| Источник | Путь / URL |
|---------|-----------|
| BIOS функции | `espprobe/bios_equ.asm` |
| DSS файлы | `espprobe/dss_equ.asm` |
| FlexNavigator | https://github.com/witchcraft2001/flexnavigator |
