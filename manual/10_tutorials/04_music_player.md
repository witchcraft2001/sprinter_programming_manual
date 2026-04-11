# 10.4 Туториал: музыкальный плеер (AY)

> **Навигация:** [← 10.3 Sprite engine](03_sprite_engine.md) | [Оглавление](../README.md)

---

## Цель

Создать простой плеер музыки:
- Загрузка AY или PT3 файла
- Воспроизведение через AY-3-8910
- Остановка по нажатию клавиши
- Вызов плеера из обработчика прерывания 50 Гц

---

## Формат PT3 (кратко)

**PT3** (ProTracker 3) — самый популярный формат трекерной музыки для ZX Spectrum AY. Файл содержит:
- Заголовок с мета-данными
- Pattern-данные (что играть)
- Sample-данные (огибающие, эффекты)

Полный парсер PT3 сложен (~5000 строк кода). Для туториала используем **готовый плеер** из SDK или простой AY-трекер.

---

## Стратегия: встроенный плеер

SDK содержит плеер в `zx-sprinter-sdk/sdk/src/wyzplayer/ayfxplay.asm`. Мы рассмотрим упрощённый подход — ручное воспроизведение простой мелодии через AY.

---

## Шаг 1: структура плеера

```asm
; Простой AY-плеер: проигрывает массив регистров
; Каждый кадр (50 Гц) плеер записывает новые значения в 14 регистров AY

player_data:
        ; Таблица: 14 байт × N кадров
        ; Каждый кадр = значения R0..R13
        ; ... предвычисленные данные ...

player_pos:     dw 0        ; текущая позиция в player_data
player_len:     dw 0        ; длина в кадрах
player_active:  db 0        ; 1 = играет
```

---

## Шаг 2: функция play_frame

```asm
; Проиграть один кадр: записать 14 регистров AY из player_data
; Вызывать из обработчика прерывания 50 Гц

AY_Frame:
        ld      a, (player_active)
        or      a
        ret     z

        ld      hl, (player_pos)
        ld      bc, player_data
        add     hl, bc

        ; Записать R0..R13 (14 регистров)
        ld      b, 0                ; номер регистра
        ld      c, 14               ; счётчик
.reg_loop:
        ld      a, b                ; выбрать регистр
        out     (#8D), a            ; AY register select
        ld      a, (hl)             ; значение
        out     (#8E), a            ; AY data

        inc     hl
        inc     b
        dec     c
        jr      nz, .reg_loop

        ; Обновить позицию
        ld      hl, (player_pos)
        ld      bc, 14
        add     hl, bc
        ld      (player_pos), hl

        ; Проверить конец
        ld      bc, (player_len)
        or      a
        sbc     hl, bc
        jr      c, .not_end

        ; Loop: начать снова
        ld      hl, 0
        ld      (player_pos), hl

.not_end:
        ret
```

---

## Шаг 3: инициализация плеера

```asm
PlayerStart:
        ; Сбросить позицию
        ld      hl, 0
        ld      (player_pos), hl

        ; Пометить как активный
        ld      a, 1
        ld      (player_active), a

        ; Установить mixer AY: все тона включены
        ld      a, 7
        out     (#8D), a
        ld      a, #F8              ; 11111000
        out     (#8E), a
        ret

PlayerStop:
        xor     a
        ld      (player_active), a

        ; Выключить звук
        ld      a, 7 : out (#8D), a
        ld      a, #FF : out (#8E), a

        ld      a, 8 : out (#8D), a
        xor     a : out (#8E), a
        ld      a, 9 : out (#8D), a
        xor     a : out (#8E), a
        ld      a, 10 : out (#8D), a
        xor     a : out (#8E), a
        ret
```

---

## Шаг 4: обработчик прерывания

```asm
; Устанавливаем IM2 с собственным обработчиком

SetupIM2:
        ; Заполнить таблицу векторов
        ld      hl, #FE00
        ld      de, #FE01
        ld      bc, 256
        ld      (hl), #FD
        ldir

        ; Jump в обработчик
        ld      a, #C3              ; JP opcode
        ld      (#FDFD), a
        ld      hl, IntHandler
        ld      (#FDFE), hl

        ld      a, #FE
        di
        ld      i, a
        im      2
        ei
        ret

IntHandler:
        push    af
        push    bc
        push    hl

        call    AY_Frame

        pop     hl
        pop     bc
        pop     af
        ei
        reti
```

---

## Шаг 5: генерация тестовой мелодии

Для простоты создадим мелодию программно — синусоидальную волну через изменение тона:

```asm
; Заполнить player_data простой мелодией:
; Нота каждые 10 кадров, меняющийся тон

GenerateMelody:
        ld      hl, player_data
        ld      b, 0                ; тональная "нота"
        ld      c, 100              ; 100 кадров мелодии

.frame:
        ; R0 = младший байт тона, R1 = старший
        ld      a, b
        ld      (hl), a : inc hl    ; R0
        ld      (hl), 0 : inc hl    ; R1 = 0

        ld      (hl), 0 : inc hl    ; R2
        ld      (hl), 0 : inc hl    ; R3
        ld      (hl), 0 : inc hl    ; R4
        ld      (hl), 0 : inc hl    ; R5
        ld      (hl), 0 : inc hl    ; R6

        ld      (hl), #FE : inc hl  ; R7 (mixer: только A)

        ld      (hl), 12 : inc hl   ; R8 (амплитуда A)
        ld      (hl), 0 : inc hl    ; R9
        ld      (hl), 0 : inc hl    ; R10
        ld      (hl), 0 : inc hl    ; R11
        ld      (hl), 0 : inc hl    ; R12
        ld      (hl), 0 : inc hl    ; R13

        ; Увеличить B на 10 (изменение тона)
        ld      a, b
        add     a, 10
        ld      b, a

        dec     c
        jr      nz, .frame

        ; Сохранить длину
        ld      hl, 100 * 14        ; байт
        ld      (player_len), hl
        ret

; Массив данных: 100 кадров × 14 байт
player_data:    ds 100 * 14
```

---

## Шаг 6: главная программа

```asm
        org     #8100 - 512
        db      "EXE", 0
        dw      #0200, #0200, 0, #8100, main, #BFFF
        ds      512 - 16

        org     #8100

main:
        ; Сгенерировать мелодию
        call    GenerateMelody

        ; Установить прерывания
        call    SetupIM2

        ; Запустить плеер
        call    PlayerStart

        ; Показать сообщение
        ld      hl, msg
        ld      c, #5C
        rst     #10

        ; Ждать любую клавишу
        ld      c, #30
        rst     #10

        ; Остановить плеер
        call    PlayerStop

        ; Вернуть IM1
        im      1
        ei

        ; Выход
        ld      c, #41
        rst     #10

msg:    db      "Playing AY music. Press any key to stop.", 13, 10, 0

; ... все функции из шагов 2-5 ...
```

---

## Использование PT3 плеера

Для воспроизведения реальных PT3 файлов:

1. Загрузите плеер из SDK или репозиториев:
   - https://github.com/ENgineE777/zx-sprinter-sdk — `sdk/src/wyzplayer/`
   - Открытые плееры: [Pro Tracker 3 Disassembly](https://zxart.ee/)

2. Интеграция в код:
   ```asm
   INCLUDE "pt3player.asm"

   ; Загрузить PT3 файл в память
   ; PT3Init: HL = адрес PT3 данных
   ; PT3Play: вызывать 50 раз в секунду
   ```

3. В обработчике прерывания:
   ```asm
   IntHandler:
           push    af : push bc : push de : push hl : push ix : push iy
           call    PT3Play
           pop     iy : pop ix : pop hl : pop de : pop bc : pop af
           ei : reti
   ```

---

## Возможные улучшения

1. **Настоящий PT3 плеер** — интегрировать открытый плеер.
2. **AYFX эффекты** — звуковые эффекты поверх музыки (готов в SDK).
3. **Управление громкостью** через R8-R10.
4. **Плавный fade-in/out** между треками.
5. **Плейлист** — загрузка нескольких файлов.
6. **Vusmeters** на экране — визуализация громкости каналов.

---

## Загрузка и проигрывание файла

```asm
; Упрощённая загрузка AY файла

LoadAndPlay:
        ld      hl, filename
        ld      a, 1
        ld      c, #11 : rst #10    ; Open
        ret     c
        ld      (handle), a

        ; Читать в буфер
        ld      a, (handle)
        ld      hl, music_buf
        ld      de, #4000           ; до 16 КБ
        ld      c, #13 : rst #10    ; Read

        ld      a, (handle)
        ld      c, #12 : rst #10    ; Close

        ; Инициализировать плеер
        ld      hl, music_buf
        call    PT3Init             ; из плеера

        ; Запустить
        call    PlayerStart
        ret

filename:   db "MUSIC.PT3", 0
handle:     db 0
music_buf:  ds #4000
```

---

## Ключевые моменты

> - Плеер = функция, вызываемая 50 раз в секунду (VSync).
> - За один вызов записываются 14 регистров AY (R0..R13).
> - IM2 с вектором `#FDFD` через таблицу `#FE00`.
> - Для реальных треков — готовые плееры (PT3, AYFX).
> - Всегда останавливайте звук перед выходом (выключить mixer + обнулить амплитуды).
> - Музыка продолжает играть даже при загрузке/рисовании — благодаря прерыванию.

---

## Ссылки и источники {#ссылки}

| Источник | Путь / URL |
|---------|-----------|
| SDK AYFX плеер | https://github.com/ENgineE777/zx-sprinter-sdk — `sdk/src/wyzplayer/ayfxplay.asm` |
| SDK sound | https://github.com/ENgineE777/zx-sprinter-sdk — `sdk/src/sprinter/lib_sound.asm` |
| PT3 плеер (открытый) | https://github.com/ (поиск "pt3 player z80") |
| Прерывания | раздел [9.1 Прерывания](../09_advanced/01_interrupts.md) |
