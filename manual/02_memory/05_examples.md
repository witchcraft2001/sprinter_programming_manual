# 2.5 Практические примеры работы с памятью

> **Навигация:** [← 2.4 Flash](04_flash.md) | [Оглавление](../README.md) | [3.1 BIOS →](../03_bios/01_bios_overview.md)

---

## Пример 1: сохранение и восстановление страниц

Базовый паттерн для любой подпрограммы, которая временно переключает окна:

```asm
EmmWin.P0   EQU     #82
EmmWin.P1   EQU     #A2
EmmWin.P2   EQU     #C2
EmmWin.P3   EQU     #E2

; Сохранить одно окно (generic-вариант)
; C = порт окна (EmmWin.Px), HL = адрес буфера
SavePage:
        in      a, (c)
        ld      (hl), a
        ret

; Восстановить одно окно
; C = порт окна, HL = адрес буфера
RestorePage:
        ld      a, (hl)
        out     (c), a
        ret

; === Пример использования ===
MyRoutine:
        ld      c, EmmWin.P3
        ld      hl, saved_page3
        call    SavePage            ; сохранить WIN3

        ld      a, VPAGE_TILES      ; #50
        out     (EmmWin.P3), a      ; WIN3 = VRAM

        ; ... работа с VRAM через 0xC000–0xFFFF ...

        ld      c, EmmWin.P3
        ld      hl, saved_page3
        call    RestorePage         ; восстановить WIN3
        ret

saved_page3:    db  0
VPAGE_TILES     EQU #50
```

*Источник: `flappybird/src/sys_utils.asm` — SavePage/RestorePage*

---

## Пример 2: копирование между страницами

Копируем 16 КБ из страницы `src_page` в страницу `dst_page`:

```asm
; Вход:
;   A = номер страницы-источника
;   B = номер страницы-приёмника
;
; Использует WIN1 (0x4000) и WIN3 (0xC000) как временные окна.
; Сохраняет и восстанавливает оба.
;
CopyPage16K:
        push    af                      ; сохранить A (src)
        in      a, (EmmWin.P1)
        push    af                      ; сохранить старый WIN1
        in      a, (EmmWin.P3)
        push    af                      ; сохранить старый WIN3

        pop     af                      ; A = старый WIN3 (мусор, не нужен)
        ld      a, b
        out     (EmmWin.P3), a          ; WIN3 = dst_page

        pop     af                      ; A = старый WIN1 (мусор)
        pop     af                      ; A = src_page
        out     (EmmWin.P1), a          ; WIN1 = src_page

        ld      hl, #4000               ; WIN1: источник
        ld      de, #C000               ; WIN3: приёмник
        ld      bc, #4000               ; 16 384 байт
        ldir

        pop     af
        out     (EmmWin.P3), a          ; восстановить WIN3
        pop     af
        out     (EmmWin.P1), a          ; восстановить WIN1
        ret
```

---

## Пример 3: обращение к нескольким страницам RAM

Паттерн для последовательного обхода нескольких страниц:

```asm
; Очистить 8 страниц RAM (128 КБ) начиная со страницы 0
; Заполнить нулями

ClearRAM128:
        in      a, (EmmWin.P3)
        push    af                      ; сохранить WIN3

        ld      b, 8                    ; 8 страниц
        ld      c, 0                    ; начальная страница
.page_loop:
        ld      a, c
        out     (EmmWin.P3), a          ; переключить WIN3 на следующую страницу

        ld      hl, #C000               ; начало WIN3
        ld      de, #C001
        ld      bc, #3FFF               ; 16383 байт (ldir копирует от hl к de)
        ld      (hl), 0
        ldir                            ; заполнить 16384 - 1 байт нулём

        ld      a, c
        inc     a
        ld      c, a                    ; следующая страница

        ld      a, b
        dec     a
        ld      b, a
        jr      nz, .page_loop

        pop     af
        out     (EmmWin.P3), a          ; восстановить WIN3
        ret
```

---

## Пример 4: подключение VRAM в WIN1 (ShowBitmap)

Реальный пример из flappybird — вывод битмапа через WIN1 = VRAM:

```asm
; Показать строки битмапа в VRAM
; BC = ширина строки (байт)
; HL = адрес источника в RAM
; DE = адрес в VRAM (смещение от начала страницы)
; A  = начальная строка (Y-координата → номер цвета в порт #89)
; A' = количество строк

ShowBitmap:
        ld      (.line_len), bc         ; сохранить длину строки
        ld      h, b
        ld      l, c                    ; HL = адрес источника

        ex      af, af'
        ld      b, a                    ; B = количество строк
        in      a, (EmmWin.P1)
        push    af                      ; сохранить WIN1

        ld      a, #50                  ; VPAGE_TILES
        out     (EmmWin.P1), a          ; WIN1 = VRAM

        ex      af, af'                 ; A = Y (строка)

.row_loop:
        push    bc
        push    de
        out     (#89), a                ; записать номер строки в порт палитры
        ld      bc, 0
.line_len   equ $-2
        ldir                            ; скопировать строку
        pop     de
        pop     bc
        inc     a                       ; следующая строка
        djnz    .row_loop

        pop     af
        out     (EmmWin.P1), a          ; восстановить WIN1
        ret
```

*Источник: `flappybird/src/grx_utils.asm` — ShowBitmap*

---

## Пример 5: копирование с ускорителем (ShowBitmapAcc)

Вариант с аппаратным акселератором (blitter) — быстрее `LDIR`:

```asm
; Показать строку с акселератором
; A = ширина (число блоков)
; HL = адрес источника
; DE = адрес назначения в VRAM

ShowBitmapAccLine:
        di                              ; запрет прерываний (акселератор требует)

        ld      d, d                    ; LD D,D — установить размер блока акселератора
        ld      a, (block_size)         ; загрузить ширину в A (размер блока)
        ; A = количество 4-байтных блоков для akcselerator

        ld      l, l                    ; LD L,L — команда "копировать строку" акселератору

        ei                              ; разрешить прерывания

        ret

block_size:     db  20          ; ширина строки в блоках
```

> **Коды команд акселератора** (LD r,r перехватываются в M1-цикле):
>
> | Инструкция | Действие |
> |------------|---------|
> | `LD B,B` | Выключить акселератор |
> | `LD D,D` | Установить размер блока (A = размер) |
> | `LD C,C` | Заполнить блок (A = значение, HL = адрес) |
> | `LD E,E` | Вертикальное заполнение |
> | `LD L,L` | Копировать строку (HL→DE, A=размер) |
> | `LD A,A` | Вертикальное копирование |

*Источник: `flappybird/src/grx_utils.asm` — ShowBitmapAcc*

---

## Пример 6: работа с несколькими окнами одновременно

Ситуация: программа выполняется в WIN2 (`0x8000`), данные в WIN0 (`0x0000`), VRAM в WIN3 (`0xC000`), буфер в WIN1 (`0x4000`):

```asm
; Инициализация: настроить все окна для работы
InitMemoryLayout:
        ; WIN0 (0x0000–0x3FFF) = RAM страница 0 (данные/стек)
        ld      a, 0
        out     (EmmWin.P0), a

        ; WIN1 (0x4000–0x7FFF) = RAM страница 2 (буфер)
        ld      a, 2
        out     (EmmWin.P1), a

        ; WIN2 (0x8000–0xBFFF) = RAM страница 10 (код) — уже настроено DSS

        ; WIN3 (0xC000–0xFFFF) = VRAM страница #50
        ld      a, #50
        out     (EmmWin.P3), a

        ret
```

После этого:
- `(0x0000..0x3FFF)` — данные программы и стек
- `(0x4000..0x7FFF)` — буфер в RAM
- `(0x8000..0xBFFF)` — код (ORG 0x8100)
- `(0xC000..0xFFFF)` — VRAM (прямой доступ к видеобуферу)

---

## Пример 7: разделённый доступ к ROM и RAM в WIN0

По умолчанию BIOS отображает ROM в WIN0. После инициализации DSS WIN0 = RAM:

```asm
; Переключить WIN0 временно на ROM (BIOS страница #80) и прочитать строку
ReadFromBIOS:
        in      a, (EmmWin.P0)
        push    af                  ; сохранить WIN0 (RAM)

        ld      a, #80              ; ROM страница 0 (BIOS начало)
        out     (EmmWin.P0), a

        ; WIN0 теперь = ROM, но RST/CALL адреса в ROM тоже!
        ; Осторожно: прерывания могут вызвать RST #38 → ROM
        di
        ld      hl, #0100           ; адрес в ROM
        ld      de, bios_buf
        ld      bc, 16
        ldir
        ei

        pop     af
        out     (EmmWin.P0), a      ; восстановить WIN0 = RAM
        ret

bios_buf:   ds  16
```

> **Важно:** При WIN0 = ROM прерывания вектора `RST #38` (`0x0038`) попадут в ROM. Если используется IM2 с таблицей в RAM, прерывания через WIN0 будут работать через ROM-заглушки. Всегда выключайте прерывания (`DI`) при временном переключении WIN0 на ROM.

---

## Шаблон программы с корректным управлением памятью

Минимальный шаблон DSS-программы с правильной обработкой страниц:

```asm
        org     #8100-512           ; заголовок EXE

        db      "EXE", 0            ; сигнатура
        dw      #0200               ; версия
        dw      0, 0, 0, 0, 0
        dw      main                ; точка входа
        dw      main
        dw      #BFFF               ; стек
        ds      490

        org     #8100

main:
        ; Сохранить все текущие страницы
        in      a, (EmmWin.P0)
        ld      (save_p0), a
        in      a, (EmmWin.P1)
        ld      (save_p1), a
        in      a, (EmmWin.P2)
        ld      (save_p2), a
        in      a, (EmmWin.P3)
        ld      (save_p3), a

        ; ... основной код программы ...

        ; При выходе: восстановить страницы
        ld      a, (save_p0)
        out     (EmmWin.P0), a
        ld      a, (save_p1)
        out     (EmmWin.P1), a
        ld      a, (save_p2)
        out     (EmmWin.P2), a
        ld      a, (save_p3)
        out     (EmmWin.P3), a

        ; Выход в DSS
        ld      c, 0                ; функция #00 = выход
        rst     #10

save_p0:    db  0
save_p1:    db  0
save_p2:    db  0
save_p3:    db  0
```

---

## Ключевые моменты

> - **Всегда** сохраняйте страницы окон перед переключением, **всегда** восстанавливайте после.
> - При переключении WIN0 на ROM — выключить прерывания (`DI`).
> - WIN2 содержит исполняемый код — не переключайте его в середине выполнения.
> - Для эффективной работы с VRAM используйте акселератор (`LD L,L`, `LD C,C`), а не `LDIR`.
> - Системные страницы DSS (`#FE`, `#FC`, `#40`–`#4F`) — только для чтения, не переключайте на них окна.

---

## Ссылки и источники {#ссылки}

| Источник | Путь / URL |
|---------|-----------|
| SavePage / RestorePage | https://github.com/witchcraft2001/flappybird — `src/sys_utils.asm` |
| ShowBitmap (VRAM через WIN1) | https://github.com/witchcraft2001/flappybird — `src/grx_utils.asm` |
| Шаблон EXE-программы | https://github.com/witchcraft2001/flappybird — `src/include/head.asm` |
| SDK startup | https://github.com/ENgineE777/zx-sprinter-sdk — `sdk/src/sprinter/lib_startup.asm` |
