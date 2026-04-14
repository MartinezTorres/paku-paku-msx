; Menu, score screen, session, and name-entry rendering for the active flow.

render_state:
        jp render_state_new

render_state_new:
        call clear_screen_buffer
        ld a, (STATE_ID)
        cp #STATE_MENU
        jp z, render_menu_new
        cp #STATE_SCORES
        jp z, render_scores_screen_new
        jp render_session_new

render_menu_new:
        ld a, #24
        ld (VISIBLE_ROWS), a
        call render_menu_title_rows
        ld hl, #str_hi_score
        ld d, #8
        ld e, #2
        call draw_string
        ld hl, (HI_SCORE_L)
        ld d, #8
        ld e, #12
        call draw_u16_5

        ld a, (MENU_INDEX)
        cp #0
        jr nz, render_menu_cursor_1
        ld a, #TILE_CURSOR
        ld d, #11
        ld e, #3
        call draw_char
render_menu_cursor_1:
        ld a, (MENU_INDEX)
        cp #1
        jr nz, render_menu_cursor_2
        ld a, #TILE_CURSOR
        ld d, #13
        ld e, #3
        call draw_char
render_menu_cursor_2:
        ld a, (MENU_INDEX)
        cp #2
        jr nz, render_menu_items
        ld a, #TILE_CURSOR
        ld d, #15
        ld e, #3
        call draw_char
render_menu_items:
        ld hl, #str_start
        ld d, #11
        ld e, #6
        call draw_string
        ld hl, #str_players
        ld d, #13
        ld e, #6
        call draw_string
        ld a, (PLAYER_COUNT_V)
        add a, #'0'
        ld d, #13
        ld e, #24
        call draw_char
        ld hl, #str_time_limit
        ld d, #15
        ld e, #6
        call draw_string
        ld a, (TIME_LIMIT_IDX)
        ld hl, #time_limit_name_ptrs
        call get_word_from_table
        ld d, #15
        ld e, #19
        call draw_string

        ld hl, #str_menu_help_1
        ld d, #20
        ld e, #4
        call draw_string
        ld hl, #str_menu_help_2
        ld d, #21
        ld e, #2
        call draw_string
        ld hl, #str_menu_help_3
        ld d, #22
        ld e, #5
        call draw_string
        ld hl, #str_menu_help_4
        ld d, #23
        ld e, #4
        call draw_string
        ret

render_scores_screen_new:
        ld a, #24
        ld (VISIBLE_ROWS), a
        call render_menu_title_rows
        ld hl, #str_best_scores
        ld d, #8
        ld e, #10
        call draw_string
        ld a, (HISCORE_COUNT)
        or a
        jr nz, render_scores_rows
        ld hl, #str_no_scores
        ld d, #13
        ld e, #10
        call draw_string
        jr render_scores_footer
render_scores_rows:
        ld b, #0
render_scores_loop:
        ld a, b
        cp #10
        jr z, render_scores_footer
        ld c, a
        ld a, (HISCORE_COUNT)
        cp c
        jr z, render_scores_footer
        jr c, render_scores_footer
        push bc
        ld d, #10
        ld a, b
        add a, d
        ld d, a
        ld a, b
        inc a
        ld e, #2
        call draw_u8_2
        ld hl, #str_dot
        ld d, #10
        ld a, b
        add a, d
        ld d, a
        ld e, #4
        call draw_string
        ld a, b
        call get_hiscore_ptr_from_index_new
        inc hl
        inc hl
        ld d, #10
        ld a, b
        add a, d
        ld d, a
        ld e, #6
        ld b, #HISCORE_NAME_LEN
        call draw_fixed_chars_new
        ld a, b
        pop bc
        push bc
        ld a, b
        call get_hiscore_ptr_from_index_new
        ld e, (hl)
        inc hl
        ld d, (hl)
        ex de, hl
        ld d, #10
        ld a, b
        add a, d
        ld d, a
        ld e, #18
        call draw_u16_5
        pop bc
        inc b
        jr render_scores_loop
render_scores_footer:
        ld hl, #str_press_any
        ld d, #22
        ld e, #6
        call draw_string
        ret

render_session_new:
        call set_session_visible_rows_new
        call render_session_header_new
        ld a, (STATE_ID)
        cp #STATE_PLAY
        jr z, render_session_play_lanes
        xor a
render_session_msg_loop:
        ld c, a
        ld a, (PLAYER_COUNT_V)
        cp c
        ld a, c
        ret z
        ret c
        push af
        call render_message_lane_new
        pop af
        inc a
        jr render_session_msg_loop
render_session_play_lanes:
        xor a
render_session_play_loop:
        ld c, a
        ld a, (PLAYER_COUNT_V)
        cp c
        ld a, c
        ret z
        ret c
        push af
        call render_one_lane
        pop af
        inc a
        jr render_session_play_loop

set_session_visible_rows_new:
        ld a, (PLAYER_COUNT_V)
        dec a
        ld (LANE_INDEX_TMP), a
        call get_lane_row
        ld b, a
        ld a, (STATE_ID)
        cp #STATE_NAME
        ld a, b
        jr z, set_session_visible_name
        add a, #4
        ld (VISIBLE_ROWS), a
        ret
set_session_visible_name:
        add a, #5
        ld (VISIBLE_ROWS), a
        ret

render_session_header_new:
        ld hl, #str_players_head
        ld d, #0
        ld e, #0
        call draw_string
        ld a, (PLAYER_COUNT_V)
        add a, #'0'
        ld d, #0
        ld e, #8
        call draw_char

        ld a, (TIME_LIMIT_L)
        ld b, a
        ld a, (TIME_LIMIT_H)
        or b
        jr z, render_session_header_endless
        ld hl, #str_time
        ld d, #0
        ld e, #20
        call draw_string
        ld hl, (TIMER_REM_L)
        ld d, #0
        ld e, #25
        call draw_time_mmss_new
        jr render_session_header_best
render_session_header_endless:
        ld hl, #str_endless
        ld d, #0
        ld e, #21
        call draw_string
render_session_header_best:
        ld hl, #str_best
        ld d, #1
        ld e, #0
        call draw_string
        ld hl, (HI_SCORE_L)
        ld d, #1
        ld e, #5
        call draw_u16_5
        ld a, (STATE_ID)
        cp #STATE_PLAY
        jr nz, render_session_header_state
        ld a, (QUIT_CONFIRM_TICKS)
        or a
        ld hl, #str_esc_to_end
        jr z, render_session_header_quit
        ld hl, #str_esc_again_quit
render_session_header_quit:
        ld d, #1
        ld e, #15
        call draw_string
        ret
render_session_header_state:
        cp #STATE_ASSIGN
        jr nz, render_session_header_cd
        ld hl, #str_bind_head
        ld d, #1
        ld e, #16
        call draw_string
        ret
render_session_header_cd:
        cp #STATE_COUNTDOWN
        jr nz, render_session_header_win
        ld hl, #str_get_ready
        ld d, #1
        ld e, #18
        call draw_string
        ret
render_session_header_win:
        cp #STATE_WINNER
        jr nz, render_session_header_name
        ld hl, #str_time_up
        ld d, #1
        ld e, #20
        call draw_string
        ret
render_session_header_name:
        ld hl, #str_enter_name
        ld d, #1
        ld e, #16
        call draw_string
        ret

render_message_lane_new:
        ld (LANE_INDEX_TMP), a
        push af
        call set_ix_to_lane_a
        pop af
        call get_lane_row
        ld (LANE_ROW_TMP), a
        ld a, (LANE_INDEX_TMP)
        ld hl, #player_name_ptrs
        call get_word_from_table
        ld a, (LANE_ROW_TMP)
        ld d, a
        ld e, #0
        call draw_string
        ld a, (LANE_ROW_TMP)
        ld d, a
        ld a, (STATE_ID)
        cp #STATE_ASSIGN
        jr nz, render_message_lane_live_score
        ld hl, #0
        jr render_message_lane_draw_score
render_message_lane_live_score:
        ld l, L_SCORE_L(ix)
        ld h, L_SCORE_H(ix)
render_message_lane_draw_score:
        ld e, #3
        call draw_u16_5
        ld a, (STATE_ID)
        cp #STATE_NAME
        jr z, render_message_lane_name_frame
        call draw_compact_message_lane_frame_new
        jr render_message_lane_dispatch
render_message_lane_name_frame:
        call draw_name_message_lane_frame_new
render_message_lane_dispatch:
        ld a, (STATE_ID)
        cp #STATE_ASSIGN
        jp z, render_lane_assign_message_new
        cp #STATE_COUNTDOWN
        jp z, render_lane_countdown_message_new
        cp #STATE_WINNER
        jp z, render_lane_winner_message_new
        jp render_lane_name_message_new

draw_compact_message_lane_frame_new:
        ld a, (LANE_ROW_TMP)
        inc a
        ld d, a
        ld e, #LANE_LEFT_COL
        ld a, #TILE_WALL_TOP
        ld b, #LANE_SPAN_COLS
        call fill_span
        ld a, (LANE_ROW_TMP)
        add a, #2
        ld d, a
        ld e, #LANE_LEFT_COL
        ld a, #' '
        ld b, #LANE_SPAN_COLS
        call fill_span
        ld a, (LANE_ROW_TMP)
        add a, #3
        ld d, a
        ld e, #LANE_LEFT_COL
        ld a, #TILE_WALL_BOT
        ld b, #LANE_SPAN_COLS
        call fill_span
        ret

draw_name_message_lane_frame_new:
        ld a, (LANE_ROW_TMP)
        inc a
        ld d, a
        ld e, #LANE_LEFT_COL
        ld a, #TILE_WALL_TOP
        ld b, #LANE_SPAN_COLS
        call fill_span
        ld a, (LANE_ROW_TMP)
        add a, #2
        ld d, a
        ld e, #LANE_LEFT_COL
        ld a, #' '
        ld b, #LANE_SPAN_COLS
        call fill_span
        ld a, (LANE_ROW_TMP)
        add a, #3
        ld d, a
        ld e, #LANE_LEFT_COL
        ld a, #' '
        ld b, #LANE_SPAN_COLS
        call fill_span
        ld a, (LANE_ROW_TMP)
        add a, #4
        ld d, a
        ld e, #LANE_LEFT_COL
        ld a, #TILE_WALL_BOT
        ld b, #LANE_SPAN_COLS
        call fill_span
        ret

render_lane_assign_message_new:
        ld a, (LANE_INDEX_TMP)
        ld b, a
        ld a, (ASSIGN_INDEX)
        cp b
        jr z, render_lane_assign_press
        jr c, render_lane_assign_wait
        ld hl, #str_ready
        ld a, (LANE_ROW_TMP)
        add a, #2
        ld d, a
        ld e, #13
        call draw_string
        ret
render_lane_assign_press:
        ld hl, #str_press_a_key
        ld a, (LANE_ROW_TMP)
        add a, #2
        ld d, a
        ld e, #9
        call draw_string
        ret
render_lane_assign_wait:
        ld hl, #str_wait
        ld a, (LANE_ROW_TMP)
        add a, #2
        ld d, a
        ld e, #13
        call draw_string
        ret

render_lane_countdown_message_new:
        ld a, (COUNTDOWN_TICKS)
        cp #45
        jr c, render_lane_countdown_start
        cp #105
        jr c, render_lane_countdown_one
        cp #165
        jr c, render_lane_countdown_two
        ld a, #'3'
        jr render_lane_countdown_digit
render_lane_countdown_two:
        ld a, #'2'
        jr render_lane_countdown_digit
render_lane_countdown_one:
        ld a, #'1'
render_lane_countdown_digit:
        ld c, a
        ld a, (LANE_ROW_TMP)
        add a, #2
        ld d, a
        ld a, c
        ld e, #14
        call draw_char
        ret
render_lane_countdown_start:
        ld hl, #str_start_bang
        ld a, (LANE_ROW_TMP)
        add a, #2
        ld d, a
        ld e, #11
        call draw_string
        ret

render_lane_winner_message_new:
        ld a, (LANE_INDEX_TMP)
        ld b, a
        ld a, (WINNER_INDEX)
        cp b
        jr nz, render_lane_winner_other
        ld hl, #str_winner_round
        ld a, (LANE_ROW_TMP)
        add a, #2
        ld d, a
        ld e, #5
        call draw_string
        ret
render_lane_winner_other:
        ld hl, #str_time_up
        ld a, (LANE_ROW_TMP)
        add a, #2
        ld d, a
        ld e, #11
        call draw_string
        ret

render_lane_name_message_new:
        ld a, (LANE_INDEX_TMP)
        ld b, a
        ld a, (NAME_LANE)
        cp b
        ret nz
        ld a, (NAME_PROMPT_KIND)
        or a
        jr z, render_lane_name_top10
        ld hl, #str_new_best_score
        jr render_lane_name_head
render_lane_name_top10:
        ld hl, #str_top10_score
render_lane_name_head:
        ld a, (LANE_ROW_TMP)
        add a, #2
        ld d, a
        ld e, #8
        call draw_string
        ld hl, #str_name_prefix
        ld a, (LANE_ROW_TMP)
        add a, #3
        ld d, a
        ld e, #8
        call draw_string
        call draw_name_buffer_new
        ret

draw_name_buffer_new:
        ld hl, #TEMP_NAME_BUF
        ld a, (LANE_ROW_TMP)
        add a, #3
        ld d, a
        ld e, #14
        call get_screen_ptr
        ex de, hl
        ld b, #0
draw_name_buffer_loop:
        ld a, b
        cp #HISCORE_NAME_LEN
        ret z
        ld c, a
        ld a, (NAME_POS)
        cp c
        jr z, draw_name_buffer_underscore
        jr c, draw_name_buffer_underscore
        ld a, c
        ld hl, #TEMP_NAME_BUF
        call get_offset_ptr_a_hl
        ld a, (hl)
        jr draw_name_buffer_store
draw_name_buffer_underscore:
        ld a, #'_'
draw_name_buffer_store:
        ld (de), a
        inc de
        inc b
        jr draw_name_buffer_loop

draw_fixed_chars_new:
        push hl
        call get_screen_ptr
        ex de, hl
        pop hl
draw_fixed_chars_loop:
        ld a, b
        or a
        ret z
        ld a, (hl)
        or a
        jr nz, draw_fixed_chars_store
        ld a, #' '
draw_fixed_chars_store:
        ld (de), a
        inc de
        inc hl
        djnz draw_fixed_chars_loop
        ret

draw_time_mmss_new:
        push hl
        call get_screen_ptr
        ex de, hl
        pop hl
        ld bc, #60
        call div_u16_bc_new
        ld a, h
        or a
        jr nz, draw_time_mmss_clamp
        ld a, l
        cp #100
        jr c, draw_time_mmss_minutes_ok
draw_time_mmss_clamp:
        ld l, #99
draw_time_mmss_minutes_ok:
        push de
        call draw_two_digits_to_buffer_new
        pop de
        inc de
        inc de
        ld a, #':'
        ld (de), a
        inc de
        ex de, hl
        ex (sp), hl
        ex de, hl
        ; DE now has remainder from div_u16_bc_new
        ld a, d
        or a
        jr nz, draw_time_mmss_secs_clamp
        ld a, e
        cp #100
        jr c, draw_time_mmss_secs_ok
draw_time_mmss_secs_clamp:
        ld e, #99
draw_time_mmss_secs_ok:
        push hl
        ex de, hl
        call draw_two_digits_to_buffer_new
        pop hl
        ret

draw_two_digits_to_buffer_new:
        push hl
        ld a, l
        ld b, #0
draw_two_digits_tens:
        cp #10
        jr c, draw_two_digits_store
        sub #10
        inc b
        jr draw_two_digits_tens
draw_two_digits_store:
        ld c, a
        pop hl
        ld a, b
        add a, #'0'
        ld (de), a
        inc de
        ld a, c
        add a, #'0'
        ld (de), a
        ret

div_u16_bc_new:
        ld de, #0
div_u16_loop:
        or a
        sbc hl, bc
        jr c, div_u16_done
        inc de
        jr div_u16_loop
div_u16_done:
        add hl, bc
        ret
