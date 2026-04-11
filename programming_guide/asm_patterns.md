# Паттерны разработки на Z80 ASM

## Организация проекта

- **sjasmplus** — большинство современных проектов (`flappybird`, `espprobe`) используют `DEVICE ZXSPECTRUM128`, `INCLUDE` для таблиц портов и связывают ресурсы через `.PHASE`/`ORG` [flappybird/src/fbird.asm:1-30](../flappybird/src/fbird.asm#L1). Макросы `include\head.asm`, `include\bios_equ.asm`, `include\dss_equ.asm` позволяют переиспользовать Equates между проектами.
- **OrgAsm/CP/M стиль** — старые программы (текстовый редактор) добавляют EXE‑заголовок (`DB 'EXE' ...`, параметры загрузки/стека) и инклюдят код по адресам `#4180-#80`. Пример: [texteditor/textedit.asm:1-40](../texteditor/textedit.asm#L1).

## Точка входа и стек

- Настройка портов `RGADR`, получение памяти и сохранение экрана — базовый шаблон. Редактор начинает с `LD (SaveSP),SP`, выставляет `RGADR=#C0`, выделяет три страницы через `Dss.GetMem`, мапит их `Bios.Emm_Fn5`, сохраняет окно `WinSave` и переходит к парсингу командной строки [texteditor/routines.asm:1-120](../texteditor/routines.asm#L1).
- Удобно держать отдельную «безопасную» область под стек (`SafeStack`), на которую переключается обёртка `CallDss`. Это предотвращает перезапись пользовательского стека при вызове DSS.

## Обёртки для системных вызовов

```asm
CallDss:
        ld (.aValue),a
        in a,(EmmWin.P2)
        ld (.page),a
        in a,(EmmWin.P1)
        out (EmmWin.P2),a
        ld (.spSave),sp
        ld sp,SafeStack
        rst 10h
        ld sp,0
.spSave equ $-2
        push af
        ld a,0
.page   equ $-1
        out (EmmWin.P2),a
        pop af
        ret
```

(упрощённая версия из [texteditor/routines.asm:400-430](../texteditor/routines.asm#L400)). Аналогично можно сделать `CallBios`, если нужно временно поменять окна перед `RST #08`.

## Клавиатура и мышь

- Для AT‑клавиатуры используйте последовательный порт `COM_A/DAT_A`. Документ `z84c15/at_keyboard.md` показывает процедуру инициализации (`OUT (COM_A),...`) и чтения байта (проверка битов, чтение `DAT_A`). Не забывайте, что FIFO на 3 байта — нужно вычитывать всё, прежде чем выходить из ISR [sprinter_docs/z84c15/at_keyboard.md:1-35](../sprinter_docs/z84c15/at_keyboard.md#L1).
- Мышь проще обрабатывать через `RST #30`. `INTMOUSE.ASM` уже реализует команды `03h` (считать состояние), `04h` (переместить). В прикладном коде достаточно выставить `C` и вызвать `RST #30`, как делает `gfxview` [gfxview/gfx_view.asm:1-120](../gfxview/gfx_view.asm#L1).

## Двойная буферизация и IM2

Игровые проекты используют IM2/двойные страницы:

- Настройте IM2 обработчик (таблица, вектор), заполните «теневую» страницу, переключите `RGMOD` (бит 0) и рисуйте в неактивном экране.
- У Flappy Bird обработчик IM2 проверяет флаг `needChangePage` и, если установлен, переключает страницу и копирует изменений. Основной цикл ставит флаг после завершения кадра [flappybird/src/fbird.asm:90-150](../flappybird/src/fbird.asm#L90).
- Палитры обновляются двойным вызовом `SetPalette` (для первой и второй страниц), чтобы избежать вспышек при переключении [flappybird/src/fbird.asm:70-110](../flappybird/src/fbird.asm#L70).

## ISA и внешнее железо

`espprobe` содержит готовые процедуры `open_isa_ports`, `reset_isa`, `close_isa_ports`:

1. Сохранить `PAGE3` и текущую карту (`save_mmu3`).
2. Записать `0x11` в `Port.System` (`1FFDh`).
3. Выделить окно и управлять `Port.ISA`/`Port.System` (сигналы RESET/AEN/A14..A19) [espprobe/isa.asm:1-70](../espprobe/isa.asm#L1).

После выхода обязательно вернуть карту (`out (PAGE3),save_mmu3`).

## Турбо/порты конфигурации

Timer демонстрирует, как безопасно включать/выключать Turbo и перенастраивать ROM:

- Сохраняет текущий `PAGE3`, настраивает ISA, копирует код в `0xC800` если RAM доступна.
- Использует системный порт `#3C/#7C` (`SYS_PORT_OFF/ON`) для записи управляющих байтов (`D_TBON/D_TBOFF`, `D_ROM16ON/D_ROM16OFF`).
- После тестов восстанавливает страницу и статус CBL/ISA [sprinter_apps/Timer/TIMER.ASZ:50-150](../sprinter_apps/Timer/TIMER.ASZ#L50).

## Декодеры файлов и ресурсные форматы

`gfxview` содержит готовые парсеры BMP/ICO/PCX (`gfx_vbmp.asm`, `gfx_vico.asm`, `gfx_vpcx.asm`) и показывает, как структурировать большие ASM проекты: по одному модулю на формат, общий `gfx_view.asm` как фронтенд, `gfx_wind.asm` для оконного вывода. Аналогичным образом `flappybird/src` делит код на `grx_utils`, `sys_utils`, `pt3play` и т.п.

