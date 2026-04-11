; Sprinter ports and control registers (matching names used in sources)

EmmWin.P0      EQU     082h    ; memory window #0000-3FFF
EmmWin.P1      EQU     0A2h    ; memory window #4000-7FFF
EmmWin.P2      EQU     0C2h    ; memory window #8000-BFFF
EmmWin.P3      EQU     0E2h    ; memory window #C000-FFFF (PAGE3)

RGADR          EQU     089h    ; video line selector (PORT_Y)
RGMOD          EQU     0C9h    ; video mode register / VRAM bank select
SYS_PORT_OFF   EQU     03Ch    ; disable system map (map 0)
SYS_PORT_ON    EQU     07Ch    ; enable system map (map 1)

Covox.Port     EQU     0FBh    ; mono Covox / Covox-Blaster DAC data
CBL.Ctrl       EQU     04Eh    ; Covox-Blaster control register
AY.PortA       EQU     0BFFDh  ; AY-3-8910 data port
AY.PortC       EQU     0FFFDh  ; AY-3-8910 register select

ISA.Port       EQU     09FBDh  ; ISA control (A14-A19, RESET/AEN)
ISA.System     EQU     01FFDh  ; ISA system setup / window latch

CMOS.DataRead  EQU     0FFBDh  ; CMOS read data port
CMOS.DataWrite EQU     0BFBDh  ; CMOS write data port
CMOS.AdrWrite  EQU     0DFBDh  ; CMOS index register

VG93.Port1F    EQU     01Fh    ; VG93 status/command
VG93.Port3F    EQU     03Fh    ; VG93 track register
VG93.Port5F    EQU     05Fh    ; VG93 sector register
VG93.Port7F    EQU     07Fh    ; VG93 data register

IDE.Data       EQU     050h    ; IDE data register
IDE.Status     EQU     051h    ; IDE status/command mirror
IDE.SectorCnt  EQU     052h    ; IDE sector count
IDE.SectorNum  EQU     053h    ; IDE sector number (LBA low)
IDE.CylLow     EQU     054h    ; IDE cylinder low (LBA mid)
IDE.CylHigh    EQU     055h    ; IDE cylinder high (LBA high)
IDE.HeadDrv    EQU     056h    ; IDE head / drive select
IDE.Command    EQU     057h    ; IDE command register
IDE.Ctrl3F6    EQU     3F6h    ; IDE device control (reset, nIEN)
IDE.Alt3F7     EQU     3F7h    ; IDE alternate status

; Convenience aliases
PAGE3          EQU     EmmWin.P3
