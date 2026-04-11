; Sprinter ports and control registers (matching names used in sources)

EmmWin.P0      EQU     082h    ; window #0000-3FFF
EmmWin.P1      EQU     0A2h    ; window #4000-7FFF
EmmWin.P2      EQU     0C2h    ; window #8000-BFFF
EmmWin.P3      EQU     0E2h    ; window #C000-FFFF (PAGE3)

RGADR          EQU     089h    ; video line selector (PORT_Y)
RGMOD          EQU     0C9h    ; video mode register / page select
SYS_PORT_OFF   EQU     03Ch    ; system control (map 0)
SYS_PORT_ON    EQU     07Ch    ; system control (map 1)

Covox.Port     EQU     0FBh    ; mono Covox / CBL data
CBL.Ctrl       EQU     04Eh    ; Covox-Blaster mode register
AY.PortA       EQU     0BFFD
AY.PortC       EQU     0FFFD

ISA.Port       EQU     09FBDh  ; ISA control (A14-A19, RESET/AEN)
ISA.System     EQU     01FFDh  ; system port for ISA window setup

CMOS.DataRead  EQU     0FFBDh
CMOS.DataWrite EQU     0BFBDh
CMOS.AdrWrite  EQU     0DFBDh

VG93.Port1F    EQU     01Fh
VG93.Port3F    EQU     03Fh
VG93.Port5F    EQU     05Fh
VG93.Port7F    EQU     07Fh

IDE.Data       EQU     050h
IDE.Status     EQU     051h
IDE.SectorCnt  EQU     052h
IDE.SectorNum  EQU     053h
IDE.CylLow     EQU     054h
IDE.CylHigh    EQU     055h
IDE.HeadDrv    EQU     056h
IDE.Command    EQU     057h
IDE.Ctrl3F6    EQU     3F6h
IDE.Alt3F7     EQU     3F7h

; Convenience aliases
PAGE3          EQU     EmmWin.P3
