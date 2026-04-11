# 8.1 Клавиатура

> **Навигация:** [← 7.5 Примеры диска](../07_disk/05_examples.md) | [Оглавление](../README.md) | [8.2 Мышь →](02_mouse.md)

---

## Два способа ввода

Sprinter поддерживает **два параллельных источника ввода**:

1. **ZX матрица** — эмуляция ZX Spectrum 40-клавишной клавиатуры через порт `#FE`. Позволяет запускать старые ZX-программы без модификаций. Подробнее об обратной совместимости см. [9.4 ZX Spectrum-совместимость](../09_advanced/04_zx_compat.md).
2. **PS/2 клавиатура** — полная PC-клавиатура через Z84C15 SIO, порты `#18`/`#19`. Основной способ ввода для нативных программ Sprinter.

DSS (функции `WaitKey`, `ScanKey`) использует комбинацию обоих источников: нажатие любой клавиши на физической PS/2 транслируется и в ZX-матрицу (для совместимости), и в расширенные коды для нативных приложений.

---

## ZX матрица через порт #FE

Классический способ ZX Spectrum. При чтении порта `#FE` значение на адресных линиях `A8`–`A15` выбирает **строку матрицы**, а данные на `D0`–`D4` возвращают состояние 5 клавиш этой строки (0 = нажата).

### Матрица клавиш ZX (8 строк × 5 колонок)

```
Address (A15..A8)   Row
#FEFE              Caps Shift, Z, X, C, V
#FDFE              A, S, D, F, G
#FBFE              Q, W, E, R, T
#F7FE              1, 2, 3, 4, 5
#EFFE              0, 9, 8, 7, 6
#DFFE              P, O, I, U, Y
#BFFE              Enter, L, K, J, H
#7FFE              Space, Symbol Shift, M, N, B
```

### Чтение ZX строки

```asm
; Прочитать строку 1-5 (клавиши 1..5)
        ld      bc, #F7FE
        in      a, (c)
        and     #1F                     ; нижние 5 бит = состояние клавиш
        cpl                             ; инвертировать (1 = нажата)
        ; Теперь A[4..0] = биты нажатых клавиш 5, 4, 3, 2, 1
```

### Пример: проверка Space

```asm
CheckSpace:
        ld      a, 127                  ; #7F (строка Space)
        in      a, (#FE)
        and     1                       ; Space = бит 0
        ret                             ; Z=1 если нажат, Z=0 если нет
```

---

## PS/2 через SIO Z84C15

Z84C15 содержит встроенный SIO (Serial Input/Output), настроенный на приём PS/2 сигналов от PC-клавиатуры через MAX7000.

| Порт | Имя | Назначение |
|------|-----|-----------|
| `#18` | `SIO_DATA_REG_A` | Принятый байт |
| `#19` | `SIO_CONTROL_A` | Статус SIO |

**Бит 0 порта `#19`**: `1` = байт готов к чтению.

### Чтение PS/2 скан-кода

```asm
SIO_CONTROL_A   EQU #19
SIO_DATA_REG_A  EQU #18

; Неблокирующая проверка наличия скан-кода
ScanPS2:
        in      a, (SIO_CONTROL_A)
        bit     0, a
        ret     z                       ; Z=1 → нет нажатия

        in      a, (SIO_DATA_REG_A)    ; A = скан-код
        ret                             ; Z=0 → A содержит скан-код
```

### Обработчик из flappybird

```asm
; Обрабатывает события PS/2, фильтрует F0 (key up) и E0 (extended)
KeysHandler:
.loop:
        in      a, (SIO_CONTROL_A)
        bit     0, a
        ret     z

        in      a, (SIO_DATA_REG_A)
        cp      #F0                     ; префикс "отпущена"
        jr      nz, .key
        ld      a, 1
        ld      (.needskipkey), a
        jr      .loop

.key:
        cp      #E0                     ; расширенная клавиша
        jr      z, .skipkey
        ld      c, 0
.needskipkey:   equ $-1
        bit     0, c
        jr      nz, .skipkey
        ld      (KeyPressed), a
.skipkey:
        xor     a
        ld      (.needskipkey), a
        jr      .loop

KeyPressed:     db 0
```

*Источник: `flappybird/src/sys_utils.asm`*

---

## PS/2 скан-коды (Set 2)

Частые скан-коды:

| Клавиша | Код |
|---------|-----|
| A | #1C |
| B | #32 |
| C | #21 |
| D | #23 |
| E | #24 |
| Enter | #5A |
| Space | #29 |
| Escape | #76 |
| Tab | #0D |
| Backspace | #66 |
| Left Shift | #12 |
| Left Ctrl | #14 |
| F1 | #05 |
| F10 | #09 |
| F12 | #07 |
| Up | #E0 #75 |
| Down | #E0 #72 |
| Left | #E0 #6B |
| Right | #E0 #74 |

Префикс `#F0` означает "клавиша отпущена", `#E0` — "расширенная клавиша".

---

## Через DSS

Самый простой способ чтения клавиатуры — через DSS API:

### WaitKey (блокирующий)

```asm
        ld      c, #30                  ; Dss.WaitKey
        rst     #10
        ; A = код клавиши (ASCII или расширенный)
```

### ScanKey (неблокирующий)

```asm
        ld      c, #31                  ; Dss.ScanKey
        rst     #10
        jr      z, .no_key              ; Z=1 → нет клавиши
        ; A = код
.no_key:
```

### Проверка модификаторов (Ctrl, Shift, Alt)

```asm
        ld      c, #33                  ; Dss.CTRLKey
        rst     #10
        ; B = биты:
        ;   bit 0 = Left Shift
        ;   bit 1 = Right Shift
        ;   bit 2 = Left Ctrl
        ;   bit 3 = Left Alt
        ;   ...
```

---

## Коды клавиш DSS

DSS возвращает коды в регистре A:

| Код | Клавиша |
|-----|---------|
| 9 | Tab |
| 13 | Enter |
| 27 | ESC |
| 32 | Space |
| 'A'..'Z' | Буквы (capital) |
| 'a'..'z' | Буквы (lower) |
| '0'..'9' | Цифры |

Расширенные клавиши (стрелки, функциональные):

| Код | Клавиша |
|-----|---------|
| #80 | Up |
| #81 | Down |
| #82 | Left |
| #83 | Right |
| #84 | Home |
| #85 | End |
| #86 | PgUp |
| #87 | PgDn |
| #88 | Insert |
| #89 | Delete |
| #B1..#BC | F1..F12 |

(Точные значения зависят от реализации DSS.)

---

## Настройка клавиатуры

Параметры клавиатуры (задержка автоповтора, скорость) задаются в CMOS регистре `#0F`:
- Биты [6:5] — задержка (250/500/750/1000 мс)
- Биты [4:0] — скорость (6–30 cps)

Установка из программы через CMOS API или `Dss.K_SETUP #36`.

---

## Очистка буфера клавиатуры

```asm
        ld      c, #35                  ; Dss.K_CLEAR
        rst     #10
```

Очищает внутренний буфер DSS — полезно перед ожиданием нового ввода.

---

## Пример: чтение строки

```asm
; Читать строку до Enter или ESC
; Вход: HL = буфер (минимум 80 байт)
; Выход: CF=0 успех, CF=1 ESC

ReadLine:
        ld      b, 0                    ; длина
.loop:
        ld      c, #30                  ; WaitKey
        rst     #10

        cp      27                      ; ESC
        jr      z, .esc
        cp      13                      ; Enter
        jr      z, .done
        cp      8                       ; Backspace
        jr      z, .bs

        ; Обычный символ
        cp      32
        jr      c, .loop                ; игнорировать управляющие
        ld      (hl), a
        inc     hl
        inc     b

        ; Echo
        push    bc
        push    hl
        ld      c, #5B
        rst     #10
        pop     hl
        pop     bc

        ld      a, b
        cp      79                      ; максимум
        jr      c, .loop
        jr      .done

.bs:
        ld      a, b
        or      a
        jr      z, .loop                ; уже пусто
        dec     hl
        dec     b

        ; Стереть на экране (BS + Space + BS)
        ld      a, 8 : push bc : ld c, #5B : rst #10 : pop bc
        ld      a, ' ' : push bc : ld c, #5B : rst #10 : pop bc
        ld      a, 8 : push bc : ld c, #5B : rst #10 : pop bc
        jr      .loop

.done:
        ld      (hl), 0                 ; терминатор
        or      a
        ret

.esc:
        scf
        ret
```

---

## Ключевые моменты

> - Два источника: ZX порт `#FE` (40 клавиш) и PS/2 через SIO `#18`/`#19` (полная клавиатура).
> - PS/2 скан-коды: `#F0` = отпущена, `#E0` = расширенная.
> - Для большинства задач используйте DSS `WaitKey #30` и `ScanKey #31`.
> - Модификаторы через `CTRLKey #33`.
> - Скорость автоповтора через CMOS регистр `#0F`.

---

## Ссылки и источники {#ссылки}

| Источник | Путь / URL |
|---------|-----------|
| PS/2 SIO handler | https://github.com/witchcraft2001/flappybird — `src/sys_utils.asm` |
| SDK клавиатура | https://github.com/ENgineE777/zx-sprinter-sdk — `sdk/src/sprinter/lib_input.asm` |
| PS/2 спецификация | общедоступная |
