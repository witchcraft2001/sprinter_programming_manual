# 6.4 Примеры звука

> **Навигация:** [← 6.3 COVOX DMA](03_covox_dma.md) | [Оглавление](../README.md) | [7.1 Обзор дисков →](../07_disk/01_disk_overview.md)

---

## Пример 1: простой "бип" через AY

```asm
        org     #8100-512
        db      "EXE", 0
        dw      #0200, #0200, 0, #8100, main, #BFFF
        ds      512 - 16

        org     #8100
main:
        ; R0 = #F9 (440 Гц)
        ld      a, 0 : out (#8D), a
        ld      a, #F9 : out (#8E), a
        ld      a, 1 : out (#8D), a
        xor     a : out (#8E), a

        ; R7 mixer: только тон A
        ld      a, 7 : out (#8D), a
        ld      a, #FE : out (#8E), a

        ; R8 амплитуда A = 15
        ld      a, 8 : out (#8D), a
        ld      a, 15 : out (#8E), a

        ; Пауза ~1 секунда
        ld      bc, 30000
.pause: dec     bc
        ld      a, b : or c
        jr      nz, .pause

        ; Остановить
        ld      a, 7 : out (#8D), a
        ld      a, #FF : out (#8E), a
        ld      a, 8 : out (#8D), a
        xor     a : out (#8E), a

        ld      c, #41
        rst     #10
```

---

## Пример 2: аккорд на 3 каналах AY

```asm
        ; Канал A: до (C5) — период #110 (приблизительно)
        ld      a, 0 : out (#8D), a
        ld      a, #10 : out (#8E), a
        ld      a, 1 : out (#8D), a
        ld      a, #01 : out (#8E), a

        ; Канал B: ми (E5) — период #D8
        ld      a, 2 : out (#8D), a
        ld      a, #D8 : out (#8E), a
        ld      a, 3 : out (#8D), a
        xor     a : out (#8E), a

        ; Канал C: соль (G5) — период #B6
        ld      a, 4 : out (#8D), a
        ld      a, #B6 : out (#8E), a
        ld      a, 5 : out (#8D), a
        xor     a : out (#8E), a

        ; Микшер: все 3 тона включены
        ld      a, 7 : out (#8D), a
        ld      a, #F8 : out (#8E), a

        ; Амплитуды
        ld      a, 8 : out (#8D), a
        ld      a, 12 : out (#8E), a
        ld      a, 9 : out (#8D), a
        ld      a, 12 : out (#8E), a
        ld      a, 10 : out (#8D), a
        ld      a, 12 : out (#8E), a
```

---

## Пример 3: AY с огибающей (pluck effect)

```asm
; Звук, затухающий через аппаратную огибающую
        ; Период тона (канал A)
        ld      a, 0 : out (#8D), a
        ld      a, #F9 : out (#8E), a
        ld      a, 1 : out (#8D), a
        xor     a : out (#8E), a

        ; Период огибающей (длительность затухания)
        ld      a, 11 : out (#8D), a
        ld      a, #FF : out (#8E), a
        ld      a, 12 : out (#8D), a
        ld      a, #08 : out (#8E), a

        ; Форма: \___ (одно падение)
        ld      a, 13 : out (#8D), a
        ld      a, 0 : out (#8E), a

        ; Амплитуда в режиме envelope
        ld      a, 8 : out (#8D), a
        ld      a, #10 : out (#8E), a

        ; Mixer: только тон A
        ld      a, 7 : out (#8D), a
        ld      a, #FE : out (#8E), a
```

---

## Пример 4: шумовой эффект (взрыв)

```asm
; Белый шум, затухающий
        ; Период шума
        ld      a, 6 : out (#8D), a
        ld      a, 5 : out (#8E), a         ; достаточно низкочастотный

        ; Микшер: только шум на канале A
        ld      a, 7 : out (#8D), a
        ld      a, #F7 : out (#8E), a       ; 11110111 = шум A on, тон A off

        ; Огибающая для затухания
        ld      a, 11 : out (#8D), a
        ld      a, #FF : out (#8E), a
        ld      a, 12 : out (#8D), a
        ld      a, #20 : out (#8E), a
        ld      a, 13 : out (#8D), a
        ld      a, 0 : out (#8E), a

        ; Амплитуда envelope
        ld      a, 8 : out (#8D), a
        ld      a, #10 : out (#8E), a
```

---

## Пример 5: секвенсер (проигрывание последовательности нот)

```asm
        org     #8100-512
        db      "EXE", 0
        dw      #0200, #0200, 0, #8100, main, #BFFF
        ds      512 - 16

        org     #8100

main:
        ; Инициализация микшера и амплитуды
        ld      a, 7 : out (#8D), a
        ld      a, #FE : out (#8E), a       ; только тон A
        ld      a, 8 : out (#8D), a
        ld      a, 15 : out (#8E), a

        ; Проиграть мелодию
        ld      hl, melody
.play_note:
        ld      a, (hl)
        inc     hl
        or      a
        jr      z, .done            ; 0 = конец

        ; Установить период
        out     (#8E), a            ; ВАЖНО: первый OUT в R0
        ld      b, a
        ld      a, 0
        out     (#8D), a            ; сначала выбрали R0
        ld      a, b
        out     (#8E), a

        ld      a, (hl)             ; старший байт периода
        inc     hl
        ld      b, a
        ld      a, 1
        out     (#8D), a
        ld      a, b
        out     (#8E), a

        ; Длительность
        ld      a, (hl)
        inc     hl
        call    Delay

        jr      .play_note

.done:
        ; Остановить
        ld      a, 7 : out (#8D), a
        ld      a, #FF : out (#8E), a

        ld      c, #41
        rst     #10

; Задержка A × 10 мс
Delay:
        ld      b, a
.outer: ld      de, 3000
.inner: dec     de
        ld      a, d : or e
        jr      nz, .inner
        djnz    .outer
        ret

; Мелодия: тон_lo, тон_hi, длительность
melody:
        db  #F9, #00, 30       ; 440 Гц, 300 мс
        db  #D8, #00, 30       ; 494 Гц
        db  #B6, #00, 60       ; 587 Гц
        db  0                  ; конец
```

---

## Пример 6: воспроизведение WAV через COVOX (упрощённо)

```asm
; Играть WAV файл через COVOX порт #88
; (Моно 8 бит, частота файла должна совпадать с установкой COVOX)

PlayWAV:
        ; Открыть файл
        ld      hl, wav_name
        ld      a, 1
        ld      c, #11 : rst #10
        jr      c, .err
        ld      (handle), a

        ; Пропустить 44 байта WAV-заголовка
        ld      a, (handle)
        ld      b, 0
        ld      hl, 0
        ld      ix, 44
        ld      c, #15 : rst #10

        ; Включить COVOX моно 22 кГц
        ld      a, #81
        out     (#89), a

.chunk:
        ld      a, (handle)
        ld      hl, audio_buf
        ld      de, 256
        ld      c, #13 : rst #10
        ld      a, d : or e
        jr      z, .done

        ld      hl, audio_buf
        ld      b, 0                ; 256 байт
.play:
        ld      a, (hl)
        out     (#88), a
        inc     hl

        ; Задержка для 22 кГц (~45 мкс)
        push    bc
        ld      b, 15
.w:     djnz    .w
        pop     bc

        djnz    .play
        jr      .chunk

.done:
        xor     a
        out     (#89), a

        ld      a, (handle)
        ld      c, #12 : rst #10

.err:
        ld      c, #41 : rst #10

wav_name:   db "SOUND.WAV", 0
handle:     db 0
audio_buf:  ds 256
```

---

## Пример 7: beeper-style (через AY амплитуду)

```asm
; Имитация динамика ZX Spectrum — быстрое переключение амплитуды
Beep1kHz:
        ld      a, 7 : out (#8D), a
        ld      a, #FE : out (#8E), a

        ld      b, 100              ; 100 циклов

.loop:
        ; Высокий
        ld      a, 8 : out (#8D), a
        ld      a, 15 : out (#8E), a

        ld      c, 100
.w1:    dec     c
        jr      nz, .w1

        ; Низкий
        ld      a, 8 : out (#8D), a
        xor     a : out (#8E), a

        ld      c, 100
.w2:    dec     c
        jr      nz, .w2

        djnz    .loop
        ret
```

---

## Пример 8: AY эффект sliding pitch (glissando)

```asm
; Плавное изменение тона — сирена
Siren:
        ; Включить тон A
        ld      a, 7 : out (#8D), a
        ld      a, #FE : out (#8E), a
        ld      a, 8 : out (#8D), a
        ld      a, 15 : out (#8E), a

        ; Цикл: от 100 до 1000 Гц и обратно
        ld      d, 5                ; 5 повторов
.outer:
        ld      hl, 1000            ; начальный период (низкий тон)
.up:
        ; Записать HL как период R0, R1
        ld      a, 0 : out (#8D), a
        ld      a, l : out (#8E), a
        ld      a, 1 : out (#8D), a
        ld      a, h : out (#8E), a

        ; Задержка
        push    hl
        ld      bc, 50
.p1:    dec     bc : ld a, b : or c : jr nz, .p1
        pop     hl

        dec     hl
        ld      a, h : or l
        jr      nz, .up

        dec     d
        jr      nz, .outer

        ; Остановить
        ld      a, 7 : out (#8D), a
        ld      a, #FF : out (#8E), a
        ret
```

---

## Пример 9: стерео тон через AY

AY в ZX стандарте моно, но можно разделить каналы: канал A → левый, B → правый. Это работает, если выход AY платы Sprinter разделён по стерео каналам (зависит от конфигурации).

```asm
; Левый канал = C4, правый канал = E4
        ld      a, 0 : out (#8D), a
        ld      a, #D3 : out (#8E), a       ; R0 — C4
        ld      a, 1 : out (#8D), a
        ld      a, 1 : out (#8E), a

        ld      a, 2 : out (#8D), a
        ld      a, #08 : out (#8E), a       ; R2 — E4
        ld      a, 3 : out (#8D), a
        ld      a, 1 : out (#8E), a

        ld      a, 7 : out (#8D), a
        ld      a, #FC : out (#8E), a       ; тон A и B

        ld      a, 8 : out (#8D), a
        ld      a, 15 : out (#8E), a
        ld      a, 9 : out (#8D), a
        ld      a, 15 : out (#8E), a
```

---

## Пример 10: детектор клавиши для бипа

```asm
; Нажатие клавиши — короткий звук

Beep:
        ; R0 = период, mixer, амплитуда
        ld      a, 0 : out (#8D), a
        ld      a, #80 : out (#8E), a
        ld      a, 7 : out (#8D), a
        ld      a, #FE : out (#8E), a
        ld      a, 8 : out (#8D), a
        ld      a, 12 : out (#8E), a

        ; Пауза 50 мс
        ld      bc, 1500
.p:     dec     bc : ld a, b : or c : jr nz, .p

        ; Выключить
        ld      a, 8 : out (#8D), a
        xor     a : out (#8E), a
        ld      a, 7 : out (#8D), a
        ld      a, #FF : out (#8E), a
        ret

; Основной цикл: beep при каждой клавише
MainLoop:
        ld      c, #30 : rst #10        ; WaitKey
        cp      27                      ; ESC = выход
        jr      z, .exit
        call    Beep
        jr      MainLoop

.exit:
        ld      c, #41 : rst #10
```

---

## Ключевые моменты

> - Простой тон AY: запись периода в R0/R1, включение в R7 (mixer), амплитуда в R8.
> - Mixer R7 — инвертированный (0 = вкл).
> - Огибающая: R11/R12 период, R13 форма, R8 (или R9/R10) = `#10` для активации envelope.
> - Шум: R6 период, R7 биты [5:3] для включения.
> - COVOX: `OUT (#89), mode_byte` для включения, `OUT (#88), sample` для сэмплов.
> - Для WAV: пропустить 44 байта заголовка, читать блоками, играть в порт `#88` с задержкой.
> - Секвенсер — таблица нот + цикл с задержками.

---

## Ссылки и источники {#ссылки}

| Источник | Путь / URL |
|---------|-----------|
| SDK sound | https://github.com/ENgineE777/zx-sprinter-sdk — `sdk/src/sprinter/lib_sound.asm` |
| SDK wyzplayer | https://github.com/ENgineE777/zx-sprinter-sdk — `sdk/src/wyzplayer/` |
| AY документация | Yamaha/GI datasheet |
