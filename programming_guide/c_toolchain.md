# C и кросс-компиляторы

## ZX‑Sprinter SDK (SDCC + sjasmplus)

Официальный SDK (`zx-sprinter-sdk`) поставляется «из коробки»: структура проекта (`Sprites`, `Sound`, `_compile.bat`, `main.c`) описана в README [zx-sprinter-sdk/README.md:1-40](../zx-sprinter-sdk/README.md#L1). Перед сборкой нужно запустить `registerSDK.bat`, чтобы прописать путь в `%ZXSDK%` и сконфигурировать ZXMak2.

Скрипты компиляции используют SDCC и sjasmplus:

```bat
%ZXSDK%\thirdparty\sdcc\bin\sdcc -mz80 --code-loc 0x0006 --data-loc 0 ... main.c -o %TEMP%\out.ihx
%ZXSDK%\thirdparty\sjasmplus\sjasmplus loader.asm
```

[ zx-sprinter-sdk/sdk/compileExe.bat:8-41 ](../zx-sprinter-sdk/sdk/compileExe.bat#L8). Библиотеки (`lib_startup.asm`, `loader.asm`, `wyzplayer.asm`) тоже собираются через sjasmplus [zx-sprinter-sdk/sdk/src/sprinter/_compile.bat:1-3](../zx-sprinter-sdk/sdk/src/sprinter/_compile.bat#L1). В результате `_compile.bat` проекта вызывает `compileExe.bat`, прошивает ресурсные банки и (при необходимости) запускает ZXMak2.

Пример минимальной программы:

```c
#include "sprinter.h"
#include "splib.h"

void main(void) {
    sprites_start();
    wyz_play_music(0);
    unpack_screen(0, 2);
    while(1) {
        sp_UpdateNow();
        check_to_quit(1);
    }
}
```

[samples/StartProject/main.c:1-18](../zx-sprinter-sdk/samples/StartProject/main.c#L1). Эти заголовки предоставляют обёртки поверх системных вызовов (`sprinter.h`), HAL для графики (`splib.h`), плейер PT3 и т.п.

### Z88DK

Порт игры UWOL в `samples/UWOL` содержит комментарии про «двигатель инерции для z88dk/splib2», а код использует типы `u8/u16`, макросы `SPTW` и функции `sp_AttrGet`/`wyz_play_sound`, которые пришли из z88dk [zx-sprinter-sdk/samples/UWOL/main.c:542-600](../zx-sprinter-sdk/samples/UWOL/main.c#L542). Это показывает, что SDK совместим с z88dk‑style API: можно перенести существующие проекты, адаптировав `splib` и платформозависимые части.

### SDCC вручную

Если нужен полный контроль, можно напрямую вызвать SDCC: скрипты `_compileSprinter.bat` и `_compileExe.bat` показывают ключи (`--code-loc 0x0006`, `--data-loc 0`, `--no-std-crt0`, `-I%ZXSDK%\include`). Таким образом легко собрать stand‑alone библиотеку или заменить стартовый код [zx-sprinter-sdk/sdk/src/sprinter/_compileSprinter.bat:1-6](../zx-sprinter-sdk/sdk/src/sprinter/_compileSprinter.bat#L1).

## Solid C и другие CP/M‑инструменты

Исторические проекты (BIOS, услуги Peters Plus) использовали CP/M инструменты M80/L80 и AS80; README BIOS прямо указывает, что для сборки нужен `AS80.EXE` и эмулятор CP/M (`z80mu`) с `M80/L80` [sprinter_bios/README.md:1-16](../sprinter_bios/README.md#L1). Solid C — компилятор из того же стека (CP/M, линковка через L80), поэтому шаги аналогичны: подготовить исходники, собрать `M80`/`L80`, упаковать в `.EXE`. Сценарии `CPMBUILD.BAT`/`SPRBUILD.BAT` в папке Timer показывают, как вызывать `z80mu m80 ...` и `z80mu l80 ...` для автоматической сборки [sprinter_apps/Timer/CPMBUILD.BAT:1-2](../sprinter_apps/Timer/CPMBUILD.BAT#L1).

## Практические советы

1. **Ресурсы и пути** — SDK ожидает строго заданную структуру (`Sprites`, `Sound`). Используйте `img.lst`/`snd.lst`, чтобы описать набор файлов; конвертер сам встроит их в бинарники [zx-sprinter-sdk/README.md:20-60](../zx-sprinter-sdk/README.md#L20).
2. **Запуск на железе** — для реального Sprinter нужно собрать «ветку» с определёнными макросами (`sdk/compileLibs.bat` вызывает sjasmplus с `-DZXMAK` или без). Если требуется ручное обновление BIOS/Flash, смотрите раздел «BIOS API».
3. **Смешанные проекты** — никто не мешает писать «горячие» участки на ASM (через sjasmplus) и вызывать их из SDCC/Z88DK. Достаточно оформить заголовок `extern void draw_sprite(void);` и подключить объект в `sdcc` линковке.

