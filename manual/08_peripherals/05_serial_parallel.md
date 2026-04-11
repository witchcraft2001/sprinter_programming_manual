# 8.5 SIO и PIO (Z84C15)

> **Навигация:** [← 8.4 CMOS/RTC](04_cmos_rtc.md) | [Оглавление](../README.md) | [9.1 Прерывания →](../09_advanced/01_interrupts.md)

---

## Обзор встроенной периферии Z84C15

Процессор Z84C15 содержит встроенные блоки Z80 SIO, Z80 CTC и Z80 PIO. В Sprinter они используются следующим образом:

| Блок | Назначение | Порты |
|------|-----------|-------|
| **SIO Channel A** | PS/2 клавиатура (приём) | `#18`/`#19` |
| **SIO Channel B** | Дополнительный (резерв) | `#1A`/`#1B` |
| **CTC** (4 канала) | Системные таймеры, прерывания | `#10`–`#13` |
| **PIO** | Не используется активно | — |

---

## Z80 SIO (Serial Input/Output)

SIO — универсальный двухканальный последовательный контроллер. В Sprinter канал A настроен для **приёма PS/2 сканкодов** от клавиатуры (прошитая MAX7000 схема декодирует PS/2 в SIO-совместимый поток).

### Порты SIO

| Порт | Имя | Назначение |
|------|-----|-----------|
| `#18` | `DAT_A` / `SIO_DATA_REG_A` | Канал A — данные |
| `#19` | `COM_A` / `SIO_CONTROL_A` | Канал A — управление/статус |
| `#1A` | `DAT_B` / `KBD_DAT` | Канал B — данные |
| `#1B` | `COM_B` / `KBD_COM` | Канал B — управление |

### Проверка готовности байта

Бит 0 регистра статуса (`COM_A`) = 1 когда байт готов:

```asm
SIO_CONTROL_A   EQU #19
SIO_DATA_REG_A  EQU #18

WaitChar:
        in      a, (SIO_CONTROL_A)
        bit     0, a
        jr      z, WaitChar
        in      a, (SIO_DATA_REG_A)
        ret
```

### Инициализация SIO

Z80 SIO имеет 8 внутренних "Write Registers" (WR0–WR7). Инициализация выполняется записью последовательности байт в control port:

```asm
; Сброс канала A
        ld      a, #18              ; WR0: Channel Reset
        out     (SIO_CONTROL_A), a

        ; WR4: x16 clock, 8-bit, 1 stop bit
        ld      a, #04              ; select WR4
        out     (SIO_CONTROL_A), a
        ld      a, #44              ; x16, 8N1
        out     (SIO_CONTROL_A), a

        ; WR3: Rx enabled, 8 bits/char
        ld      a, #03              ; select WR3
        out     (SIO_CONTROL_A), a
        ld      a, #C1              ; 8 bits, Rx enable
        out     (SIO_CONTROL_A), a

        ; WR5: Tx enabled, 8 bits/char, DTR/RTS
        ld      a, #05
        out     (SIO_CONTROL_A), a
        ld      a, #EA              ; Tx enable, 8 bits, RTS
        out     (SIO_CONTROL_A), a

        ; WR1: Rx interrupt on all chars
        ld      a, #01
        out     (SIO_CONTROL_A), a
        ld      a, #18              ; Rx int on all, status affects vector
        out     (SIO_CONTROL_A), a
```

> **Замечание:** в Sprinter SIO уже инициализирован BIOS для PS/2. Пользовательской программе обычно не нужно его переинициализировать.

---

## Z80 CTC (Counter/Timer)

CTC имеет 4 независимых канала. В Sprinter используются:
- **Канал 2** — системный таймер 50 Гц (через делитель 256)
- **Канал 3** — дополнительный таймер (частота выше)

### Порты CTC

```asm
CTC_CH0         EQU #10
CTC_CH1         EQU #11
CTC_CH2         EQU #12
CTC_CH3         EQU #13
```

### Режим работы

CTC может работать в двух режимах:

**Timer mode** — делит входной клок на предделитель (16 или 256) и затем на время-константу:
```
f_out = f_in / (prescaler × time_constant)
```

**Counter mode** — считает внешние события.

---

### Инициализация канала (из SDK lib_startup.asm)

```asm
; Настройка CTC для прерываний 50 Гц при 7 МГц CPU
; Формула: 7000000 / (256 × 112) ≈ 244 Гц (частота канала 2)
; Или более точно для 50 Гц: 7000000/(256×547)≈50

        ld      a, #57              ; Timer, prescaler=256, INT enable
        out     (CTC_CH2), a
        ld      a, 112              ; time constant
        out     (CTC_CH2), a

        ld      a, #D7              ; Timer, prescaler=256, INT enable (альт.)
        out     (CTC_CH3), a
        ld      a, 160
        out     (CTC_CH3), a
```

Байт control:

| Бит | Значение |
|-----|---------|
| 7 | Interrupt enable (1) / disable (0) |
| 6 | Counter (1) / Timer (0) |
| 5 | Prescaler: 256 (1) / 16 (0) |
| 4 | Rising edge trigger |
| 3 | Wait for trigger |
| 2 | Time constant follows (1) |
| 1 | Software reset |
| 0 | Control word (1) / vector (0) |

### Установка вектора прерывания

CTC в IM2 сам формирует вектор. Первая запись в любой канал (с бит 0 = 0) — это базовый вектор:

```asm
        ld      a, #80              ; вектор (старшие 5 бит) + базовый
        out     (CTC_CH0), a
```

После этого 4 канала будут прерывать с векторами: base, base+2, base+4, base+6.

---

## Пример: таймер для замера времени

```asm
; Использовать CTC канал 0 как счётчик миллисекунд (приблизительно)
; При 7 МГц, prescaler 256: 7e6 / 256 = ~27343 Гц
; Для 1 мс → time constant ≈ 27

InitMsTimer:
        ld      a, #47              ; Timer, prescaler 256, без INT
        out     (CTC_CH0), a
        ld      a, 27
        out     (CTC_CH0), a
        ret

; Прочитать текущее значение счётчика
ReadCounter:
        in      a, (CTC_CH0)
        ret
```

---

## Z80 PIO (Parallel I/O)

PIO в Z84C15 не задействован Sprinter'ом активно. Теоретически можно использовать как 2×8-бит GPIO:

```asm
; PIO порты (если доступны)
PIO_DAT_A       EQU #1C     ; (в Sprinter перекрыто LPT1)
PIO_CTR_A       EQU #1D
PIO_DAT_B       EQU #1E
PIO_CTR_B       EQU #1F
```

> В Sprinter эти адреса используются под **LPT1/LPT2** через FPGA. Прямое обращение к PIO Z84C15 может не работать.

---

## Пример: Watchdog через CTC

```asm
; Настроить CTC канал 0 как watchdog (2 секунды)
; При истечении — прерывание, в котором программа может сбросить систему

InitWatchdog:
        ld      a, #A7              ; Timer, INT enable, prescaler=256, TC follows
        out     (CTC_CH0), a
        ld      a, 200              ; ~2 секунды
        out     (CTC_CH0), a
        ret

; Сбрасывать watchdog периодически (каждые 1.5 секунды)
KickWatchdog:
        ld      a, #A7
        out     (CTC_CH0), a
        ld      a, 200
        out     (CTC_CH0), a
        ret

; Обработчик истечения watchdog (вектор IM2)
WatchdogHandler:
        ; Перезагрузка / лог / восстановление
        jp      #0000               ; warm reboot
```

---

## Связь SIO и IM2

SIO генерирует прерывания при получении данных. В Sprinter этот механизм используется в обработчике PS/2 клавиатуры:

```asm
; Обработчик SIO Rx (через IM2)
SIORxHandler:
        push    af
        ; Чтение из SIO автоматически сбрасывает флаг
        in      a, (SIO_DATA_REG_A)
        ; ... обработать скан-код ...
        ld      (PS2_Buffer), a
        pop     af
        ei
        reti
```

---

## Ключевые моменты

> - SIO канал A в Sprinter — для приёма PS/2 клавиатуры (порты `#18`/`#19`).
> - SIO канал B обычно не используется (порты `#1A`/`#1B` могут быть LPT).
> - CTC 4 канала (`#10`–`#13`) — системные таймеры, источник прерываний.
> - CTC Timer mode: `f_out = f_in / (prescaler × TC)`.
> - В IM2 каналы CTC формируют вектор автоматически (первая запись = базовый вектор).
> - PIO Z84C15 в Sprinter не активен (адреса заняты LPT).

---

## Ссылки и источники {#ссылки}

| Источник | Путь / URL |
|---------|-----------|
| SDK CTC init | https://github.com/ENgineE777/zx-sprinter-sdk — `sdk/src/sprinter/lib_startup.asm` |
| Flappybird SIO PS/2 | https://github.com/witchcraft2001/flappybird — `src/sys_utils.asm` |
| Zilog Z80-CTC manual | `docs/howto_program_the_Z80-CTC.pdf` |
| Zilog Z80-SIO manual | `docs/Zilog_Z80-SIO_Technical_Manual.pdf` |
