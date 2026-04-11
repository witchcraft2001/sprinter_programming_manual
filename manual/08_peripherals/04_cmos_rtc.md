# 8.4 CMOS и часы реального времени (RTC)

> **Навигация:** [← 8.3 ISA шина](03_isa_bus.md) | [Оглавление](../README.md) | [8.5 SIO/PIO →](05_serial_parallel.md)

---

## Обзор

CMOS RAM — небольшая (~128 байт) энергонезависимая память с батарейкой. В Sprinter она содержит:
- **Часы реального времени** (секунды, минуты, часы, дата)
- **Настройки BIOS** (тип компьютера, turbo, диски, видеорежим)
- **Контрольную сумму** для проверки целостности

---

## Доступ к CMOS

### Через ISA-порты

```asm
CMOS_DRD        equ #FFBDh      ; чтение данных
CMOS_DWR        equ #BFBDh      ; запись данных
CMOS_AWR        equ #DFBDh      ; запись адреса регистра
```

### Через MAX7000 DCP порты

```asm
; Альтернатива — порты #1C, #1D, #1E
; #1C : CMOS_DAT_RD
; #1D : CMOS_ADR_WR
; #1E : CMOS_DAT_WR
```

---

## Чтение регистра CMOS

```asm
; Прочитать регистр ADR из CMOS
; Вход: A = номер регистра
; Выход: A = значение регистра
ReadCMOS:
        ld      bc, CMOS_AWR
        out     (c), a              ; выбрать регистр
        ld      bc, CMOS_DRD
        in      a, (c)              ; прочитать
        ret
```

## Запись регистра CMOS

```asm
; Записать в регистр
; Вход: H = номер регистра, A = значение
WriteCMOS:
        push    af
        ld      bc, CMOS_AWR
        ld      a, h
        out     (c), a
        pop     af
        ld      bc, CMOS_DWR
        out     (c), a
        ret
```

---

## Часы реального времени (RTC)

Структура регистров стандартного PC RTC:

| Регистр | Назначение | Диапазон |
|---------|-----------|---------|
| `#00` | Секунды | 0–59 |
| `#01` | Секунды тревоги | — |
| `#02` | Минуты | 0–59 |
| `#03` | Минуты тревоги | — |
| `#04` | Часы | 0–23 |
| `#05` | Часы тревоги | — |
| `#06` | День недели | 1–7 (1=Вс или Пн) |
| `#07` | День месяца | 1–31 |
| `#08` | Месяц | 1–12 |
| `#09` | Год | 0–99 (младшие 2 цифры) |
| `#0A` | Status A | — |
| `#0B` | Status B | — |
| `#0C` | Status C | — |
| `#0D` | Status D | — |
| `#32` | Век | обычно 19 или 20 |

---

## Чтение времени

```asm
ReadTime:
        ; Чтение секунд
        ld      a, #00
        call    ReadCMOS
        ld      (sec), a

        ; Минуты
        ld      a, #02
        call    ReadCMOS
        ld      (min), a

        ; Часы
        ld      a, #04
        call    ReadCMOS
        ld      (hr), a

        ; День
        ld      a, #07
        call    ReadCMOS
        ld      (day), a

        ; Месяц
        ld      a, #08
        call    ReadCMOS
        ld      (mon), a

        ; Год
        ld      a, #09
        call    ReadCMOS
        ld      (yr), a
        ret

sec:    db 0
min:    db 0
hr:     db 0
day:    db 0
mon:    db 0
yr:     db 0
```

---

## Формат BCD

Во многих реализациях RTC значения хранятся в **BCD** (Binary Coded Decimal): каждая цифра в отдельном nibble.

```
Число 25 (decimal) → 0x25 (BCD) = 0010_0101
```

### Преобразование BCD → binary

```asm
BCDToBin:
        push    bc
        ld      b, a
        and     #F0
        srl     a : srl a : srl a : srl a   ; верхний nibble
        ld      c, a
        add     a, a                ; *2
        add     a, c                ; *3
        add     a, c                ; *4
        add     a, c                ; *5
        add     a, c                ; *6
        add     a, c                ; *7
        add     a, c                ; *8
        add     a, c                ; *9
        add     a, c                ; *10 (упрощённо)
        ld      c, a
        ld      a, b
        and     #0F
        add     a, c
        pop     bc
        ret
```

Проще: `(A >> 4) * 10 + (A & 0x0F)`.

### Преобразование binary → BCD

```asm
BinToBCD:
        ld      b, 0
.loop:  cp      10
        jr      c, .done
        sub     10
        inc     b
        jr      .loop
.done:
        push    af
        ld      a, b
        rlca : rlca : rlca : rlca
        pop     bc                  ; B = младшая цифра (был A)
        or      c
        ret
```

---

## Запись времени

```asm
SetTime:
        ; Установить часы в 12:30:00
        ld      h, #04              ; часы
        ld      a, #12              ; 12 (BCD)
        call    WriteCMOS

        ld      h, #02              ; минуты
        ld      a, #30
        call    WriteCMOS

        ld      h, #00              ; секунды
        ld      a, #00
        call    WriteCMOS
        ret
```

---

## Через DSS API (проще)

DSS предоставляет высокоуровневые функции:

```asm
; Получить текущее время
        ld      c, #21              ; Dss.SysTime
        rst     #10
        ; D = день, E = месяц
        ; IX = год (полный, например 2026)
        ; H = часы, L = минуты, B = секунды
        ; C = день недели (1-7)

; Установить время
        ld      c, #22              ; Dss.SetTime
        ; D, E, IX, H, L, B — параметры
        rst     #10
```

DSS автоматически конвертирует BCD и делает нужную корректировку.

---

## Регистры конфигурации BIOS в CMOS

Начиная с регистра `#0E` и до `#3F` — настройки BIOS (см. подробнее в [3.4 Конфигурация](../03_bios/04_bios_config.md)):

| Регистр | Назначение |
|---------|-----------|
| `#0E` | Флаги старта (тест памяти, язык) |
| `#0F` | Клавиатура (автоповтор) |
| `#10` | Загрузочный диск |
| `#11` | Типы FDD/IDE |
| `#1A` | Цветовая схема |
| `#1B` | Turbo, тип ПК |
| `#1C` | Режим сброса |
| `#1E` | TR-DOS маршрутизация |
| `#32` | Век |
| `#35`–`#3E` | Дополнительные настройки |
| `#3F` | Контрольная сумма |

---

## Контрольная сумма

При каждой записи в CMOS BIOS проверяет `#3F` — сумму всех настроечных регистров. Если неверна → загружаются defaults.

```asm
; Пересчитать и записать контрольную сумму
UpdateCMOSChecksum:
        ld      b, 0                ; сумма
        ld      a, #0E              ; первый настроечный регистр
.loop:
        push    bc
        push    af
        call    ReadCMOS
        pop     bc                  ; B = текущий регистр (был A)
        push    bc

        ; Добавить к сумме
        ld      c, a
        ld      a, b                ; сумма
        add     a, c
        ld      (sum), a

        pop     af
        inc     a                   ; следующий регистр
        pop     bc
        ld      b, (sum)
        cp      #1E                 ; до #1D включительно
        jr      nz, .loop

        ; Записать сумму в #3F
        ld      h, #3F
        ld      a, b
        call    WriteCMOS
        ret

sum:    db 0
```

---

## Ключевые моменты

> - CMOS — энергонезависимая RAM с RTC (128 байт).
> - Доступ: `CMOS_AWR #DFBD` (адрес), `CMOS_DWR #BFBD` (запись), `CMOS_DRD #FFBD` (чтение).
> - Время: регистры `#00`–`#09` в BCD формате.
> - Настройки BIOS: `#0E`–`#3E`, контрольная сумма `#3F`.
> - Для прикладных задач используйте `Dss.SysTime #21` и `Dss.SetTime #22`.
> - После записи настроек — обновляйте контрольную сумму.

---

## Ссылки и источники {#ссылки}

| Источник | Путь / URL |
|---------|-----------|
| CMOS константы | `zx-sprinter-sdk/sdk/src/sprinter/sprint00.asm` |
| CMOS регистры BIOS | `sprinter_bios/SETUP/DSETUP.ASM` (комментарии 30–100) |
| DSS time API | `espprobe/dss_equ.asm` — `Dss.SysTime`, `Dss.SetTime` |
