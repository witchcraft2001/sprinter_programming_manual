; ZX keyboard matrix scancodes (port #FE)
; Rows correspond to address bus masks (A15..A8) described in generated_doc/peripherals.md.
; Usage:
;   LD   A,KBD_ROW_SHIFT
;   IN   A,(#FE)
;   BIT  KBD_SHIFT_BIT,A    ; 0 = key pressed
;
KBD_ROW_SHIFT   EQU 0FEh
KBD_ROW_ASDFG   EQU 0FDh
KBD_ROW_QWERT   EQU 0FBh
KBD_ROW_12345   EQU 0F7h
KBD_ROW_09876   EQU 0EFh
KBD_ROW_POIUY   EQU 0DFh
KBD_ROW_ENTER   EQU 0BFh
KBD_ROW_SPACE   EQU 07Fh

KBD_SHIFT_BIT   EQU 0
KBD_Z_BIT       EQU 1
KBD_X_BIT       EQU 2
KBD_C_BIT       EQU 3
KBD_V_BIT       EQU 4

KBD_A_BIT       EQU 0
KBD_S_BIT       EQU 1
KBD_D_BIT       EQU 2
KBD_F_BIT       EQU 3
KBD_G_BIT       EQU 4

KBD_Q_BIT       EQU 0
KBD_W_BIT       EQU 1
KBD_E_BIT       EQU 2
KBD_R_BIT       EQU 3
KBD_T_BIT       EQU 4

KBD_1_BIT       EQU 0
KBD_2_BIT       EQU 1
KBD_3_BIT       EQU 2
KBD_4_BIT       EQU 3
KBD_5_BIT       EQU 4

KBD_0_BIT       EQU 0   ; row 0/9/8/7/6
KBD_9_BIT       EQU 1
KBD_8_BIT       EQU 2
KBD_7_BIT       EQU 3
KBD_6_BIT       EQU 4

KBD_P_BIT       EQU 0
KBD_O_BIT       EQU 1
KBD_I_BIT       EQU 2
KBD_U_BIT       EQU 3
KBD_Y_BIT       EQU 4

KBD_ENTER_BIT   EQU 0
KBD_L_BIT       EQU 1
KBD_K_BIT       EQU 2
KBD_J_BIT       EQU 3
KBD_H_BIT       EQU 4

KBD_SPACE_BIT   EQU 0
KBD_SYM_BIT     EQU 1
KBD_M_BIT       EQU 2
KBD_N_BIT       EQU 3
KBD_B_BIT       EQU 4
