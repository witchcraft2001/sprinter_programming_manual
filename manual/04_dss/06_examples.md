# 4.6 Практические примеры DSS

> **Навигация:** [← 4.5 EXE формат](05_dss_exe.md) | [Оглавление](../README.md) | [5.1 Видео →](../05_graphics/01_video_overview.md)

---

## Пример 1: Hello World

```asm
DssRst          EQU #10
Dss.PChars      EQU #5C
Dss.WaitKey     EQU #30
Dss.Exit        EQU #41

        org     #8100-512
        db      "EXE", 0
        dw      #0200
        dw      #0200, 0, #8100
        dw      main, #BFFF
        ds      512 - 16

        org     #8100
main:
        ld      hl, msg
        ld      c, Dss.PChars
        rst     DssRst

        ld      c, Dss.WaitKey
        rst     DssRst

        ld      c, Dss.Exit
        rst     DssRst

msg:    db      "Hello, DSS!", 13, 10, 0
```

---

## Пример 2: чтение и печать текстового файла

```asm
        org     #8100-512
        db      "EXE", 0
        dw      #0200, #0200, 0, #8100, main, #BFFF
        ds      512 - 16

        org     #8100

main:
        ; Открыть файл
        ld      hl, filename
        ld      a, #01              ; Read
        ld      c, #11              ; OPEN
        rst     #10
        jr      c, .err
        ld      (handle), a

        ; Читать циклом по 256 байт и печатать
.read_loop:
        ld      a, (handle)
        ld      hl, buf
        ld      de, 256
        ld      c, #13              ; READ
        rst     #10
        jr      c, .read_done

        ld      a, d
        or      e                   ; DE = реально прочитано
        jr      z, .read_done

        ; Напечатать прочитанные байты
        push    de
        ld      hl, buf
        pop     bc                  ; BC = количество
.print:
        ld      a, (hl)
        push    bc
        push    hl
        ld      c, #5B              ; PUTCHAR
        rst     #10
        pop     hl
        pop     bc
        inc     hl
        dec     bc
        ld      a, b
        or      c
        jr      nz, .print

        jr      .read_loop

.read_done:
        ld      a, (handle)
        ld      c, #12              ; CLOSE
        rst     #10

.err:
        ld      c, #41              ; EXIT
        rst     #10

filename:   db "README.TXT", 0
handle:     db 0
buf:        ds 256
```

---

## Пример 3: листинг текущего каталога

```asm
        org     #8100-512
        db      "EXE", 0
        dw      #0200, #0200, 0, #8100, main, #BFFF
        ds      512 - 16

        org     #8100

main:
        ld      de, fbuf
        ld      hl, mask
        ld      a, #FF              ; все файлы и каталоги
        ld      b, 0
        ld      c, #19              ; F_FIRST
        rst     #10
        jr      c, .done

.next_file:
        call    PrintEntry

        ld      de, fbuf
        ld      c, #1A              ; F_NEXT
        rst     #10
        jr      nc, .next_file

.done:
        ld      c, #41
        rst     #10

PrintEntry:
        ; Напечатать имя (11 байт) с атрибутом
        ld      hl, fbuf
        ld      b, 11
.loop:
        ld      a, (hl)
        push    bc
        push    hl
        ld      c, #5B
        rst     #10
        pop     hl
        pop     bc
        inc     hl
        djnz    .loop

        ; Пробел
        ld      a, ' '
        ld      c, #5B
        rst     #10

        ; Показать D (каталог) если атрибут бит 4 = 1
        ld      a, (fbuf+11)        ; атрибуты
        and     #10
        jr      z, .file
        ld      a, 'D'
        jr      .print_type
.file:
        ld      a, 'F'
.print_type:
        ld      c, #5B
        rst     #10

        ; Перевод строки
        ld      a, 13
        ld      c, #5B
        rst     #10
        ld      a, 10
        ld      c, #5B
        rst     #10

        ret

mask:       db  "*.*", 0
fbuf:       ds 32
```

---

## Пример 4: интерактивный ввод

```asm
; Ждать клавишу, отображать на экране, выход по ESC

        org     #8100-512
        db      "EXE", 0
        dw      #0200, #0200, 0, #8100, main, #BFFF
        ds      512 - 16

        org     #8100

main:
        ld      hl, prompt
        ld      c, #5C              ; PCHARS
        rst     #10

.loop:
        ld      c, #30              ; WAITKEY
        rst     #10
        ; A = код клавиши

        cp      27                  ; ESC
        jr      z, .exit

        cp      13                  ; Enter
        jr      nz, .not_enter
        ld      a, 13
        ld      c, #5B
        rst     #10
        ld      a, 10
        ld      c, #5B
        rst     #10
        jr      .loop

.not_enter:
        ld      c, #5B              ; PUTCHAR
        rst     #10
        jr      .loop

.exit:
        ld      c, #41
        rst     #10

prompt: db "Type something (ESC to exit):", 13, 10, 0
```

---

## Пример 5: запись лога с датой и временем

```asm
        org     #8100-512
        db      "EXE", 0
        dw      #0200, #0200, 0, #8100, main, #BFFF
        ds      512 - 16

        org     #8100

main:
        ; Получить текущее время
        ld      c, #21              ; SYSTIME
        rst     #10
        ; D=день, E=месяц, IX=год, H=часы, L=мин, B=сек
        ld      (day), de
        ld      (hour), hl
        ld      a, b
        ld      (sec), a
        ld      (year), ix

        ; Открыть (создать) файл лога
        ld      hl, logfile
        ld      c, #0A              ; CREATE
        rst     #10
        jr      c, .err
        ld      (handle), a

        ; Собрать строку в буфере (дата + сообщение)
        ; Упрощённо: напишем "LOG entry: message\n"
        ld      a, (handle)
        ld      hl, log_msg
        ld      de, log_msg_len
        ld      c, #14              ; WRITE
        rst     #10

        ; Закрыть
        ld      a, (handle)
        ld      c, #12              ; CLOSE
        rst     #10

.err:
        ld      c, #41
        rst     #10

logfile:    db "LOG.TXT", 0
log_msg:    db "LOG entry: Sprinter says hello", 13, 10
log_msg_len equ $-log_msg
handle:     db 0
day:        dw 0
hour:       dw 0
sec:        db 0
year:       dw 0
```

---

## Пример 6: выделение памяти и работа со страницами

```asm
        org     #8100-512
        db      "EXE", 0
        dw      #0200, #0200, 0, #8100, main, #BFFF
        ds      512 - 16

        org     #8100

main:
        ; Выделить 4 страницы (64 КБ)
        ld      b, 4
        ld      c, #3D              ; GETMEM
        rst     #10
        jr      c, .err
        ld      (blk), a

        ; Заполнить все 4 страницы
        ld      b, 0
.page:
        push    bc
        ld      a, (blk)
        ld      c, #3B              ; SETWIN3
        rst     #10

        ; WIN3 теперь содержит страницу B блока blk
        ld      hl, #C000
        ld      de, #C001
        ld      bc, #3FFF
        ld      a, (.pattern)
        ld      (hl), a
        ldir

        ; Инкремент паттерна
        ld      a, (.pattern)
        inc     a
        ld      (.pattern), a

        pop     bc
        inc     b
        ld      a, b
        cp      4
        jr      c, .page

        ; Освободить
        ld      a, (blk)
        ld      c, #3E              ; FREEMEM
        rst     #10

.err:
        ld      c, #41
        rst     #10

.pattern:   db #AA
blk:        db 0
```

---

## Пример 7: запуск внешней программы

```asm
        org     #8100-512
        db      "EXE", 0
        dw      #0200, #0200, 0, #8100, main, #BFFF
        ds      512 - 16

        org     #8100

main:
        ld      hl, hello_msg
        ld      c, #5C
        rst     #10

        ; Запустить TEST.EXE с аргументом
        ld      hl, exec_cmd
        ld      c, #40              ; EXEC
        rst     #10
        jr      c, .err

        ld      hl, done_msg
        ld      c, #5C
        rst     #10
        jr      .exit

.err:
        ld      hl, err_msg
        ld      c, #5C
        rst     #10

.exit:
        ld      c, #41
        rst     #10

hello_msg:  db  "Running TEST.EXE...", 13, 10, 0
done_msg:   db  "Back from TEST.EXE", 13, 10, 0
err_msg:    db  "EXEC failed!", 13, 10, 0
exec_cmd:   db  "TEST.EXE hello", 0
```

---

## Пример 8: печать аргументов командной строки

```asm
        org     #8100-512
        db      "EXE", 0
        dw      #0200, #0200, 0, #8100, main, #BFFF
        ds      512 - 16

        org     #8100

main:
        ld      hl, label
        ld      c, #5C
        rst     #10

        ; Аргументы в #8080
        ld      hl, #8080
        ld      a, (hl)             ; длина
        or      a
        jr      z, .no_args
        inc     hl

.loop:
        ld      a, (hl)
        or      a
        jr      z, .done
        push    hl
        ld      c, #5B
        rst     #10
        pop     hl
        inc     hl
        jr      .loop

.no_args:
        ld      hl, none
        ld      c, #5C
        rst     #10

.done:
        ld      a, 13 : ld c, #5B : rst #10
        ld      a, 10 : ld c, #5B : rst #10

        ld      c, #41
        rst     #10

label:  db "Args: ", 0
none:   db "(none)", 0
```

---

## Пример 9: копирование файла

```asm
        org     #8100-512
        db      "EXE", 0
        dw      #0200, #0200, 0, #8100, main, #BFFF
        ds      512 - 16

        org     #8100

main:
        ; Открыть источник
        ld      hl, src_name
        ld      a, #01              ; Read
        ld      c, #11              ; OPEN
        rst     #10
        jr      c, .err
        ld      (src_h), a

        ; Создать приёмник
        ld      hl, dst_name
        ld      c, #0A              ; CREATE
        rst     #10
        jr      c, .err_close_src
        ld      (dst_h), a

.copy_loop:
        ld      a, (src_h)
        ld      hl, buf
        ld      de, 1024
        ld      c, #13              ; READ
        rst     #10
        ld      a, d
        or      e
        jr      z, .done            ; 0 байт = EOF

        ld      a, (dst_h)
        ld      hl, buf
        ; DE уже = количество прочитанных байт
        ld      c, #14              ; WRITE
        rst     #10

        jr      .copy_loop

.done:
        ld      a, (dst_h)
        ld      c, #12 : rst #10    ; CLOSE dst
.err_close_src:
        ld      a, (src_h)
        ld      c, #12 : rst #10    ; CLOSE src
.err:
        ld      c, #41
        rst     #10

src_name:   db "SOURCE.BIN", 0
dst_name:   db "TARGET.BIN", 0
src_h:      db 0
dst_h:      db 0
buf:        ds 1024
```

---

## Пример 10: файловый менеджер (шаблон)

```asm
; Интерактивный мини-файловый-менеджер:
; - показать список файлов
; - стрелки для выбора
; - Enter для запуска .EXE
; - ESC для выхода

        org     #8100-512
        db      "EXE", 0
        dw      #0200, #0200, 0, #8100, main, #BFFF
        ds      512 - 16

        org     #8100

main:
        ; 1. Очистить экран
        ld      c, #56              ; CLEAR
        ld      a, ' '
        ld      b, #07              ; атрибут
        ld      d, 0                ; start row
        ld      e, 0                ; start col
        ld      h, 25               ; высота
        ld      l, 80               ; ширина
        rst     #10

        ; 2. Получить первый файл
        ld      de, file_entry
        ld      hl, mask
        ld      a, #FF
        ld      b, 0
        ld      c, #19              ; F_FIRST
        rst     #10
        jr      c, .no_files

        ld      a, 0                ; текущий индекс строки
        ld      (cur_row), a

.list_loop:
        ; Переместить курсор
        ld      a, (cur_row)
        ld      d, a
        ld      e, 2
        ld      c, #52              ; LOCATE
        rst     #10

        ; Напечатать имя
        ld      hl, file_entry
        ld      b, 11
.name_loop:
        ld      a, (hl)
        push    hl
        push    bc
        ld      c, #5B
        rst     #10
        pop     bc
        pop     hl
        inc     hl
        djnz    .name_loop

        ld      a, (cur_row)
        inc     a
        ld      (cur_row), a
        cp      20
        jr      nc, .wait_key

        ; Следующий файл
        ld      de, file_entry
        ld      c, #1A              ; F_NEXT
        rst     #10
        jr      nc, .list_loop

.wait_key:
        ld      c, #30              ; WAITKEY
        rst     #10
        cp      27                  ; ESC
        jr      z, .exit
        jr      .wait_key

.no_files:
        ld      hl, no_files_msg
        ld      c, #5C
        rst     #10
        ld      c, #30
        rst     #10

.exit:
        ld      c, #41
        rst     #10

mask:           db "*.*", 0
no_files_msg:   db "No files found", 13, 10, 0
cur_row:        db 0
file_entry:     ds 32
```

---

## Ключевые моменты

> - Все DSS программы начинаются с 512-байтного EXE заголовка.
> - Файлы: Open/Read/Write/Close + Move_FP для seek.
> - Листинг каталога: F_First → цикл F_Next до CF=1.
> - Клавиатура: WAITKEY (блокирует), SCANKEY (неблок.).
> - Память: GetMem → SetWin3 → работа → FreeMem (или автоматическое освобождение при EXIT).
> - EXEC для запуска дочерних программ, EXIT для выхода.
> - Аргументы командной строки по адресу #8080 (длина + текст).

---

## Ссылки и источники {#ссылки}

| Источник | Путь / URL |
|---------|-----------|
| DSS примеры | https://gitlab.com/mikhaltchenkov/dos |
| SDK примеры | https://github.com/ENgineE777/zx-sprinter-sdk |
| Flappybird | https://github.com/witchcraft2001/flappybird |
| Текстовый редактор | https://github.com/witchcraft2001/sprinter-texteditor |
| FlexNavigator | https://github.com/witchcraft2001/flexnavigator |
