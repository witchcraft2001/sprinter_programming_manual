; BIOS call numbers (matching bios_equ.asm)

Bios.Rst          EQU    08h

; EMS / memory windows
Bios.Emm_Fn0      EQU    0C0h
Bios.Emm_Fn1      EQU    0C1h
Bios.Emm_Fn2      EQU    0C2h
Bios.Emm_Fn3      EQU    0C3h
Bios.Emm_Fn4      EQU    0C4h
Bios.Emm_Fn5      EQU    0C5h
Bios.Emm_Fn6      EQU    0C6h
Bios.Emm_Fn7      EQU    0C7h
Bios.Emm_Fn8      EQU    0C8h
Bios.Emm_Fn9      EQU    0C9h

; Window/text services
Bios.Win_Open        EQU  0B0h
Bios.Win_Close       EQU  0B1h
Bios.Win_Copy_Win    EQU  0B2h
Bios.Win_Restore_Win EQU  0B3h
Bios.Win_Get_Sym     EQU  0B4h
Bios.Win_Put_Sym     EQU  0B5h
Bios.Win_Set_ZG      EQU  0B6h
Bios.Win_Move_Win    EQU  0B7h
Bios.Win_Get_ZG      EQU  0B8h

Bios.Lp_Print_All    EQU  081h
Bios.Lp_Print_Sym    EQU  082h
Bios.Lp_Print_Atr    EQU  083h
Bios.Lp_Set_Place    EQU  084h
Bios.Lp_Print_Ln     EQU  085h
Bios.Lp_Print_Ln2    EQU  086h
Bios.Lp_Print_Ln3    EQU  087h
Bios.Lp_Print_Ln4    EQU  088h
Bios.Lp_Cls_Win      EQU  089h
Bios.Lp_Scroll_Up    EQU  08Ah
Bios.Lp_Print_Ln5    EQU  08Bh
Bios.Lp_Print_Ln6    EQU  08Ch
Bios.Lp_Cls_Win2     EQU  08Dh
Bios.Lp_Get_Place    EQU  08Eh

; Drive services
Bios.Drv_Reset       EQU  051h
Bios.Drv_Verify      EQU  054h
Bios.Drv_Read        EQU  055h
Bios.Drv_Write       EQU  056h
Bios.Drv_Detect      EQU  057h
Bios.Drv_Get_Par     EQU  058h
Bios.Drv_Set_Par     EQU  059h
Bios.Ext_Version     EQU  05Ah
Bios.Drv_List        EQU  05Fh
