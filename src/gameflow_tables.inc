; Tables used by state flow, input translation, and menu rendering.

name_entry_keymap:
        .db '0','1','2','3','4','5','6','7'
        .db '8','9','-','=',92,0,0,0
        .db 0,0,0,'.',0,0,'A','B'
        .db 'C','D','E','F','G','H','I','J'
        .db 'K','L','M','N','O','P','Q','R'
        .db 'S','T','U','V','W','X','Y','Z'
        .db 0,0,0,0,0,0,0,0
        .db 0,0,8,0,0,0,0,13
        .db ' ',0,0,0,0,0,0,0
        .db 0,0,0,0,0,0,0,0
        .db 0,0,0,0,0,0,0,0

time_limit_seconds_lo:
        .db 0, 60, 44, 88

time_limit_seconds_hi:
        .db 0, 0, 1, 2

time_limit_name_ptrs:
        .dw str_endless, str_1_min, str_5_min, str_10_min
