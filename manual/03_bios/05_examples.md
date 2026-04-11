# 3.5 Практические примеры BIOS API

> **Навигация:** [← 3.4 Конфигурация](04_bios_config.md) | [Оглавление](../README.md) | [4.1 DSS →](../04_dss/01_dss_overview.md)

---

## Пример 1: Hello World через BIOS

```asm
        org     #8100-512
        db      "EXE", 0
        dw      #0200
        dw      0, 0, 0, 0, 0
        dw      main
        dw      main
        dw      #BFFF
        ds      490

        org     #8100

main:
        ; Установить позицию курсора: строка 5, колонка 10
        ld      c, #84              ; LP_SET_PLACE
        ld      d, 5
        ld      e, 10
        rst     #08

        ; Напечатать строку
        ld      hl, msg
        ld      b, msg_len
        ld      e, #0F              ; атрибут: белый на чёрном
        ld      c, #85              ; LP_PRINT_LN
        rst     #08

        ; Выход в DSS
        ld      c, #41              ; EXIT (DSS)
        rst     #10

msg:        db  "Hello from BIOS!"
msg_len     equ $-msg
```

---

## Пример 2: выделение и использование RAM

```asm
        org     #8100

main:
        ; Выделить блок из 4 страниц (64 КБ)
        ld      c, #C2              ; EMM_FN2
        ld      b, 4
        rst     #08
        jr      c, .no_mem
        ld      (block_id), a       ; сохранить ID

        ; Получить физические номера страниц
        ld      a, (block_id)
        ld      hl, page_list
        ld      c, #C5              ; EMM_FN5
        rst     #08

        ; Подключить первую страницу блока в WIN3 и заполнить её
        ld      a, (page_list)      ; первая физическая страница
        out     (#E2), a

        ld      hl, #C000
        ld      de, #C001
        ld      bc, #3FFF
        ld      (hl), #FF
        ldir                        ; заполнить WIN3 значением #FF

        ; ... работа с данными ...

        ; Освободить блок
        ld      a, (block_id)
        ld      c, #C3              ; EMM_FN3
        rst     #08

.no_mem:
        ld      c, #41              ; DSS EXIT
        rst     #10

block_id:   db  0
page_list:  ds  16
```

---

## Пример 3: чтение сектора с диска

```asm
        org     #8100

main:
        ; Сброс IDE диска
        ld      a, #80
        ld      c, #51              ; DRV_RESET
        rst     #08

        ; Прочитать сектор LBA=0 (MBR)
        ld      a, #80              ; IDE 0 master
        ld      hl, 0
        ld      de, 0
        ld      ix, buf
        ld      b, 1
        ld      c, #55              ; DRV_READ
        rst     #08
        jr      c, .error

        ; Напечатать первые 16 байт как hex
        ld      hl, buf
        ld      b, 16
.hex_loop:
        ld      a, (hl)
        call    PrintHex            ; своя подпрограмма для hex-вывода
        inc     hl
        djnz    .hex_loop

.error:
        ld      c, #41
        rst     #10

; Печать A как 2 hex-цифры
PrintHex:
        push    af
        rra
        rra
        rra
        rra
        call    PrintNibble
        pop     af
PrintNibble:
        and     #0F
        cp      10
        jr      c, .digit
        add     a, 'A' - 10 - '0'
.digit: add     a, '0'
        ; Печать символа в A
        push    bc
        ld      c, #82              ; LP_PRINT_SYM
        ld      b, 1
        rst     #08
        pop     bc
        ret

buf:    ds  512
```

---

## Пример 4: графический пиксель

```asm
        org     #8100

main:
        ; Установить палитру: цвет 1 = ярко-красный (63, 0, 0)
        ld      hl, 1               ; индекс цвета
        ld      e, 63               ; R
        ld      d, 0                ; G
        ld      a, 0                ; B
        ld      c, #A4              ; PIC_SET_PAL
        rst     #08

        ; Нарисовать 100 пикселей по диагонали
        ld      hl, 100             ; X
        ld      de, 100             ; Y
        ld      b, 100              ; счётчик
.loop:
        push    bc
        push    hl
        push    de
        ld      b, 1                ; цвет 1 (красный)
        ld      c, #A1              ; PIC_POINT
        rst     #08
        pop     de
        pop     hl
        inc     hl                  ; X++
        inc     de                  ; Y++
        pop     bc
        djnz    .loop

        ld      c, #41
        rst     #10
```

---

## Пример 5: чтение мыши в цикле

```asm
        org     #8100

main:
        ; Инициализация мыши
        ld      c, #00
        rst     #30

.loop:
        ; Прочитать состояние мыши
        ld      c, #03              ; READ MOUSE STATE
        rst     #30
        ; A = кнопки, HL = X, DE = Y

        ld      (m_buttons), a
        ld      (m_x), hl
        ld      (m_y), de

        ; Если нажата левая кнопка — выход
        bit     0, a
        jr      nz, .exit

        ; Вывести координаты (упрощённо)
        ; ...

        ; Ждать кадра (через DSS или NOP-пауза)
        halt
        jr      .loop

.exit:
        ld      c, #41              ; EXIT
        rst     #10

m_buttons:  db  0
m_x:        dw  0
m_y:        dw  0
```

---

## Пример 6: очистка экрана BIOS

```asm
; Очистить текстовое окно с атрибутом (фон=синий, текст=белый)
        ld      c, #89              ; LP_CLS_WIN
        ld      d, 0                ; строка начала
        ld      e, 0                ; колонка начала
        ld      h, 25               ; высота
        ld      l, 80               ; ширина
        ld      b, #17              ; атрибут (1=синий фон, 7=белый текст)
        rst     #08
```

---

## Пример 7: версия BIOS

```asm
        org     #8100

main:
        ; Получить строку версии
        ld      hl, ver_buf
        ld      c, #EF              ; FN_VERSION
        rst     #08

        ; Получить версию в BCD
        ld      c, #5A              ; EXT_VERSION
        rst     #08
        ; DE = версия (например #0232 → 2.32)

        ld      (bcd_ver), de

        ; Показать строку версии
        ld      c, #84              ; LP_SET_PLACE
        ld      d, 0
        ld      e, 0
        rst     #08

        ld      hl, ver_buf
        ld      b, 40
        ld      e, #0F
        ld      c, #85              ; LP_PRINT_LN
        rst     #08

        ld      c, #41
        rst     #10

ver_buf:    ds  64
bcd_ver:    dw  0
```

---

## Пример 8: проверка Sprinter

```asm
; Определить, на какой модели мы работаем
DetectSprinter:
        ld      c, #F1              ; SPRINTER_2
        rst     #08
        ret     nc                  ; CF=0 → Sprinter-2 (подтверждено)

        ld      c, #F0              ; SPRINTER_1
        rst     #08
        ; CF=1 → не Sprinter-1
        ; CF=0 → Sprinter-1
        ret
```

---

## Пример 9: прямое чтение дескриптора диска

```asm
; Прочитать тип диска IDE 0 master (смещение 7 в дескрипторе C1C0)
GetIDE0Type:
        in      a, (#E2)            ; сохранить WIN3
        push    af
        ld      a, #FE              ; системная страница
        out     (#E2), a

        ld      a, (#C1C0 + 7)      ; TYPE_H: 0=нет, 1=HDD, 2=CD
        ld      (ide0_type), a

        pop     af
        out     (#E2), a
        ret

ide0_type:  db  0
```

---

## Пример 10: печать десятичного числа

```asm
; Напечатать 16-битное число HL как десятичное
PrintDec:
        ld      bc, -10000
        call    .digit
        ld      bc, -1000
        call    .digit
        ld      bc, -100
        call    .digit
        ld      c, -10
        call    .digit
        ld      c, -1
.digit:
        ld      a, '0'-1
.sub:   inc     a
        add     hl, bc
        jr      c, .sub
        sbc     hl, bc
        push    hl
        push    bc
        ld      c, #82              ; LP_PRINT_SYM
        ld      b, 1
        rst     #08
        pop     bc
        pop     hl
        ret
```

---

## Ключевые моменты

> - BIOS API через `RST #08` прост: номер функции в C, параметры в регистрах.
> - Для вывода текста используйте `#82` (LP_PRINT_SYM), `#85` (LP_PRINT_LN).
> - Графика через `#A1` (PIC_POINT) и `#A4` (PIC_SET_PAL).
> - Память: `#C2` выделить, `#C3` освободить, `#C4`/`#C5` получить физические страницы.
> - Диск: `#55` READ, `#56` WRITE, `#51` RESET, все LBA через HL+DE.
> - Мышь через `RST #30`, функция `#03` возвращает состояние.
> - Всегда завершайте программу через DSS `#41` (EXIT), не через RET.

---

## Ссылки и источники {#ссылки}

| Источник | Путь / URL |
|---------|-----------|
| Шаблон EXE | https://github.com/witchcraft2001/flappybird — `src/include/head.asm` |
| BIOS константы | `espprobe/bios_equ.asm` |
| SDK примеры | https://github.com/ENgineE777/zx-sprinter-sdk — `sdk/src/sprinter/` |
| BIOS эмулятор | `sprintem/bios.cpp` |
