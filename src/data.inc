; Tables and text shared across gameplay, menu, and rendering code.

row_offsets:
        .dw 0, 32, 64, 96, 128, 160, 192, 224
        .dw 256, 288, 320, 352, 384, 416, 448, 480
        .dw 512, 544, 576, 608, 640, 672, 704, 736

lane_rows_1p:
        .db 10, 10, 10, 10

lane_rows_2p:
        .db 6, 15, 15, 15

lane_rows_3p:
        .db 4, 11, 18, 18

lane_rows_4p:
        .db 4, 9, 14, 19

player_sprite_colors:
        .db 11, 3, 13, 15

dot_positions:
        .db 5, 11, 17, 23, 29, 35, 41, 47
        .db 53, 59, 65, 71, 77, 83, 89, 95

dot_tile_indices:
        .db 1, 3, 4, 6, 7, 9, 10, 12
        .db 13, 15, 16, 18, 19, 21, 22, 24

dot_masks_lo:
        .db 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
        .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

dot_masks_hi:
        .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        .db 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80

lane_ptrs:
        .dw LANE0, LANE1, LANE2, LANE3

mode_player_counts:
        .db 1, 2, 4, 2, 4

mode_round_seconds:
        .db 0, 60, 60, 45, 45

mode_round_limits:
        .db 1, 1, 1, 5, 3

mode_series_types:
        .db SERIES_ENDLESS, SERIES_SPRINT, SERIES_SPRINT, SERIES_CHAMP, SERIES_LEAGUE

mode_name_ptrs:
        .dw str_mode_1p, str_mode_2p, str_mode_4p, str_mode_champ, str_mode_league

player_name_ptrs:
        .dw str_p1, str_p2, str_p3, str_p4

str_title:
        .ascii "PAKU PAKU MSX"
        .db 0
str_hi_score:
        .ascii "HI SCORE"
        .db 0
str_select_mode:
        .ascii "SELECT MODE"
        .db 0
str_time:
        .ascii "TIME"
        .db 0
str_demo:
        .ascii "DEMO"
        .db 0
str_best:
        .ascii "BEST"
        .db 0
str_start:
        .ascii "START"
        .db 0
str_players:
        .ascii "NUMBER OF PLAYERS"
        .db 0
str_time_limit:
        .ascii "TIME LIMIT"
        .db 0
str_players_head:
        .ascii "PLAYERS"
        .db 0
str_endless:
        .ascii "ENDLESS"
        .db 0
str_1_min:
        .ascii "1 MIN"
        .db 0
str_5_min:
        .ascii "5 MIN"
        .db 0
str_10_min:
        .ascii "10 MIN"
        .db 0
str_best_scores:
        .ascii "BEST SCORES"
        .db 0
str_no_scores:
        .ascii "NO SCORES YET"
        .db 0
str_press_any:
        .ascii "PRESS ANY KEY"
        .db 0
str_any_key_menu:
        .ascii "ANY KEY MENU"
        .db 0
str_menu_help_1:
        .ascii "UP/DOWN MOVE"
        .db 0
str_menu_help_2:
        .ascii "RET/SPACE START"
        .db 0
str_menu_help_3:
        .ascii "L/R/RET/SPACE SET"
        .db 0
str_menu_help_4:
        .ascii "5 SEC IDLE = SCORES"
        .db 0
str_bind_head:
        .ascii "BIND KEYS"
        .db 0
str_get_ready:
        .ascii "GET READY"
        .db 0
str_wait:
        .ascii "WAIT"
        .db 0
str_ready:
        .ascii "READY"
        .db 0
str_press_a_key:
        .ascii "PRESS A KEY"
        .db 0
str_time_up:
        .ascii "TIME UP"
        .db 0
str_winner_round:
        .ascii "WINNER OF THE ROUND"
        .db 0
str_top10_name:
        .ascii "TOP-10! NAME: "
        .db 0
str_new_best_name:
        .ascii "NEW BEST! NAME: "
        .db 0
str_enter_name:
        .ascii "ENTER NAME"
        .db 0
str_esc_to_end:
        .ascii "ESC TO QUIT"
        .db 0
str_esc_again_quit:
        .ascii "ESC AGAIN QUIT"
        .db 0
str_start_bang:
        .ascii "START!"
        .db 0
str_dot:
        .ascii "."
        .db 0
str_round:
        .ascii "R"
        .db 0
str_hi:
        .ascii "HI"
        .db 0
str_round_over:
        .ascii "ROUND OVER"
        .db 0
str_series_over:
        .ascii "SERIES OVER"
        .db 0
str_press_next:
        .ascii "SPACE OR RET = NEXT ROUND"
        .db 0
str_press_menu:
        .ascii "SPACE OR RET = MENU"
        .db 0
str_start_help:
        .ascii "RET/SPACE START"
        .db 0
str_pick_help:
        .ascii "UP/DOWN PICK MODE"
        .db 0
str_ctrl_help:
        .ascii "P1 SPACE P2 RET P3 F1 P4 F2"
        .db 0
str_rank_head:
        .ascii "RK PL SCORE"
        .db 0
str_pts_head:
        .ascii "PTS"
        .db 0
str_sc:
        .ascii "SC"
        .db 0
str_x:
        .ascii "X"
        .db 0
str_pt:
        .ascii "PT"
        .db 0
str_out:
        .ascii "OUT"
        .db 0

str_mode_1p:
        .ascii "1P ENDLESS"
        .db 0
str_mode_2p:
        .ascii "2P BEST IN 1 MIN"
        .db 0
str_mode_4p:
        .ascii "4P BEST IN 1 MIN"
        .db 0
str_mode_champ:
        .ascii "2P CHAMPIONSHIP"
        .db 0
str_mode_league:
        .ascii "4P LEAGUE"
        .db 0

str_p1:
        .ascii "P1"
        .db 0
str_p2:
        .ascii "P2"
        .db 0
str_p3:
        .ascii "P3"
        .db 0
str_p4:
        .ascii "P4"
        .db 0
