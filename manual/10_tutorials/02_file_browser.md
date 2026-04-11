# 10.2 Туториал: простой файловый браузер

> **Навигация:** [← 10.1 Hello World](01_hello_world.md) | [Оглавление](../README.md) | [10.3 Sprite engine →](03_sprite_engine.md)

---

## Цель

Создать интерактивный браузер файлов:
- Показывает список файлов в текущем каталоге
- Позволяет выбирать стрелками
- Enter открывает файл (показывает размер)
- Esc для выхода

---

## Шаг 1: сбор списка файлов

Используем `F_First #19` и `F_Next #1A` из DSS API для обхода каталога.

```asm
; Собрать список файлов в массив
;   file_count — количество найденных
;   file_list  — массив из 32-байтовых записей F_First

CollectFiles:
        ld      hl, 0
        ld      (file_count), hl

        ld      de, fbuf
        ld      hl, mask
        ld      a, #FF              ; все атрибуты
        ld      b, 0
        ld      c, #19              ; F_First
        rst     #10
        ret     c                   ; ничего не найдено

        ; Скопировать в file_list[0]
        call    CopyEntry

.next:
        ld      hl, (file_count)
        ld      a, l
        cp      MAX_FILES
        ret     nc                  ; достаточно

        ld      de, fbuf
        ld      c, #1A              ; F_Next
        rst     #10
        ret     c

        call    CopyEntry
        jr      .next

; Копировать fbuf в file_list + index * 32
CopyEntry:
        ld      hl, (file_count)
        ; индекс × 32: умножить HL на 32
        add     hl, hl : add hl, hl : add hl, hl : add hl, hl : add hl, hl
        ld      bc, file_list
        add     hl, bc              ; HL = адрес слота
        ex      de, hl              ; DE = слот, HL = fbuf
        ld      hl, fbuf
        ld      bc, 32
        ldir

        ld      hl, (file_count)
        inc     hl
        ld      (file_count), hl
        ret

MAX_FILES       equ 20

mask:           db "*.*", 0
fbuf:           ds 32
file_count:     dw 0
file_list:      ds 32 * MAX_FILES
```

---

## Шаг 2: отображение списка

```asm
ShowList:
        ; Очистить экран (DSS Clear #56)
        ld      c, #56
        xor     a                   ; пробел
        ld      b, #07              ; атрибут: серый на чёрном
        ld      d, 0                ; row
        ld      e, 0                ; col
        ld      h, 25               ; высота
        ld      l, 80               ; ширина
        rst     #10

        ; Шапка
        ld      c, #52              ; Locate
        ld      d, 0
        ld      e, 0
        rst     #10

        ld      hl, header
        ld      c, #5C              ; PChars
        rst     #10

        ; Список
        ld      hl, (file_count)
        ld      a, l
        ld      (count_byte), a

        ld      b, 0                ; индекс
.loop:
        ld      a, b
        ld      hl, count_byte
        cp      (hl)
        ret     nc                  ; все показаны

        push    bc

        ; Курсор на строку B+2
        ld      d, b
        inc     d
        inc     d
        ld      e, 2
        ld      c, #52
        rst     #10

        ; Выделение если B = cursor
        ld      a, (cursor_pos)
        cp      b
        jr      nz, .not_sel

        ; Выделенная строка — пробел и "=>"
        ld      a, '>'
        ld      c, #5B : rst #10

.not_sel:
        ; Напечатать имя файла (11 байт)
        ld      h, 0
        ld      l, b
        ; index * 32:
        add     hl, hl : add hl, hl : add hl, hl : add hl, hl : add hl, hl
        ld      bc, file_list
        add     hl, bc

        ld      b, 11
.name_loop:
        ld      a, (hl)
        push    bc
        push    hl
        ld      c, #5B
        rst     #10
        pop     hl
        pop     bc
        inc     hl
        djnz    .name_loop

        pop     bc
        inc     b
        jr      .loop

header:         db "File Browser — Arrows/Enter/Esc", 13, 10, 0
count_byte:     db 0
cursor_pos:     db 0
```

---

## Шаг 3: обработка клавиш

```asm
HandleInput:
.loop:
        ld      c, #30              ; WaitKey
        rst     #10

        cp      27                  ; ESC
        jr      z, .exit

        cp      13                  ; Enter
        jr      z, .enter

        ; Upper arrow — предполагаем код (зависит от DSS)
        cp      #80                 ; Up
        jr      z, .up

        cp      #81                 ; Down
        jr      z, .down

        jr      .loop

.up:
        ld      a, (cursor_pos)
        or      a
        jr      z, .loop
        dec     a
        ld      (cursor_pos), a
        call    ShowList
        jr      .loop

.down:
        ld      a, (cursor_pos)
        inc     a
        ld      hl, count_byte
        cp      (hl)
        jr      nc, .loop
        ld      (cursor_pos), a
        call    ShowList
        jr      .loop

.enter:
        call    ShowFileInfo
        call    ShowList
        jr      .loop

.exit:
        ret
```

---

## Шаг 4: показ информации о файле

```asm
ShowFileInfo:
        ; Получить адрес выбранной записи
        ld      a, (cursor_pos)
        ld      h, 0
        ld      l, a
        add     hl, hl : add hl, hl : add hl, hl : add hl, hl : add hl, hl
        ld      bc, file_list
        add     hl, bc              ; HL = запись

        ; Сохранить для использования
        push    hl

        ; Очистить нижнюю область
        ld      c, #56
        xor     a
        ld      b, #07
        ld      d, 23
        ld      e, 0
        ld      h, 2
        ld      l, 80
        rst     #10

        ld      c, #52
        ld      d, 23
        ld      e, 0
        rst     #10

        ld      hl, info_label
        ld      c, #5C
        rst     #10

        pop     hl
        push    hl
        ; Имя файла (11 байт)
        ld      b, 11
.name_loop:
        ld      a, (hl)
        push    hl : push bc
        ld      c, #5B : rst #10
        pop     bc : pop hl
        inc     hl
        djnz    .name_loop

        ; Пробел + размер (offset 28 в записи)
        ld      a, ' '
        ld      c, #5B : rst #10

        pop     hl
        ld      bc, 28
        add     hl, bc
        ld      e, (hl) : inc hl    ; младшие 2 байта размера
        ld      d, (hl) : inc hl

        ; Напечатать DE как десятичное
        ex      de, hl
        call    PrintDec
        ld      a, 'b'
        ld      c, #5B : rst #10

        ret

info_label:     db "File: ", 0

; Печать HL как десятичное 16-битное число
PrintDec:
        ld      bc, -10000
        call    .dig
        ld      bc, -1000
        call    .dig
        ld      bc, -100
        call    .dig
        ld      c, -10
        call    .dig
        ld      c, -1
.dig:
        ld      a, '0' - 1
.sub:   inc     a
        add     hl, bc
        jr      c, .sub
        sbc     hl, bc
        push    hl
        push    bc
        ld      c, #5B : rst #10
        pop     bc
        pop     hl
        ret
```

---

## Полный код

```asm
        org     #8100 - 512
        db      "EXE", 0
        dw      #0200, #0200, 0, #8100, main, #BFFF
        ds      512 - 16

        org     #8100

main:
        call    CollectFiles

        xor     a
        ld      (cursor_pos), a

        call    ShowList
        call    HandleInput

        ld      c, #41              ; Exit
        rst     #10

; (далее — все функции из шагов 1-4)
```

---

## Компиляция и запуск

```bash
sjasmplus --raw=browser.exe browser.asm
sprintem browser.exe
```

Создайте в DSS каталог с парой файлов, запустите — и увидите список.

---

## Возможные улучшения

1. **Прокрутка списка** для более чем 20 файлов.
2. **Вход в подкаталоги** при Enter на записи с атрибутом Directory.
3. **Запуск .EXE файлов** через `Dss.Exec #40`.
4. **Сортировка** по имени/размеру/дате.
5. **Фильтр** по расширению.
6. **Подсветка** текущего файла цветом.

---

## Ключевые моменты

> - `F_First #19` / `F_Next #1A` для обхода каталога.
> - Запись каталога — 32 байта: имя 11, атрибуты, дата, кластер, размер.
> - Размер файла — offset 28 (4 байта little-endian).
> - DSS `Locate #52` и `Clear #56` для позиционирования курсора.
> - `WaitKey #30` для ожидания ввода.
> - Каталог (атрибут бит 4) отличается от файла в списке.

---

## Ссылки и источники {#ссылки}

| Источник | Путь / URL |
|---------|-----------|
| FlexNavigator (полный браузер) | https://github.com/witchcraft2001/flexnavigator |
| DSS API | раздел [4.2 DSS API](../04_dss/02_dss_api.md) |
