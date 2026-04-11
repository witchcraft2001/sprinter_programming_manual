; AT keyboard (Set 2) scancodes for Sprinter Z84C15 input
; Prefix 0xF0 indicates key release, 0xE0 indicates extended key.
; Source: IBM Set 2 tables (see docs/Sprinter_Programming.pdf and PC references).

SC_ESC          EQU 076h
SC_F1           EQU 005h
SC_F2           EQU 006h
SC_F3           EQU 004h
SC_F4           EQU 00Ch
SC_F5           EQU 003h
SC_F6           EQU 00Bh
SC_F7           EQU 083h
SC_F8           EQU 00Ah
SC_F9           EQU 001h
SC_F10          EQU 009h
SC_F11          EQU 078h
SC_F12          EQU 007h

SC_GRAVE        EQU 00Eh  ; `~
SC_1            EQU 016h
SC_2            EQU 01Eh
SC_3            EQU 026h
SC_4            EQU 025h
SC_5            EQU 02Eh
SC_6            EQU 036h
SC_7            EQU 03Dh
SC_8            EQU 03Eh
SC_9            EQU 046h
SC_0            EQU 045h
SC_MINUS        EQU 04Eh
SC_EQUAL        EQU 055h
SC_BACKSPACE    EQU 066h
SC_TAB          EQU 00Dh

SC_Q            EQU 015h
SC_W            EQU 01Dh
SC_E            EQU 024h
SC_R            EQU 02Dh
SC_T            EQU 02Ch
SC_Y            EQU 035h
SC_U            EQU 03Ch
SC_I            EQU 043h
SC_O            EQU 044h
SC_P            EQU 04Dh
SC_LBRACKET     EQU 054h
SC_RBRACKET     EQU 05Bh
SC_BACKSLASH    EQU 05Dh

SC_CAPSLOCK     EQU 058h
SC_A            EQU 01Ch
SC_S            EQU 01Bh
SC_D            EQU 023h
SC_F            EQU 02Bh
SC_G            EQU 034h
SC_H            EQU 033h
SC_J            EQU 03Bh
SC_K            EQU 042h
SC_L            EQU 04Bh
SC_SEMICOLON    EQU 04Ch
SC_APOSTROPHE   EQU 052h
SC_ENTER        EQU 05Ah

SC_LSHIFT       EQU 012h
SC_Z            EQU 01Ah
SC_X            EQU 022h
SC_C            EQU 021h
SC_V            EQU 02Ah
SC_B            EQU 032h
SC_N            EQU 031h
SC_M            EQU 03Ah
SC_COMMA        EQU 041h
SC_PERIOD       EQU 049h
SC_SLASH        EQU 04Ah
SC_RSHIFT       EQU 059h

SC_LCTRL        EQU 014h
SC_LALT         EQU 011h
SC_SPACE        EQU 029h
SC_RALT         EQU 011h   ; with 0xE0 prefix
SC_RCTRL        EQU 014h   ; with 0xE0 prefix
SC_MENU         EQU 02Eh   ; with 0xE0 prefix (context menu)
SC_LGUI         EQU 01Fh   ; with 0xE0 prefix (Win key)
SC_RGUI         EQU 027h   ; with 0xE0 prefix

SC_PRINTSCREEN  EQU 07Ch   ; sequence E0 12 E0 7C (press)
SC_SCROLLLOCK   EQU 07Eh
SC_PAUSE        EQU 077h   ; sends E1 sequence
SC_NUMLOCK      EQU 077h

; Navigation (E0 prefix)
SC_INSERT       EQU 070h
SC_DELETE       EQU 071h
SC_HOME         EQU 06Ch
SC_END          EQU 069h
SC_PAGEUP       EQU 07Dh
SC_PAGEDOWN     EQU 07Ah
SC_ARROWUP      EQU 075h
SC_ARROWDOWN    EQU 072h
SC_ARROWLEFT    EQU 06Bh
SC_ARROWRIGHT   EQU 074h

; Keypad (without prefixes unless noted)
SC_KP_SLASH     EQU 04Ah   ; with E0 prefix
SC_KP_ASTERISK  EQU 07Ch
SC_KP_MINUS     EQU 07Bh
SC_KP_PLUS      EQU 079h
SC_KP_ENTER     EQU 05Ah   ; with E0 prefix
SC_KP_1         EQU 069h
SC_KP_2         EQU 072h
SC_KP_3         EQU 07Ah
SC_KP_4         EQU 06Bh
SC_KP_5         EQU 073h
SC_KP_6         EQU 074h
SC_KP_7         EQU 06Ch
SC_KP_8         EQU 075h
SC_KP_9         EQU 07Dh
SC_KP_0         EQU 070h
SC_KP_PERIOD    EQU 071h

SC_PREFIX_EXT   EQU 0E0h
SC_PREFIX_BREAK EQU 0F0h
