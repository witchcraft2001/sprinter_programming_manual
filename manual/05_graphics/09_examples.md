# 5.9 Примеры графики

> **Навигация:** [← 5.8 Текстовый режим](08_text_mode.md) | [Оглавление](../README.md) | [6.1 AY-3-8910 →](../06_audio/01_ay_3_8910.md)

---

## Пример 1: минимальная графическая программа

```asm
; Установить режим 320×256, нарисовать пиксели, ждать клавишу, выйти

        org     #8100-512
        db      "EXE", 0
        dw      #0200, #0200, 0, #8100, main, #BFFF
        ds      512 - 16

        org     #8100
main:
        ; Установить режим 320×256
        ld      a, 1
        ld      b, 0
        ld      c, #50              ; Dss.SetVMod
        rst     #10

        ; Установить палитру: цвет 1 = белый
        ld      hl, 1
        ld      e, 63 : ld d, 63 : ld a, 63
        ld      c, #A4              ; Bios.SetPalette
        rst     #08

        ; Подключить первую страницу VRAM в WIN3
        ld      a, #50
        out     (#E2), a

        ; Нарисовать 100 белых пикселей в первой строке
        ld      hl, #C000
        ld      b, 100
.loop:
        ld      (hl), 1
        inc     hl
        djnz    .loop

        ; Ждать клавишу
        ld      c, #30              ; WaitKey
        rst     #10

        ; Вернуть видеорежим
        ld      a, 0
        ld      b, 0
        ld      c, #50
        rst     #10

        ; Выход
        ld      c, #41
        rst     #10
```

---

## Пример 2: рисование линии (Bresenham)

```asm
; Нарисовать линию по алгоритму Брезенхэма
; (упрощённый вариант для горизонтально-преобладающих линий)
; Вход: BC = X1, DE = Y1, HL = X2, IX = Y2, A = цвет

DrawLine:
        ; Вычислить dx = X2 - X1, dy = Y2 - Y1
        ; ... (стандартный Брезенхэм)
        ; На каждом шаге вызывать SetPixel
        ret

; Простая версия — горизонтальная линия
; Вход: HL = Y, DE = X1, BC = длина, A = цвет
DrawHLine:
        ; ... вычислить адрес пикселя (X1, Y)
        ; Заполнить BC байт значением A
        ret
```

---

## Пример 3: закрашенный прямоугольник (через акселератор)

```asm
; Нарисовать заполненный прямоугольник
; Вход: HL = X, DE = Y, BC = ширина, A = высота, L' = цвет

FillRect:
        push    af                  ; сохранить высоту
        ex      af, af'             ; A' = цвет
        ld      a, l
        ; Вычислить адрес начала в VRAM (только для первых 16 КБ)
        ; (упрощённо: предполагаем Y в пределах страницы #50)

        ex      de, hl              ; HL = Y, DE = X
        ld      h, 0
        add     hl, hl
        add     hl, hl
        add     hl, hl              ; HL = Y * 8
        ; нужно Y * 320 — для простоты используем предвычисленную таблицу

        ; ... пропущено сложное вычисление адреса ...

        ; Подключить нужную страницу VRAM
        ld      a, #50
        out     (#E2), a

        ; Заполнение через акселератор
        pop     af                  ; A = высота
        ld      b, a
.row:
        push    bc
        push    hl

        di
        ld      d, d                ; установить счётчик
        ld      a, c                ; ширина (младший байт)
        ex      af, af'             ; A = цвет
        ld      c, c                ; заполнить
        ei

        pop     hl
        ; Перейти на следующую строку
        ld      bc, 320
        add     hl, bc

        pop     bc
        djnz    .row

        ld      b, b                ; выключить акселератор
        ret
```

---

## Пример 4: показать битмап (из flappybird)

```asm
; Показать битмап в VRAM
; BC = адрес битмапа (в RAM)
; HL = ширина строки (байт)
; DE = адрес назначения (смещение в VRAM)
; A  = Y координата
; A' = высота

ShowBitmap:
        ld      (.len), hl
        ld      h, b
        ld      l, c

        ex      af, af'
        ld      b, a
        in      a, (#A2)            ; EmmWin.P1
        push    af

        ld      a, #50              ; VPAGE_TILES
        out     (#A2), a            ; WIN1 = VRAM

        ex      af, af'             ; A = Y
.loop:
        push    bc
        push    de
        out     (#89), a            ; установить Y в PORT_Y
        ld      bc, 0
.len    equ     $-2
        ldir                        ; копировать строку
        pop     de
        pop     bc
        inc     a                   ; следующая строка
        djnz    .loop

        pop     af
        out     (#A2), a            ; восстановить WIN1
        ret
```

*Источник: `flappybird/src/grx_utils.asm`*

---

## Пример 5: прокрутка экрана

```asm
; Горизонтальная прокрутка экрана на 8 пикселей влево
; Простой способ: копировать каждую строку со смещением

ScrollLeft8:
        ld      a, #50              ; первая страница VRAM
        out     (#E2), a

        ld      hl, #C008           ; источник: +8 байт
        ld      de, #C000           ; назначение
        ld      b, 32               ; количество "полустрок" (320/8 = 40, но строки...)

        ; Упрощённо: копировать полосу 16 КБ (примерно первые строки)
        ld      bc, #3FF8
        ldir

        ; Очистить правую кромку
        ld      b, 8
        xor     a
.clear:
        ld      (de), a
        inc     de
        djnz    .clear

        ret
```

---

## Пример 6: цикл double buffering

```asm
        org     #8100

GameLoop:
.loop:
        ; 1. Определить "скрытый" буфер
        in      a, (#C9)
        and     1
        jr      z, .draw_b          ; если активен A (bit0=0) — рисуем в B

.draw_a:
        ld      a, #50              ; страницы экрана A
        jr      .setup
.draw_b:
        ld      a, #55              ; страницы экрана B
.setup:
        out     (#E2), a            ; WIN3 = первая страница буфера

        ; 2. Очистить буфер
        ld      hl, #C000
        ld      de, #C001
        ld      bc, #3FFF
        ld      (hl), 0
        ldir

        ; 3. Нарисовать сцену
        call    DrawScene

        ; 4. Ждать VSync
        halt

        ; 5. Flip
        in      a, (#C9)
        xor     1
        out     (#C9), a

        ; 6. Проверить клавишу ESC
        ld      c, #31              ; Dss.ScanKey
        rst     #10
        jr      z, .loop
        cp      27                  ; ESC
        jr      nz, .loop

        ; Выход
        ld      c, #41
        rst     #10

DrawScene:
        ; Нарисовать что-то... например движущийся пиксель
        ld      hl, ball_addr
        ld      a, (hl)
        ld      l, a
        ld      h, #C0              ; адрес в WIN3
        ld      (hl), 15             ; белый пиксель

        ; Обновить позицию
        ld      hl, ball_addr
        inc     (hl)
        ret

ball_addr: db 0
```

---

## Пример 7: текст на графическом экране

```asm
; Вывести символ в режиме 320×256 использую 8×8 шрифт
; Вход: B = X (0..39), C = Y (0..31), A = символ, E = цвет

PrintCharGfx:
        ; Вычислить адрес шрифта: font_data + A * 8
        push    bc
        push    af
        ld      h, 0
        ld      l, a
        add     hl, hl              ; *2
        add     hl, hl              ; *4
        add     hl, hl              ; *8
        ld      bc, font_data
        add     hl, bc              ; HL = указатель на данные символа
        pop     af
        pop     bc

        ; Вычислить адрес в VRAM: (Y*8) * 320 + X*8
        ; (упрощённо для первой страницы)
        ld      h, 0
        ld      l, c                ; L = Y
        ; ... H = Y*8, умноженное на 320...

        ; Для каждой из 8 строк символа:
        ; - прочитать байт шрифта (8 бит)
        ; - для каждого бита: если 1 → E, если 0 → 0
        ld      b, 8
.row_loop:
        ld      a, (hl)             ; байт строки
        inc     hl
        ; ... раскодировать 8 бит в 8 байт VRAM ...
        djnz    .row_loop
        ret

font_data:  ; 256 символов × 8 байт = 2048 байт стандартного шрифта
            ds 2048
```

---

## Пример 8: простая анимация (bouncing ball)

```asm
        org     #8100

        ; Инициализация: режим 320×256
        ld      a, 1
        ld      b, 0
        ld      c, #50
        rst     #10

        ; Палитра
        ld      hl, 1
        ld      e, 63 : ld d, 0 : ld a, 0   ; красный
        ld      c, #A4 : rst #08

        ; Начальные координаты и скорости
        ld      hl, 100             ; X
        ld      (ball_x), hl
        ld      hl, 50              ; Y
        ld      (ball_y), hl
        ld      a, 1                ; DX
        ld      (ball_dx), a
        ld      a, 1                ; DY
        ld      (ball_dy), a

.loop:
        ; Подключить буфер
        ld      a, #50
        out     (#E2), a

        ; Очистить экран
        ld      hl, #C000
        ld      de, #C001
        ld      bc, #3FFF
        ld      (hl), 0
        ldir

        ; Нарисовать мячик в (ball_x, ball_y)
        ld      hl, (ball_x)
        ld      de, (ball_y)
        ld      b, 1                ; цвет
        ld      c, #A1              ; Bios.Pic_Point
        rst     #08

        ; Обновить позицию
        ld      a, (ball_dx)
        ld      hl, (ball_x)
        ld      e, a
        ld      d, 0
        add     hl, de
        ld      (ball_x), hl

        ; Проверить границы X (0..319)
        ; ... (упрощённо пропущено) ...

        ; Ждать кадр
        halt

        ; Проверить клавишу
        ld      c, #31              ; ScanKey
        rst     #10
        jr      z, .loop

        ; Выход
        ld      a, 0
        ld      b, 0
        ld      c, #50
        rst     #10

        ld      c, #41
        rst     #10

ball_x:     dw 0
ball_y:     dw 0
ball_dx:    db 0
ball_dy:    db 0
```

---

## Пример 9: палитровый эффект (fade-in)

```asm
; Плавное появление изображения через палитру
FadeIn:
        ld      b, 0                ; level = 0
.fade_loop:
        push    bc

        ; Установить 16 основных цветов с масштабом B/63
        ld      c, 0                ; индекс цвета
.color_loop:
        push    bc
        push    bc

        ; R = base_R * B / 63 (упрощение)
        ld      h, 0
        ld      l, c
        push    hl                  ; индекс цвета
        ; ... вычисление R, G, B ...
        ld      e, b                ; R = level (мастер яркости)
        ld      d, 0
        ld      a, 0
        pop     hl
        push    bc
        ld      c, #A4
        rst     #08
        pop     bc
        pop     bc
        pop     bc

        inc     c
        ld      a, c
        cp      16
        jr      nz, .color_loop

        ; Ждать кадр
        halt

        pop     bc
        inc     b
        ld      a, b
        cp      64
        jr      nz, .fade_loop
        ret
```

---

## Пример 10: загрузка картинки из файла

```asm
; Загрузить сырой образ 320×256 (80000 байт) из файла и показать

LoadAndShow:
        ; Открыть файл
        ld      hl, filename
        ld      a, #01
        ld      c, #11              ; Open
        rst     #10
        jr      c, .err
        ld      (h_file), a

        ; Установить видеорежим 320×256
        ld      a, 1
        ld      b, 0
        ld      c, #50
        rst     #10

        ; Читать по страницам VRAM (16 КБ за раз)
        ld      b, 5                ; 5 страниц
        ld      c, #50              ; первая VRAM страница
.page_loop:
        push    bc

        ld      a, c
        out     (#E2), a            ; WIN3 = VRAM страница

        ld      a, (h_file)
        ld      hl, #C000
        ld      de, #4000           ; 16 КБ
        ld      c, #13              ; Read
        rst     #10

        pop     bc
        inc     c
        djnz    .page_loop

        ; Закрыть файл
        ld      a, (h_file)
        ld      c, #12              ; Close
        rst     #10

        ; Ждать клавишу
        ld      c, #30
        rst     #10

.err:
        ld      c, #41
        rst     #10

filename:   db "PICTURE.RAW", 0
h_file:     db 0
```

---

## Пример 11: режимы записи VRAM (прозрачность, теневая запись)

```asm
; Демонстрация режимов записи через биты T и S номера VRAM-страницы
; (см. 5.1 Обзор видео — Страницы VRAM)

VPAGE_TILES     EQU #50     ; T=0 S=0: обычная запись (VRAM + DRAM)
VPAGE_SHADOW    EQU #54     ; T=0 S=1: только VRAM (DRAM не меняется)
VPAGE_TRANSP    EQU #58     ; T=1 S=0: прозрачность #FF
VPAGE_SPRITES   EQU #5C     ; T=1 S=1: прозрачность + только VRAM

; --- Шаг 1: нарисовать фон через обычную запись ---
; Фон пишется и в VRAM, и в DRAM (S=0)
        ld      a, VPAGE_TILES      ; #50
        out     (#E2), a
        ld      a, 0                ; строка Y
        out     (#89), a            ; PORT_Y
        ld      hl, #C000
        ld      de, #C001
        ld      bc, 319
        ld      (hl), #20           ; залить цветом #20
        ldir

; --- Шаг 2: нарисовать спрайт с прозрачностью + только VRAM ---
; Спрайт пишется ТОЛЬКО в VRAM (S=1), DRAM сохраняет чистый фон.
; Байты #FF пропускаются (T=1) — фон просвечивает.
        ld      a, VPAGE_SPRITES    ; #5C
        out     (#E2), a
        ld      hl, sprite_data     ; данные: #FF = прозрачный
        ld      de, #C000 + 100     ; позиция X=100
        ld      bc, 16              ; ширина спрайта
        ldir                        ; #FF пропускаются аппаратно

; --- Шаг 3: стереть спрайт — восстановить фон из DRAM ---
; Переключаемся на страницу с S=0, читаем DRAM (где фон остался чистым)
; и копируем обратно в VRAM
        ld      a, VPAGE_TILES      ; #50 — обычная запись
        out     (#E2), a
        ; DRAM по адресу #C000+100 содержит чистый фон (#20),
        ; потому что спрайт был записан с S=1.
        ; Просто переписываем область заново:
        ld      hl, #C000 + 100
        ld      de, #C001 + 100
        ld      bc, 15
        ld      (hl), #20
        ldir                        ; спрайт стёрт, фон восстановлен

; --- Не забыть: PORT_Y в безопасную зону ---
        ld      a, #C0
        out     (#89), a

sprite_data:
        db  #FF, #FF, #01, #01, #01, #01, #FF, #FF
        db  #01, #01, #02, #02, #02, #02, #01, #01
        ; #FF = прозрачный, #01/#02 = цвета спрайта
```

---

## Пример 12: double buffering с выбором режима

```asm
RGMOD           EQU #C9

; Определить скрытый буфер и подключить нужную страницу
SetupHiddenBuffer:
        in      a, (RGMOD)
        and     1
        jr      z, .draw_to_b       ; активен A → рисуем в B

.draw_to_a:
        ld      a, #50              ; экран A, обычная запись
        jr      .setup
.draw_to_b:
        ld      a, #54              ; экран B, только VRAM
.setup:
        out     (#E2), a            ; WIN3 = скрытый буфер
        ret

; Нарисовать спрайт в скрытый буфер с прозрачностью
DrawSpriteToHidden:
        in      a, (RGMOD)
        and     1
        jr      z, .spr_b

.spr_a:
        ld      a, #5C              ; экран A, T=1 S=1
        jr      .spr_setup
.spr_b:
        ld      a, #5C + 1          ; экран B, T=1 S=1, PP=01 (#5D)
.spr_setup:
        out     (#E2), a
        ; ... LDIR спрайт ...
        ret

; Flip
FlipBuffer:
        in      a, (RGMOD)
        xor     1
        out     (RGMOD), a
        ret
```

---

## Ключевые моменты

> - Базовый поток: SetVMod → SetPalette → рисование в VRAM через окна → Flip/Exit.
> - Для статичных эффектов достаточно прямых `LDIR`; для игр — акселератор.
> - Double buffering: определить скрытый буфер → нарисовать → HALT → flip.
> - BIOS `PIC_POINT #A1` и `PIC_SET_PAL #A4` полезны для быстрых проб.
> - Для сложной графики используйте акселератор (`LD D,D`, `LD L,L`, `LD C,C`).
> - Картинки загружаются напрямую в VRAM через `Dss.Read` с переключением страниц.

---

## Ссылки и источники {#ссылки}

| Источник | Путь / URL |
|---------|-----------|
| Flappybird графические утилиты | https://github.com/witchcraft2001/flappybird — `src/grx_utils.asm` |
| SDK графика | https://github.com/ENgineE777/zx-sprinter-sdk — `sdk/src/sprinter/lib_startup.asm` |
| GfxView | https://github.com/witchcraft2001/sprinter-gfxview |
