; DSS call numbers (taken from dss_equ.asm)

Dss.Rst         EQU     10h

Dss.Version     EQU     00h
Dss.ChDisk      EQU     01h
Dss.CurDisk     EQU     02h
Dss.DskInfo     EQU     03h
Dss.BootDsk     EQU     09h

; Files
Dss.Create      EQU     0Ah
Dss.Create_N    EQU     0Bh
Dss.Delete      EQU     0Eh
Dss.Rename      EQU     10h
Dss.Open        EQU     11h
Dss.Close       EQU     12h
Dss.Read        EQU     13h
Dss.Write       EQU     14h
Dss.Move_FP     EQU     15h
Dss.Attrib      EQU     16h
Dss.Get_D_T     EQU     17h
Dss.Put_D_T     EQU     18h
Dss.F_First     EQU     19h
Dss.F_Next      EQU     1Ah
Dss.MkDir       EQU     1Bh
Dss.RmDir       EQU     1Ch
Dss.ChDir       EQU     1Dh
Dss.CurDir      EQU     1Eh
Dss.SysTime     EQU     21h
Dss.SetTime     EQU     22h

; Keyboard
Dss.WaitKey     EQU     30h
Dss.ScanKey     EQU     31h
Dss.EchoKey     EQU     32h
Dss.CtrlKey     EQU     33h
Dss.K_Clear     EQU     35h
Dss.K_Setup     EQU     36h
Dss.TestKey     EQU     37h

; Memory
Dss.SetWin      EQU     38h
Dss.SetWin1     EQU     39h
Dss.SetWin2     EQU     3Ah
Dss.SetWin3     EQU     3Bh
Dss.InfoMem     EQU     3Ch
Dss.GetMem      EQU     3Dh
Dss.FreeMem     EQU     3Eh
Dss.SetMem      EQU     3Fh

; Execution
Dss.Exec        EQU     40h
Dss.Exit        EQU     41h
Dss.Wait        EQU     42h

; Strings / environment
Dss.GSwitch     EQU     43h
Dss.DosName     EQU     44h
Dss.Ex_Path     EQU     45h
Dss.Environ     EQU     46h
Dss.AppInfo     EQU     47h

; Screen
Dss.SetVMod     EQU     50h
Dss.GetVMod     EQU     51h
Dss.Locate      EQU     52h
Dss.Cursor      EQU     53h
Dss.SelPage     EQU     54h
Dss.Scroll      EQU     55h
Dss.Clear       EQU     56h
Dss.RdChar      EQU     57h
Dss.WrChar      EQU     58h
Dss.WinCopy     EQU     59h
Dss.WinRest     EQU     5Ah
Dss.PutChar     EQU     5Bh
Dss.PChars      EQU     5Ch
Dss.Print       EQU     5Fh

; Helpers
FileMode.RW     EQU     00h
FileMode.Read   EQU     01h
FileMode.Write  EQU     02h

FileAttrib.Normal EQU   00h
FileAttrib.RDOnly EQU   01h
FileAttrib.Hidden EQU   02h
FileAttrib.System EQU   04h
FileAttrib.Label  EQU   08h
FileAttrib.Dir    EQU   10h
FileAttrib.Arch   EQU   20h
