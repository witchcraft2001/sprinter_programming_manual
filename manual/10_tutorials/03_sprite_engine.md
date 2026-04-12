# 10.3 Туториал: спрайтовый движок

> **Навигация:** [← 10.2 Файловый браузер](02_file_browser.md) | [Оглавление](../README.md) | [10.4 Музыкальный плеер →](04_music_player.md)

---

## Цель

Создать простой спрайтовый движок:
- Рендер спрайтов 16×16 пикселей в режиме 320×256
- Использование акселератора для копирования
- Double buffering для плавной анимации
- Простейшая анимация (движущийся спрайт)

---

## Шаг 1: структура спрайта

Спрайт 16×16 в режиме 8bpp = **256 байт**:

```
sprite_data:
        ds 256              ; 16 × 16 пикселей, 1 байт = цвет
```

Каждый пиксель — индекс в палитре (0–255). Цвет `#FF` является **аппаратно прозрачным** при записи в VRAM-страницы с установленным битом 3 номера страницы (`#58`–`#5F`). В этом режиме **любая** запись (CPU через `LDIR`, `LD (HL),A` или акселератор) автоматически пропускает байты `#FF`, оставляя на экране предыдущий фон. Подробнее — см. ниже «Аппаратная прозрачность».

---

## Шаг 2: инициализация

```asm
        org     #8100 - 512
        db      "EXE", 0
        dw      #0200, #0200, 0, #8100, main, #BFFF
        ds      512 - 16

        org     #8100

main:
        ; Установить режим 320×256
        ld      a, 1
        ld      b, 0
        ld      c, #50              ; SetVMod
        rst     #10

        ; Установить палитру
        call    InitPalette

        ; Основной цикл
        call    GameLoop

        ; Вернуть ZX режим
        ld      a, 0
        ld      b, 0
        ld      c, #50
        rst     #10

        ld      c, #41
        rst     #10

InitPalette:
        ; Цвет 0 — чёрный (фон)
        ld      hl, 0
        ld      e, 0 : ld d, 0 : ld a, 0
        ld      c, #A4 : rst #08

        ; Цвет 1 — красный
        ld      hl, 1
        ld      e, 63 : ld d, 0 : ld a, 0
        ld      c, #A4 : rst #08

        ; Цвет 2 — зелёный
        ld      hl, 2
        ld      e, 0 : ld d, 63 : ld a, 0
        ld      c, #A4 : rst #08

        ; Цвет 3 — синий
        ld      hl, 3
        ld      e, 0 : ld d, 0 : ld a, 63
        ld      c, #A4 : rst #08

        ret
```

---

## Шаг 3: базовая отрисовка спрайта

```asm
; Нарисовать спрайт 16×16 в точке (X, Y)
; Вход: BC = X, DE = Y, HL = адрес спрайта
;
; Предполагается, что нужная страница VRAM уже подключена в WIN3

DrawSprite16:
        push    hl

        ; Вычислить адрес начала: Y * 320 + X + 0xC000 (base WIN3)
        ; Упрощение: работает только если (Y*320 + X) < 16384

        ex      de, hl              ; HL = Y, DE = X
        ld      b, h
        ld      c, l                ; BC = Y
        add     hl, hl              ; *2
        add     hl, hl              ; *4
        add     hl, hl              ; *8
        add     hl, hl              ; *16
        add     hl, hl              ; *32
        add     hl, hl              ; *64
        push    hl                  ; сохранить Y*64
        ld      h, b
        ld      l, c                ; HL = Y
        add     hl, hl              ; *2
        add     hl, hl              ; *4
        add     hl, hl              ; *8
        add     hl, hl              ; *16
        add     hl, hl              ; *32
        add     hl, hl              ; *64
        add     hl, hl              ; *128
        add     hl, hl              ; *256
        pop     bc                  ; BC = Y*64
        add     hl, bc              ; HL = Y*320

        add     hl, de              ; + X
        ld      bc, #C000           ; база WIN3
        add     hl, bc              ; HL = целевой адрес

        ex      de, hl              ; DE = dest
        pop     hl                  ; HL = sprite data

        ; Копировать 16 строк по 16 байт
        ld      b, 16
.row:
        push    bc
        push    de
        ld      bc, 16
        ldir
        pop     de
        ; Перейти на следующую строку VRAM
        ld      bc, 320
        ex      de, hl
        add     hl, bc
        ex      de, hl
        pop     bc
        djnz    .row

        ret
```

---

## Шаг 4: отрисовка через акселератор

Быстрая версия с акселератором:

```asm
DrawSpriteAcc:
        push    hl
        ; ... вычислить DE = VRAM target (как выше) ...
        pop     hl

        ld      b, 16               ; 16 строк
.row:
        push    bc
        push    de

        di
        ld      d, d                ; установить размер блока
        ld      a, 16
        ld      l, l                ; копировать строку
        ei

        pop     de
        ; HL += 16, DE += 320
        ld      bc, 16
        add     hl, bc
        ex      de, hl
        ld      bc, 320
        add     hl, bc
        ex      de, hl

        pop     bc
        djnz    .row

        ld      b, b                ; выключить акселератор
        ret
```

---

## Шаг 5: главный игровой цикл с double buffering

```asm
GameLoop:
        ; Начальная позиция спрайта
        ld      hl, 100
        ld      (sprite_x), hl
        ld      hl, 50
        ld      (sprite_y), hl

.loop:
        ; Определить скрытый буфер
        in      a, (#C9)
        and     1
        jr      z, .draw_a_hidden   ; если активен B — рисуем в A

.draw_b_hidden:
        ld      a, #55
        jr      .do_draw

.draw_a_hidden:
        ld      a, #50

.do_draw:
        out     (#E2), a            ; WIN3 = hidden buffer

        ; Очистить буфер
        ld      hl, #C000
        ld      de, #C001
        ld      bc, #3FFF
        ld      (hl), 0
        ldir

        ; Нарисовать спрайт
        ld      bc, (sprite_x)
        ld      de, (sprite_y)
        ld      hl, player_sprite
        call    DrawSprite16

        ; Обновить позицию
        ld      hl, (sprite_x)
        ld      a, (sprite_vx)
        ld      e, a
        xor     a
        rl      e
        sbc     a, a                ; знак расширение
        ld      d, a
        add     hl, de
        ld      (sprite_x), hl

        ; Границы X
        ld      a, h
        or      a
        jr      nz, .flip_vx
        ld      a, l
        cp      320 - 16
        jr      c, .y_ok
.flip_vx:
        ld      a, (sprite_vx)
        neg
        ld      (sprite_vx), a

.y_ok:
        ; Ждать VSync
        halt

        ; Flip buffer
        in      a, (#C9)
        xor     1
        out     (#C9), a

        ; Проверить клавишу
        ld      c, #31              ; ScanKey
        rst     #10
        jr      z, .loop

        cp      27                  ; ESC
        jr      nz, .loop

        ret

sprite_x:       dw 0
sprite_y:       dw 0
sprite_vx:      db 1
sprite_vy:      db 0

player_sprite:
        ; 16×16 спрайт (256 байт)
        ; Красный квадрат с зелёным центром
        db  0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0
        db  0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0
        db  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
        db  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
        db  1,1,1,1,2,2,2,2,2,2,2,2,1,1,1,1
        db  1,1,1,2,2,2,2,2,2,2,2,2,2,1,1,1
        db  1,1,1,2,2,2,2,2,2,2,2,2,2,1,1,1
        db  1,1,1,2,2,2,2,2,2,2,2,2,2,1,1,1
        db  1,1,1,2,2,2,2,2,2,2,2,2,2,1,1,1
        db  1,1,1,2,2,2,2,2,2,2,2,2,2,1,1,1
        db  1,1,1,2,2,2,2,2,2,2,2,2,2,1,1,1
        db  1,1,1,1,2,2,2,2,2,2,2,2,1,1,1,1
        db  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
        db  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
        db  0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0
        db  0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0
```

---

## Ограничения примера

1. **Только первая страница VRAM**: пример работает только если `sprite_y * 320 < 16384`, т.е. Y < 51. Для полного экрана нужно переключать VRAM страницы при рисовании.

2. **Аппаратная прозрачность** — используйте VRAM-страницы `#58`–`#5F` (бит 3 номера страницы = 1). При записи в эти страницы байт `#FF` **автоматически пропускается** — на экране остаётся предыдущий фон. Работает с любым типом записи (CPU `LDIR`, `LD (HL),A`, акселератор). Страницы `#50`–`#57` (бит 3 = 0) пишут все байты без проверки.

3. **Без спрайтов-коллизий**: простая анимация без обнаружения столкновений.

4. **Нет сохранения фона**: перерисовка всего экрана каждый кадр — не оптимально.

---

## Улучшения

### Прозрачность через проверку

```asm
DrawSpriteTransparent:
        ld      b, 16               ; 16 строк
.row:
        push    bc
        push    de
        push    hl

        ld      b, 16               ; 16 пикселей в строке
.pixel:
        ld      a, (hl)
        or      a
        cp      #FF
        jr      z, .skip            ; #FF = прозрачный (аппаратная конвенция)
        ld      (de), a
.skip:
        inc     hl
        inc     de
        djnz    .pixel

        pop     hl
        pop     de
        ld      bc, 16
        add     hl, bc
        ex      de, hl
        ld      bc, 320
        add     hl, bc
        ex      de, hl

        pop     bc
        djnz    .row
        ret
```

### Несколько спрайтов

Добавьте массив структур спрайтов:

```asm
MAX_SPRITES     equ 10

sprites:
        ; структура: X (2), Y (2), vx (1), vy (1), sprite_ptr (2)
        ds 10 * 8

NumSprites:     db 0

UpdateSprites:
        ld      a, (NumSprites)
        or      a
        ret     z
        ld      b, a
        ld      hl, sprites
.loop:
        push    bc
        push    hl
        call    UpdateOneSprite
        pop     hl
        ld      bc, 8               ; размер структуры
        add     hl, bc
        pop     bc
        djnz    .loop
        ret
```

---

## Ключевые моменты

> - Спрайт 16×16 в 8bpp = 256 байт.
> - Отрисовка через LDIR или акселератор (`LD L,L`).
> - Double buffering через RGMOD: halt → flip.
> - Полный экран требует переключения страниц VRAM при Y > 51.
> - Прозрачность `#FF`: страницы `#58`–`#5F` (бит 3=1) автоматически пропускают запись `#FF`. Работает с CPU и акселератором. На страницах `#50`–`#57` прозрачности нет.
> - Для игр с множеством спрайтов — массив структур спрайтов.

---

## Ссылки и источники {#ссылки}

| Источник | Путь / URL |
|---------|-----------|
| SDK sprite engine | https://github.com/ENgineE777/zx-sprinter-sdk — `sdk/src/sprinter/lib_sprites.asm` |
| Flappybird sprites | https://github.com/witchcraft2001/flappybird — `src/grx_utils.asm` |
| Acceлератор | раздел [5.6 Акселератор](../05_graphics/06_accelerator.md) |
