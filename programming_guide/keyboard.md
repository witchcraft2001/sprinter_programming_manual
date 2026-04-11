# Клавиатура и Z84C15

Sprinter поддерживает два режима ввода: классическую ZX-матрицу (`IN A,(#FE)`) и AT-клавиатуру, подключенную к последовательному порту Z84C15. Ниже приведены практические рекомендации, основанные на [generated_doc/peripherals.md](../generated_doc/peripherals.md) и заметках из `sprinter_docs/z84c15`.

## ZX-матрица (порт #FE)

- Чтение `IN A,(#FE)` возвращает пятивбитный столбец выбранной строки (0 = клавиша нажата). Адресные линии A8..A15 играют роль маски строки. Например, для чтения ряда `Caps Shift/Z/X/C/V` нужно выставить `A8=0` (значение `0xFE`) перед инструкцией.
- Порядок строк:

| Маска (A15..A8) | Клавиши |
|-----------------|---------|
| `0FEh` | Shift, Z, X, C, V |
| `0FDh` | A, S, D, F, G |
| `0FBh` | Q, W, E, R, T |
| `0F7h` | 1, 2, 3, 4, 5 |
| `0EFh` | 0, 9, 8, 7, 6 |
| `0DFh` | P, O, I, U, Y |
| `0BFh` | Enter, L, K, J, H |
| `07Fh` | Space, Sym Shift, M, N, B |

Для удобства создан include-файл `programming_guide/include/kbd_matrix.inc` с константами `KBD_ROW_*` и `KBD_*_BIT`. Использование:

```asm
        INCLUDE "programming_guide/include/kbd_matrix.inc"

        LD   A,KBD_ROW_QWERT
        IN   A,(#FE)
        BIT  KBD_W_BIT,A      ; 0 = клавиша W нажата
        JR   Z,.pressed
```

Порты `#FE` также управляют бордером (биты 0-2) и бипером (бит 4) — см. [generated_doc/peripherals.md](../generated_doc/peripherals.md).

## AT-клавиатура через Z84C15

AT-клавиатура подключена к последовательному каналу Z84C15. Управляющий порт (`COM_A`) = `#19`, данные (`DAT_A`) = `#18`. Пример инициализации и чтения приведён в [sprinter_docs/z84c15/at_keyboard.md](../sprinter_docs/z84c15/at_keyboard.md):

```asm
COM_A   EQU 19h
DAT_A   EQU 18h

KbdInit:
        LD   A,0
        OUT  (COM_A),A
        LD   A,1
        OUT  (COM_A),A
        LD   A,0
        OUT  (COM_A),A
        LD   A,3
        OUT  (COM_A),A
        LD   A,0C1h
        OUT  (COM_A),A
        LD   A,4
        OUT  (COM_A),A
        LD   A,5h
        OUT  (COM_A),A
        LD   A,5
        OUT  (COM_A),A
        LD   A,062h
        OUT  (COM_A),A
        RET

ReadKey:
        IN   A,(COM_A)
        BIT  0,A          ; есть байт?
        RET  Z
        IN   A,(DAT_A)    ; scancode
        RET
```

Замечания:

- FIFO последовательного порта длиной 3 байта, поэтому обработчик должен вычитывать все накопившиеся коды до выхода из ISR.
- TR-DOS переназначает порт `#1F`, поэтому любые операции с внутренними портами Z84C15 выполняйте только через форму `LD BC,#001F` / `OUT (C),A`, иначе адрес может быть заменён (что важно для настроек PIO/ISA) [sprinter_docs/z84c15/main.md].
- Клавиатурные прерывания разрешаются битами `ALL_MODE0/ALL_MODE3` (см. `generated_doc/peripherals.md`). Если `ALL_MODE0=0`, IRQ от клавиатуры полностью отключены.

## Сканкоды AT-клавиатуры

Sprinter использует стандартный набор Set 2. В каталоге `programming_guide/include` лежит файл `at_scancodes.inc` с определениями `SC_*` для всех клавиш (ESC, функциональные, стрелки, цифровая панель). Важно помнить:

- Любой код `>= 0x80` не встречается — отпускание обозначается отдельным префиксом `0xF0` (`SC_PREFIX_BREAK`).
- Расширенные клавиши (стрелки, клавиши навигации, правый Ctrl/Alt, клавиатура NumPad при выключенном NumLock) присылают префикс `0xE0` (`SC_PREFIX_EXT`).
- Некоторые клавиши (Print Screen, Pause) отправляют многобайтовые последовательности (`E0 12 E0 7C`, `E1 14 77  E1 F0 14 F0 77`).

Таблица реальных кодов приведена в `at_scancodes.inc` и соответствует источникам [docs/Sprinter_Programming.pdf](../docs/Sprinter_Programming.pdf#page=2) и стандартам IBM. Например:

| Клавиша | Код (Set 2) |
|---------|-------------|
| ESC | `76h` |
| F1 | `05h` |
| Enter | `5Ah` |
| Space | `29h` |
| Левый Shift | `12h` |
| Правый Shift | `59h` |
| Левый Ctrl | `14h` |
| Левый Alt | `11h` |
| Num Lock | `77h` |
| Стрелка вверх | `E0 75h` |
| Клавиша «/» на NumPad | `E0 4Ah` |

Если нужно переназначить раскладки или обновить таблицы, используйте утилиту `KEYBOARD.EXE` из `sprinter_dss/utils/KEYBOARD`, которая работает через DSS и порт `COM_A/DAT_A`.

### Пример: отслеживание модификаторов и курсора

```asm
        INCLUDE "programming_guide/include/at_scancodes.inc"

MOD_SHIFT EQU 0
MOD_CTRL  EQU 1
MOD_ALT   EQU 2
MOD_CAPS  EQU 3
mod_state DB 0
prefix    DB 0        ; 0=обычный, 1=E0 получен

PollAT:
        CALL ReadKey
        RET  Z
        CP   SC_PREFIX_EXT
        JR   Z,.got_ext
        CP   SC_PREFIX_BREAK
        JR   Z,.got_break
        LD   HL,mod_state
        BIT  0,prefix
        JR   NZ,.extended
        CP   SC_LSHIFT
        JR   Z,.press_shift
        CP   SC_RSHIFT
        JR   Z,.press_shift
        CP   SC_LCTRL
        JR   Z,.press_ctrl
        CP   SC_LALT
        JR   Z,.press_alt
        CP   SC_CAPSLOCK
        JR   Z,.toggle_caps
        CP   SC_ENTER
        JR   Z,.handle_enter
        CP   SC_ARROWUP
        JR   Z,.cursor_up
        ; обработка прочих символов...
        XOR  A
        LD   (prefix),A
        RET

.extended:
        CP   SC_RCTRL
        JR   Z,.press_ctrl
        CP   SC_RALT
        JR   Z,.press_alt
        CP   SC_ARROWLEFT
        JR   Z,.cursor_left
        XOR  A
        LD   (prefix),A
        RET

.press_shift:
        SET  MOD_SHIFT,(HL)
        XOR  A
        LD   (prefix),A
        RET

.press_ctrl:
        SET  MOD_CTRL,(HL)
        XOR  A
        LD   (prefix),A
        RET

.press_alt:
        SET  MOD_ALT,(HL)
        XOR  A
        LD   (prefix),A
        RET

.toggle_caps:
        XOR  A
        BIT  MOD_CAPS,(HL)
        JR   Z,.caps_on
        RES  MOD_CAPS,(HL)
        JR   .caps_update
.caps_on:
        SET  MOD_CAPS,(HL)
.caps_update:
        XOR  A
        LD   (prefix),A
        RET

.got_ext:
        LD   A,1
        LD   (prefix),A
        RET

.got_break:
        CALL ReadKey          ; получить код отпускания
        LD   HL,mod_state
        CP   SC_LSHIFT
        JR   Z,.rel_shift
        CP   SC_RSHIFT
        JR   Z,.rel_shift
        CP   SC_LCTRL
        JR   Z,.rel_ctrl
        CP   SC_RCTRL
        JR   Z,.rel_ctrl
        CP   SC_LALT
        JR   Z,.rel_alt
        CP   SC_RALT
        JR   Z,.rel_alt
        XOR  A
        LD   (prefix),A
        RET

.rel_shift: RES MOD_SHIFT,(HL)
.rel_ctrl:  RES MOD_CTRL,(HL)
.rel_alt:   RES MOD_ALT,(HL)
            XOR A
            LD  (prefix),A
            RET
```

В примере выше `mod_state` хранит флаги Shift/Ctrl/Alt/Caps. Для стрелок и цифровой клавиатуры достаточно проверить код и наличие `prefix` (если `prefix=1`, значит пришёл E0 — это стрелки и расширенные клавиши, а не цифры Numpad). Аналогично можно отличать `KP_ENTER` (`E0 5A`) от основного Enter.

## Дополнительные советы

## Дополнительные советы

- Перед переключением конфигураций (F12 + комбинации) обработчик должен очистить FIFO, иначе после возврата в DOS вы получите «залипшие» коды.
- Для ZX-режима удобно объединять матрицу и AT-коды: сперва опросите `#FE` для базовых клавиш, затем дополняйте AT-кодами (F11/F12, стрелки, NumPad).
- Состояние LED и альтернативных раскладок управляется функциями `Dss.CtrlKey/K_Setup` (см. `sprinter_dss/docs/r_dsslist.htm`) — BIOS интегрирует аппаратный драйвер и выставляет флаги при смене раскладки.
