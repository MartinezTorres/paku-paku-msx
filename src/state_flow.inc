; Active session/menu/name-entry state flow and high-score bookkeeping.

clear_temp_name_buffer:
        ld hl, #TEMP_NAME_BUF
        ld b, #HISCORE_NAME_LEN
clear_temp_name_loop:
        ld (hl), #' '
        inc hl
        djnz clear_temp_name_loop
        ret

clear_name_context_new:
        ld a, #0xff
        ld (NAME_LANE), a
        ld (NAME_RANK), a
        xor a
        ld (NAME_POS), a
        ld (NAME_PROMPT_KIND), a
        ret

clear_high_scores:
        xor a
        ld (HISCORE_COUNT), a
        ld (HI_SCORE_L), a
        ld (HI_SCORE_H), a
        ld hl, #HISCORE_TABLE
        ld (hl), a
        ld de, #HISCORE_TABLE + 1
        ld bc, #((HISCORE_ENTRY_SIZE * 10) - 1)
        ldir
        ret

load_time_limit_config_from_index:
        ld a, (TIME_LIMIT_IDX)
        ld hl, #time_limit_seconds_lo
        call get_byte_from_table
        ld (TIME_LIMIT_L), a
        ld a, (TIME_LIMIT_IDX)
        ld hl, #time_limit_seconds_hi
        call get_byte_from_table
        ld (TIME_LIMIT_H), a
        ret

clear_attract_scores_timer_new:
        xor a
        ld (ATTRACT_TICKS_L), a
        ld (ATTRACT_TICKS_H), a
        ret

reset_menu_attract_new:
        xor a
        ld (MENU_IDLE_L), a
        ld (MENU_IDLE_H), a
        ld (ATTRACT_PHASE), a
        call clear_attract_scores_timer_new
        ret

advance_attract_scores_timer_new:
        ld a, (ATTRACT_TICKS_L)
        inc a
        ld (ATTRACT_TICKS_L), a
        jr nz, attract_scores_compare
        ld a, (ATTRACT_TICKS_H)
        inc a
        ld (ATTRACT_TICKS_H), a
attract_scores_compare:
        ld a, (ATTRACT_TICKS_H)
        cp #1
        jr c, attract_scores_not_yet
        jr nz, attract_scores_reached
        ld a, (ATTRACT_TICKS_L)
        cp #44
        jr c, attract_scores_not_yet
attract_scores_reached:
        scf
        ret
attract_scores_not_yet:
        or a
        ret

save_menu_config_for_demo_new:
        ld a, (PLAYER_COUNT_V)
        ld (MENU_SAVE_PLAYERS), a
        ld a, (TIME_LIMIT_IDX)
        ld (MENU_SAVE_TIME_IDX), a
        ret

restore_menu_config_if_demo_new:
        ld a, (DEMO_MODE)
        or a
        ret z
        xor a
        ld (DEMO_MODE), a
        ld (DEMO_TICKS_L), a
        ld (DEMO_TICKS_H), a
        ld a, (MENU_SAVE_PLAYERS)
        ld (PLAYER_COUNT_V), a
        ld a, (MENU_SAVE_TIME_IDX)
        ld (TIME_LIMIT_IDX), a
        call load_time_limit_config_from_index
        ret

return_to_menu_common_new:
        xor a
        ld (STATE_ID), a
        ld (MENU_IDLE_L), a
        ld (MENU_IDLE_H), a
        ld (NAME_RELEASE_WAIT), a
        call clear_name_context_new
        call clear_attract_scores_timer_new
        call restore_menu_config_if_demo_new
        call update_hi_score_from_table_new
        ret

return_to_menu_auto_new:
        call return_to_menu_common_new
        call clear_input_buffers
        ret

start_attract_scores_new:
        ld a, #STATE_SCORES
        ld (STATE_ID), a
        call clear_attract_scores_timer_new
        call clear_input_buffers
        ret

start_attract_demo_new:
        call save_menu_config_for_demo_new
        ld a, #2
        ld (PLAYER_COUNT_V), a
        xor a
        ld (TIME_LIMIT_L), a
        ld (TIME_LIMIT_H), a
        ld (SERIES_TYPE_V), a
        ld (ATTRACT_PHASE), a
        ld a, #1
        ld (DEMO_MODE), a
        ld a, #208
        ld (DEMO_TICKS_L), a
        ld a, #2
        ld (DEMO_TICKS_H), a
        call clear_attract_scores_timer_new
        call begin_countdown_new
        ret

advance_demo_timer_new:
        ld hl, (DEMO_TICKS_L)
        ld a, h
        or l
        jr z, advance_demo_timer_done
        dec hl
        ld (DEMO_TICKS_L), hl
        ld a, h
        or l
        jr nz, advance_demo_timer_active
advance_demo_timer_done:
        scf
        ret
advance_demo_timer_active:
        or a
        ret

; Main state dispatcher for menu, gameplay, and postgame flows.
update_state_new:
        ld a, (STATE_ID)
        cp #STATE_MENU
        jp z, update_menu_new
        cp #STATE_ASSIGN
        jp z, update_assign_new
        cp #STATE_COUNTDOWN
        jp z, update_countdown_new
        cp #STATE_PLAY
        jp z, update_play_new
        cp #STATE_WINNER
        jp z, update_winner_new
        cp #STATE_NAME
        jp z, update_name_new
        jp update_scores_new

; Menu navigation is edge-triggered. A separate idle counter pushes the menu to
; the best-scores attract screen after roughly five seconds of inactivity.
update_menu_new:
        ld a, (MENU_RELEASE_WAIT)
        or a
        jr z, update_menu_idle_gate
        ld a, (ANY_KEY_CURR)
        ld b, a
        ld a, (JOY_CURR)
        or b
        ret nz
        call clear_input_buffers
        ret
update_menu_idle_gate:
        ld a, (ANY_KEY_EDGE)
        or a
        jr nz, update_menu_reset_idle
        ld a, (JOY_EDGE)
        or a
        jr nz, update_menu_reset_idle
        call menu_advance_idle_new
        jr update_menu_actions
update_menu_reset_idle:
        call reset_menu_attract_new
update_menu_actions:
        ld a, (MENU_ACTION_EDGE)
        and #M_ACT_UP
        jr z, update_menu_no_up
        ld a, (MENU_INDEX)
        or a
        jr nz, update_menu_up_dec
        ld a, #MENU_ITEM_COUNT - 1
        jr update_menu_up_store
update_menu_up_dec:
        dec a
update_menu_up_store:
        ld (MENU_INDEX), a
        call play_ui_move_sfx_new
update_menu_no_up:
        ld a, (MENU_ACTION_EDGE)
        and #M_ACT_DOWN
        jr z, update_menu_no_down
        ld a, (MENU_INDEX)
        inc a
        cp #MENU_ITEM_COUNT
        jr c, update_menu_down_store
        xor a
update_menu_down_store:
        ld (MENU_INDEX), a
        call play_ui_move_sfx_new
update_menu_no_down:
        ld a, (MENU_INDEX)
        or a
        jr z, update_menu_start_item
        dec a
        jr z, update_menu_players_item
        jr update_menu_time_item

update_menu_start_item:
        ld a, (MENU_ACTION_EDGE)
        and #M_ACT_SELECT
        ret z
        call start_selected_game_new
        ret

update_menu_players_item:
        ld a, (MENU_ACTION_EDGE)
        and #M_ACT_LEFT
        jr nz, update_menu_players_left
        ld a, (MENU_ACTION_EDGE)
        and #(M_ACT_RIGHT | M_ACT_SELECT)
        ret z
        ld a, (PLAYER_COUNT_V)
        inc a
        cp #5
        jr c, update_menu_players_store
        ld a, #1
update_menu_players_store:
        ld (PLAYER_COUNT_V), a
        call play_ui_move_sfx_new
        ret
update_menu_players_left:
        ld a, (PLAYER_COUNT_V)
        dec a
        jr nz, update_menu_players_store
        ld a, #4
        jr update_menu_players_store

update_menu_time_item:
        ld a, (MENU_ACTION_EDGE)
        and #M_ACT_LEFT
        jr nz, update_menu_time_left
        ld a, (MENU_ACTION_EDGE)
        and #(M_ACT_RIGHT | M_ACT_SELECT)
        ret z
        ld a, (TIME_LIMIT_IDX)
        inc a
        cp #4
        jr c, update_menu_time_store
        xor a
update_menu_time_store:
        ld (TIME_LIMIT_IDX), a
        call load_time_limit_config_from_index
        call play_ui_move_sfx_new
        ret
update_menu_time_left:
        ld a, (TIME_LIMIT_IDX)
        or a
        jr nz, update_menu_time_left_dec
        ld a, #3
        jr update_menu_time_store
update_menu_time_left_dec:
        dec a
        jr update_menu_time_store

; Count idle frames on the menu. The attract cycle alternates between the score
; list and a short self-running two-player demo.
menu_advance_idle_new:
        ld a, (MENU_IDLE_L)
        inc a
        ld (MENU_IDLE_L), a
        jr nz, menu_idle_compare
        ld a, (MENU_IDLE_H)
        inc a
        ld (MENU_IDLE_H), a
menu_idle_compare:
        ld a, (MENU_IDLE_H)
        cp #1
        ret c
        jr nz, menu_idle_trigger
        ld a, (MENU_IDLE_L)
        cp #44
        ret c
menu_idle_trigger:
        ld a, (ATTRACT_PHASE)
        or a
        jr nz, menu_idle_start_demo
        ld a, #ATTRACT_TO_DEMO
        ld (ATTRACT_PHASE), a
        call start_attract_scores_new
        ret
menu_idle_start_demo:
        call start_attract_demo_new
        ret

; Single-player starts immediately. Multiplayer first enters key-assignment so
; each lane can claim one keyboard binding.
start_selected_game_new:
        xor a
        ld (DEMO_MODE), a
        ld (DEMO_TICKS_L), a
        ld (DEMO_TICKS_H), a
        ld (POSTGAME_SCAN), a
        ld (WINNER_INDEX), a
        ld (WINNER_TICKS), a
        ld a, (PLAYER_COUNT_V)
        cp #1
        jr z, start_selected_game_single
        xor a
        ld (ASSIGN_INDEX), a
        ld a, #STATE_ASSIGN
        ld (STATE_ID), a
        call clear_input_edges_keep_hold_new
        ld a, #1
        ld (ASSIGN_RELEASE_WAIT), a
        ret
start_selected_game_single:
        call begin_countdown_new
        ret

; Reset the match and enter the shared 3-2-1-START countdown state.
begin_countdown_new:
        call prepare_match_new
        call clear_input_buffers
        ld a, #225
        ld (COUNTDOWN_TICKS), a
        ld a, #STATE_COUNTDOWN
        ld (STATE_ID), a
        call play_countdown_sfx_new
        ret

; Reinitialize timing, difficulty, timer, and all lane-local gameplay state for
; a fresh round.
prepare_match_new:
        call KILBUF
        xor a
        ld (WINNER_INDEX), a
        ld (WINNER_TICKS), a
        ld (POSTGAME_SCAN), a
        ld (NAME_POS), a
        ld (QUIT_CONFIRM_TICKS), a
        ld (NAME_RELEASE_WAIT), a
        ld (HI_SCORE_L), a
        ld (HI_SCORE_H), a
        call reset_difficulty_progress_new
        call reseed_match_new
        ld a, (TIME_LIMIT_L)
        ld (TIMER_REM_L), a
        ld a, (TIME_LIMIT_H)
        ld (TIMER_REM_H), a
        ld b, a
        ld a, (TIMER_REM_L)
        or b
        jr z, prepare_match_no_timer
        ld a, #60
        ld (TIMER_FRAME), a
        jr prepare_match_timer_done
prepare_match_no_timer:
        xor a
        ld (TIMER_FRAME), a
prepare_match_timer_done:
        call init_all_lanes
        ret

; Mix time and current menu selections into the small PRNG seed so consecutive
; rounds diverge even with identical player actions.
reseed_match_new:
        ld a, (RAND_STATE)
        ld b, a
        ld a, (JIFFY)
        xor b
        ld b, a
        ld a, (PLAYER_COUNT_V)
        xor b
        ld b, a
        ld a, (TIME_LIMIT_IDX)
        xor b
        or #1
        ld (RAND_STATE), a
        ret

; Capture one key per player lane. The release gate prevents a held key from
; being assigned to several players in a row.
update_assign_new:
        ld a, (ASSIGN_RELEASE_WAIT)
        or a
        jr z, update_assign_read_key
        ld a, (ANY_KEY_CURR)
        ld b, a
        ld a, (JOY_CURR)
        or b
        ret nz
        call clear_input_buffers
        xor a
        ld (ASSIGN_RELEASE_WAIT), a
        ret
update_assign_read_key:
        ld a, (PENDING_KEY_CODE)
        cp #0xff
        ret z
        cp #58
        jr z, update_assign_wait_release
        ld a, (ASSIGN_INDEX)
        ld hl, #BIND_ROWS_BASE
        call get_offset_ptr_a_hl
        ld a, (PENDING_KEY_ROW)
        ld (hl), a
        ld a, (ASSIGN_INDEX)
        ld hl, #BIND_MASKS_BASE
        call get_offset_ptr_a_hl
        ld a, (PENDING_KEY_MASK)
        ld (hl), a
        ld a, (ASSIGN_INDEX)
        inc a
        ld (ASSIGN_INDEX), a
        call play_ui_move_sfx_new
        ld a, (PLAYER_COUNT_V)
        ld b, a
        ld a, (ASSIGN_INDEX)
        cp b
        jr z, update_assign_begin_countdown
update_assign_wait_release:
        call clear_input_edges_keep_hold_new
        ld a, #1
        ld (ASSIGN_RELEASE_WAIT), a
        ret
update_assign_begin_countdown:
        call begin_countdown_new
        ret

; The countdown duration lives in simulation ticks. Once it expires, the round
; enters active play and inputs are cleared before the first live frame.
update_countdown_new:
        ld a, (DEMO_MODE)
        or a
        jr z, update_countdown_tick
        ld a, (ANY_KEY_EDGE)
        or a
        jp nz, return_to_menu_new
        ld a, (JOY_EDGE)
        or a
        jp nz, return_to_menu_new
update_countdown_tick:
        ld a, (COUNTDOWN_TICKS)
        or a
        jr z, update_countdown_start
        dec a
        ld (COUNTDOWN_TICKS), a
        cp #164
        jr z, update_countdown_digit_sfx
        cp #104
        jr z, update_countdown_digit_sfx
        cp #44
        jr z, update_countdown_start_sfx
        ret
update_countdown_digit_sfx:
        call play_countdown_sfx_new
        ret
update_countdown_start_sfx:
        call play_start_sfx_new
        ret
update_countdown_start:
        ld a, #STATE_PLAY
        ld (STATE_ID), a
        call clear_input_buffers
        ret

; ESC uses a two-step confirmation. All other live gameplay work happens in the
; per-lane simulation, difficulty ramp, timer, and high-score helpers.
update_play_new:
        ld a, (DEMO_MODE)
        or a
        jr z, update_play_check_esc
        ld a, (ANY_KEY_EDGE)
        or a
        jp nz, return_to_menu_new
        ld a, (JOY_EDGE)
        or a
        jp nz, return_to_menu_new
        jr update_play_run
update_play_check_esc:
        ld a, (MENU_ACTION_EDGE)
        and #M_ACT_ESC
        jr z, update_play_no_esc
        ld a, (QUIT_CONFIRM_TICKS)
        or a
        jr z, update_play_arm_quit
        jp finish_match_to_names_new
update_play_arm_quit:
        ld a, #120
        ld (QUIT_CONFIRM_TICKS), a
        ret
update_play_no_esc:
        ld a, (QUIT_CONFIRM_TICKS)
        or a
        jr z, update_play_run
        dec a
        ld (QUIT_CONFIRM_TICKS), a
update_play_run:
        ld a, (DEMO_MODE)
        or a
        call nz, build_demo_inputs_new
        call update_all_lanes
        call advance_difficulty
        call advance_timer_new
        call update_live_hi_score_new
        ld a, (DEMO_MODE)
        or a
        jr z, update_play_check_time_limit
        call advance_demo_timer_new
        jp c, return_to_menu_auto_new
update_play_check_time_limit:
        ld a, (TIME_LIMIT_L)
        ld b, a
        ld a, (TIME_LIMIT_H)
        or b
        ret z
        ld a, (TIMER_REM_L)
        ld b, a
        ld a, (TIMER_REM_H)
        or b
        ret nz
        jp finish_timed_match_new

; Timed modes count down in whole seconds from the virtual 60 Hz simulation.
advance_timer_new:
        ld a, (TIME_LIMIT_L)
        ld b, a
        ld a, (TIME_LIMIT_H)
        or b
        ret z
        ld a, (TIMER_REM_L)
        ld b, a
        ld a, (TIMER_REM_H)
        or b
        ret z
        ld a, (TIMER_FRAME)
        dec a
        jr nz, advance_timer_store_frame
        ld a, #60
        ld (TIMER_FRAME), a
        ld hl, (TIMER_REM_L)
        dec hl
        ld a, l
        ld (TIMER_REM_L), a
        ld a, h
        ld (TIMER_REM_H), a
        ret
advance_timer_store_frame:
        ld (TIMER_FRAME), a
        ret

; Freeze the round, compute the winner once, and show the winner banner before
; name-entry processing begins.
finish_timed_match_new:
        call build_rankings
        ld a, (RANKS_BASE)
        ld (WINNER_INDEX), a
        ld a, #180
        ld (WINNER_TICKS), a
        xor a
        ld (POSTGAME_SCAN), a
        ld a, #STATE_WINNER
        ld (STATE_ID), a
        call clear_input_buffers
        call play_winner_sfx_new
        ret

; Endless mode skips the winner banner and jumps straight into the postgame
; high-score/name-entry scan.
finish_match_to_names_new:
        xor a
        ld (POSTGAME_SCAN), a
        jp begin_name_sequence_new

; Any key can dismiss the winner banner early; otherwise it times out into the
; name-entry phase automatically.
update_winner_new:
        ld a, (ANY_KEY_EDGE)
        or a
        jr nz, update_winner_done
        ld a, (JOY_EDGE)
        or a
        jr nz, update_winner_done
        ld a, (WINNER_TICKS)
        or a
        jr z, update_winner_done
        dec a
        ld (WINNER_TICKS), a
        ret
update_winner_done:
        jp begin_name_sequence_new

; Walk the player lanes in order and stop on each score that qualifies for the
; high-score table.
begin_name_sequence_new:
        call find_next_name_entry_new
        ret c
        jp return_to_menu_new

; Search for the next lane that earned a high-score entry opportunity.
find_next_name_entry_new:
        ld a, (POSTGAME_SCAN)
        ld c, a
find_next_name_loop:
        ld a, (PLAYER_COUNT_V)
        cp c
        jr z, find_next_name_none
        jr c, find_next_name_none
        ld a, c
        push bc
        call set_ix_to_lane_a
        ld l, L_BEST_L(ix)
        ld h, L_BEST_H(ix)
        call find_hiscore_rank_hl_new
        pop bc
        cp #0xff
        jr z, find_next_name_skip
        ld (NAME_RANK), a
        ld a, c
        ld (NAME_LANE), a
        xor a
        ld (NAME_POS), a
        call clear_temp_name_buffer
        ld a, (NAME_RANK)
        or a
        jr z, find_next_name_best
        xor a
        ld (NAME_PROMPT_KIND), a
        jr find_next_name_store_state
find_next_name_best:
        ld a, #1
        ld (NAME_PROMPT_KIND), a
find_next_name_store_state:
        ld a, #STATE_NAME
        ld (STATE_ID), a
        call clear_input_edges_keep_hold_new
        ld a, #1
        ld (NAME_RELEASE_WAIT), a
        scf
        ret
find_next_name_skip:
        inc c
        jr find_next_name_loop
find_next_name_none:
        call clear_name_context_new
        or a
        ret

; Return to the menu and force an input release before accepting the next key.
return_to_menu_new:
        call return_to_menu_common_new
        xor a
        ld (ATTRACT_PHASE), a
        call clear_input_edges_keep_hold_new
        ld a, #1
        ld (MENU_RELEASE_WAIT), a
        ret

; The best-scores attract screen exits on any fresh keyboard or joystick input.
update_scores_new:
        ld a, (ANY_KEY_EDGE)
        or a
        jr nz, update_scores_exit
        ld a, (JOY_EDGE)
        or a
        jr nz, update_scores_exit
        call advance_attract_scores_timer_new
        ret nc
        jp return_to_menu_auto_new
update_scores_exit:
        jp return_to_menu_new

; Name entry consumes one translated pending key at a time on the lane that
; earned the score.
update_name_new:
        ld a, (NAME_RANK)
        cp #0xff
        jp z, return_to_menu_new
        ld a, (NAME_LANE)
        cp #0xff
        jp z, return_to_menu_new
        ld b, a
        ld a, (PLAYER_COUNT_V)
        cp b
        jp z, return_to_menu_new
        jp c, return_to_menu_new
        ld a, (NAME_RELEASE_WAIT)
        or a
        jr z, update_name_read
        ld a, (ANY_KEY_CURR)
        ld b, a
        ld a, (JOY_CURR)
        or b
        ret nz
        call clear_input_buffers
        xor a
        ld (NAME_RELEASE_WAIT), a
        ret
update_name_read:
        ld a, (PENDING_KEY_CODE)
        cp #0xff
        ret z
        call translate_pending_key_new
        or a
        ret z
        cp #13
        jp z, finalize_name_entry_new
        cp #8
        jr z, update_name_backspace
        ld b, a
        ld a, (NAME_POS)
        cp #HISCORE_NAME_LEN
        ret nc
        ld hl, #TEMP_NAME_BUF
        call get_offset_ptr_a_hl
        ld a, b
        ld (hl), a
        ld a, (NAME_POS)
        inc a
        ld (NAME_POS), a
        ret
update_name_backspace:
        ld a, (NAME_POS)
        or a
        ret z
        dec a
        ld (NAME_POS), a
        ld hl, #TEMP_NAME_BUF
        call get_offset_ptr_a_hl
        ld (hl), #' '
        ret

; Insert the current lane score into the table, then continue scanning later
; lanes for more qualifying entries.
finalize_name_entry_new:
        call insert_current_score_new
        ld a, (NAME_LANE)
        inc a
        ld (POSTGAME_SCAN), a
        jp begin_name_sequence_new

; Keep the session header high score live while the round is still running.
update_live_hi_score_new:
        ld ix, #LANES_BASE
        ld a, (PLAYER_COUNT_V)
        ld b, a
update_live_hi_loop:
        ld e, L_BEST_L(ix)
        ld d, L_BEST_H(ix)
        ld a, (HI_SCORE_H)
        cp d
        jr c, update_live_hi_store
        jr nz, update_live_hi_next
        ld a, (HI_SCORE_L)
        cp e
        jr c, update_live_hi_store
        jr z, update_live_hi_next
update_live_hi_store:
        ld a, e
        ld (HI_SCORE_L), a
        ld a, d
        ld (HI_SCORE_H), a
update_live_hi_next:
        push bc
        ld bc, #LANE_SIZE
        add ix, bc
        pop bc
        djnz update_live_hi_loop
        ret

; Find the insertion rank for HL. Return 0xff when the score does not reach the
; top-10 list.
find_hiscore_rank_hl_new:
        ld a, h
        or l
        jr nz, find_hiscore_rank_nonzero
        ld a, #0xff
        ret
find_hiscore_rank_nonzero:
        ld a, l
        ld (HISCORE_CANDIDATE_L), a
        ld a, h
        ld (HISCORE_CANDIDATE_H), a
        ld a, (HISCORE_COUNT)
        ld b, a
        ld c, #0
find_hiscore_rank_loop:
        ld a, c
        cp b
        jr z, find_hiscore_rank_append
        push bc
        ld a, c
        call get_hiscore_ptr_from_index_new
        ld e, (hl)
        inc hl
        ld d, (hl)
        pop bc
        ld a, (HISCORE_CANDIDATE_H)
        cp d
        jr c, find_hiscore_rank_next
        jr nz, find_hiscore_rank_found
        ld a, (HISCORE_CANDIDATE_L)
        cp e
        jr c, find_hiscore_rank_next
        jr z, find_hiscore_rank_next
find_hiscore_rank_found:
        ld a, c
        ret
find_hiscore_rank_next:
        inc c
        jr find_hiscore_rank_loop
find_hiscore_rank_append:
        ld a, b
        cp #10
        jr c, find_hiscore_rank_append_ok
        ld a, #0xff
        ret
find_hiscore_rank_append_ok:
        ld a, b
        ret

; Insert the newly named score by shifting lower entries down in place.
insert_current_score_new:
        ld a, (HISCORE_COUNT)
        ld b, a
        cp #10
        jr nc, insert_current_full
        inc a
        ld (HISCORE_COUNT), a
        jr insert_current_shift_prep
insert_current_full:
        ld a, #8
        jr insert_current_shift_start
insert_current_shift_prep:
        ld a, b
        or a
        jr z, insert_current_write
        dec a
insert_current_shift_start:
        ld c, a
insert_current_shift_loop:
        ld a, c
        ld d, a
        ld a, (NAME_RANK)
        cp d
        jr c, insert_current_do_copy
        jr z, insert_current_do_copy
        jr insert_current_write
insert_current_do_copy:
        push bc
        ld a, c
        call get_hiscore_ptr_from_index_new
        push hl
        ld a, c
        inc a
        call get_hiscore_ptr_from_index_new
        ex de, hl
        pop hl
        ld bc, #HISCORE_ENTRY_SIZE
        ldir
        pop bc
        ld a, c
        or a
        jr z, insert_current_write
        dec c
        jr insert_current_shift_loop
insert_current_write:
        ld a, (NAME_RANK)
        call get_hiscore_ptr_from_index_new
        push hl
        ld a, (NAME_LANE)
        call set_ix_to_lane_a
        pop hl
        ld a, L_BEST_L(ix)
        ld (hl), a
        inc hl
        ld a, L_BEST_H(ix)
        ld (hl), a
        inc hl
        ld de, #TEMP_NAME_BUF
        ld b, #HISCORE_NAME_LEN
insert_current_name_copy:
        ld a, (de)
        ld (hl), a
        inc de
        inc hl
        djnz insert_current_name_copy
        call update_hi_score_from_table_new
        ret

update_hi_score_from_table_new:
        ld a, (HISCORE_COUNT)
        or a
        jr nz, update_hi_score_from_table_have
        xor a
        ld (HI_SCORE_L), a
        ld (HI_SCORE_H), a
        ret
update_hi_score_from_table_have:
        ld hl, #HISCORE_TABLE
        ld a, (hl)
        ld (HI_SCORE_L), a
        inc hl
        ld a, (hl)
        ld (HI_SCORE_H), a
        ret

get_hiscore_ptr_from_index_new:
        ld hl, #HISCORE_TABLE
        or a
        ret z
        ld b, a
get_hiscore_ptr_loop:
        ld de, #HISCORE_ENTRY_SIZE
        add hl, de
        djnz get_hiscore_ptr_loop
        ret

get_offset_ptr_a_hl:
        ld e, a
        ld d, #0
        add hl, de
        ret
