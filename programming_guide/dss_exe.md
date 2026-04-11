# Формат EXE-файлов Estex DSS

Программы DSS используют простой 512-байтовый заголовок «EXE», после которого сразу лежит основной код. Заголовок заполняется напрямую в исходниках (см. `texteditor/textedit.asm`, `gfxview/loader.asm`, `sprintem/disk/HELLO.ASM`) и определяет, куда система загрузит и откуда запустит программу [texteditor/textedit.asm:1-40](../texteditor/textedit.asm#L1) [gfxview/loader.asm:1-30](../gfxview/loader.asm#L1) [sprintem/disk/HELLO.ASM:1-30](../sprintem/disk/HELLO.ASM#L1).

## Структура заголовка

| Смещение | Размер | Поле | Описание |
|----------|--------|------|----------|
| 0x00 | 3 байта | Signature | ASCII `"EXE"`. Загружчик использует это, чтобы отличить двоичные файлы DSS от сырых `.BIN`. |
| 0x03 | 1 байт | Version | Ревизия формата. В известных примерах равна `0`, поэтому принято оставлять ноль [gfxview/loader.asm:1-10](../gfxview/loader.asm#L1). |
| 0x04 | 2 байта | HeaderLenLo | Размер заголовка. Почти все программы ставят `0x0200`, чтобы указать стандартные 512 байт. |
| 0x06 | 2 байта | HeaderLenHi | Старшие биты длины заголовка; оставляйте `0`. |
| 0x08 | 2 байта | LoaderLenLo | Длина «первичного загрузчика». В простых программах поле оставляют `0`, а сложные загрузчики (gfxview) записывают сюда `LoaderEnd-LoaderStart`, чтобы ОС знала, сколько байт копировать ещё до запуска [gfxview/loader.asm:1-15](../gfxview/loader.asm#L1). |
| 0x0A | 2 байта | LoaderLenHi | Старшие биты длины загрузчика, по умолчанию `0`. |
| 0x0C | 2 байта | Reserved0 | Зарезервировано, заполняется нулями. |
| 0x0E | 2 байта | Reserved1 | Зарезервировано, заполняется нулями. |
| 0x10 | 2 байта | Reserved2 | Зарезервировано, заполняется нулями. |
| 0x12 | 2 байта | LoadAddr | Адрес, в который DSS загрузит тело программы. Обычно совпадает со стартовым адресом (`ORG 0x8100` и т.п.). |
| 0x14 | 2 байта | StartAddr | PC, с которого начнётся исполнение (vector `RST #10` передаст управление сюда). |
| 0x16 | 2 байта | StackAddr | Исходное значение SP. Его часто ставят чуть ниже рабочей области (`StartAddr-1` или `0xC000`) [sprintem/disk/HELLO.ASM:10-20](../sprintem/disk/HELLO.ASM#L10). |
| 0x18..0x1FF | ... | Padding | Свободная область, доводит заголовок до 512 байт. Можно заполнить `DS 512-($-Header)`.

Так как код грузится сразу после заголовка, исходники собирают файл с `ORG LoadAddr-512` — так адреса в теле остаются корректными после добавления заголовка [gfxview/loader.asm:1-15](../gfxview/loader.asm#L1).

## Шаблон заголовка

```asm
        DEVICE  ZXSPECTRUM128
        ORG     #8100-512
EXEHeader:
        DB      "EXE"
        DB      0                       ; format version
        DW      ProgramStart-EXEHeader  ; header size (512)
        DW      0                       ; header size hi
        DW      ProgramEnd-ProgramStart ; primary loader length (0 if unused)
        DW      0,0,0                   ; reserved words
        DW      ProgramStart            ; load address
        DW      ProgramStart            ; entry point (PC)
        DW      ProgramStart-1          ; initial stack pointer
        DS      512-( $-EXEHeader )     ; pad header to 512 bytes

ProgramStart:
        ; ... code ...
ProgramEnd:
```

Этот макет повторяет структуру исходников `gfxview` и `OrgAsm`: та же сигнатура, те же поля, только вместо «магических» чисел использованы вычисляемые смещения [gfxview/loader.asm:1-20](../gfxview/loader.asm#L1) [sprinter_apps/OrgAsm/orgasm.asm:23-60](../sprinter_apps/OrgAsm/orgasm.asm#L23).

## Мини-программа «Hello, World!»

Ниже — минимальный EXE, который печатает строку, ждёт клавишу и выходит через DSS. Он использует готовые `INCLUDE`‑файлы из `programming_guide/include`.

```asm
        DEVICE  ZXSPECTRUM128
        ORG     #8100-512
        INCLUDE "programming_guide/include/dss_equ.inc"

EXEHeader:
        DB      "EXE"
        DB      0
        DW      Start-EXEHeader
        DW      0
        DW      ProgramEnd-Start       ; loader len (0 for bare EXE)
        DW      0,0,0
        DW      Start
        DW      Start
        DW      Start-1
        DS      512-( $-EXEHeader )

Start:
        LD      HL,Msg
        LD      C,Dss.PChars        ; print string
        RST     10h

        LD      C,Dss.WaitKey       ; wait for key (blocking)
        RST     10h

        LD      B,0                 ; exit code 0
        LD      C,Dss.Exit
        RST     10h

Msg:    DB      "Hello, Sprinter!",13,10,0
ProgramEnd:
```

Пример опирается на те же шаги, что и штатный `HELLO.ASM` из каталога `sprintem/disk`: сначала печать через `Dss.PChars`, затем ожидание `Dss.WaitKey` и завершение `Dss.Exit` [sprintem/disk/HELLO.ASM:20-40](../sprintem/disk/HELLO.ASM#L20).

Такой шаблон удобно компилировать `sjasmplus` или `zmac`: заголовок выдаёт корректные адреса, а тело программы можно расширять, добавлять загрузку страниц через `Dss.GetMem` или обращения к BIOS уже после секции `Start`.
