; Core gameplay simulation: lane init, movement, collisions, and difficulty ramp.

init_all_lanes:
        ld ix, #LANES_BASE
        ld a, (PLAYER_COUNT_V)
        ld (ACTIVE_LANE_COUNT), a
        ld b, #4
init_lanes_loop:
        ld a, (ACTIVE_LANE_COUNT)
        or a
        jr z, init_lane_inactive
        dec a
        ld (ACTIVE_LANE_COUNT), a
        call init_active_lane
        jr init_lane_next
init_lane_inactive:
        xor a
        ld L_STATUS(ix), a
        ld L_SCORE_L(ix), a
        ld L_SCORE_H(ix), a
        ld L_MULT(ix), a
        ld L_DOTS_L(ix), a
        ld L_DOTS_H(ix), a
init_lane_next:
        push bc
        ld bc, #LANE_SIZE
        add ix, bc
        pop bc
        djnz init_lanes_loop
        ret

init_active_lane:
        ld hl, #PLAYER_START
        ld L_PX_L(ix), l
        ld L_PX_H(ix), h
        ld a, #1
        ld L_VX(ix), a
        ld hl, #ENEMY_START
        ld L_EX_L(ix), l
        ld L_EX_H(ix), h
        xor a
        ld L_EYE(ix), a
        ld L_PWR_L(ix), a
        ld L_PWR_H(ix), a
        ld L_ANM_L(ix), a
        ld L_ANM_H(ix), a
        ld L_MULT(ix), a
        ld L_SCORE_L(ix), a
        ld L_SCORE_H(ix), a
        ld a, #ST_PART | ST_ALIVE
        ld L_STATUS(ix), a
        call add_dots
        ret

update_all_lanes:
        ld ix, #LANES_BASE
        ld a, (PLAYER_COUNT_V)
        ld b, a
        ld e, #1
update_lane_loop:
        push bc
        push de
        call update_one_lane
        pop de
        sla e
        pop bc
        push bc
        ld bc, #LANE_SIZE
        add ix, bc
        pop bc
        djnz update_lane_loop
        ret

update_one_lane:
        ld a, L_STATUS(ix)
        and #ST_ALIVE
        ret z

        ld a, (INPUT_EDGE)
        and e
        jr z, no_reverse
        ld a, L_VX(ix)
        xor #0xfe
        ld L_VX(ix), a
no_reverse:
        call add_anim_ticks
        call move_player
        call check_dot_hits
        call move_enemy
        call check_enemy_hit
        call update_power_timer
        call refill_dots_if_empty
        ret

add_anim_ticks:
        ld a, (DIFF_FP_L)
        ld c, a
        ld a, L_ANM_L(ix)
        add a, c
        ld L_ANM_L(ix), a
        ld a, (DIFF_FP_H)
        ld c, a
        ld a, L_ANM_H(ix)
        adc a, c
        ld L_ANM_H(ix), a
        ret

move_player:
        ld a, (PLAYER_DELTA_L)
        ld e, a
        ld a, (PLAYER_DELTA_H)
        ld d, a
        ld l, L_PX_L(ix)
        ld h, L_PX_H(ix)
        ld a, L_VX(ix)
        cp #1
        jr z, move_player_right
        or a
        sbc hl, de
        bit 7, h
        jr z, move_player_store
        ld a, h
        cp #0xfd
        jr nc, move_player_store
        ld hl, #PLAYER_WRAP_MAX
        jr move_player_store
move_player_right:
        add hl, de
        bit 7, h
        jr nz, move_player_store
        ld a, h
        cp #0x67
        jr c, move_player_store
        jr nz, move_player_wrap
        ld a, l
        or a
        jr z, move_player_store
move_player_wrap:
        ld hl, #PLAYER_WRAP_MIN
move_player_store:
        ld L_PX_L(ix), l
        ld L_PX_H(ix), h
        ret

check_dot_hits:
        ld b, #0
dot_hit_loop:
        ld a, b
        cp #16
        ret z
        push bc
        call dot_is_active
        pop bc
        jr z, dot_hit_next
        ld a, b
        ld hl, #dot_positions
        call get_byte_from_table
        ld c, a
        ld a, L_PX_H(ix)
        call absdiff_a_c
        cp #3
        jr nc, dot_hit_next
        push bc
        call clear_dot_bit
        pop bc
        ld a, b
        cp L_PWR_IDX(ix)
        jr nz, dot_normal
        ld a, L_EYE(ix)
        or a
        jr nz, dot_score_only
        ld hl, #POWER_INIT
        ld L_PWR_L(ix), l
        ld L_PWR_H(ix), h
        jr dot_score_only
dot_normal:
        nop
dot_score_only:
        call add_multiplier_score
dot_hit_next:
        inc b
        jr dot_hit_loop

move_enemy:
        ld a, L_EYE(ix)
        or a
        jr z, enemy_not_eyes
        ld (ENEMY_MOVE_DIR), a
        ld a, (EYE_DELTA_L)
        ld e, a
        ld a, (EYE_DELTA_H)
        ld d, a
        jr enemy_apply_move
enemy_not_eyes:
        ld a, L_PX_H(ix)
        bit 7, a
        jr nz, enemy_dir_left
        ld c, L_EX_H(ix)
        cp c
        jr c, enemy_dir_left
        jr z, enemy_dir_left
        ld a, #1
        jr enemy_dir_ready
enemy_dir_left:
        ld a, #0xff
enemy_dir_ready:
        ld c, a
        ld a, L_PWR_L(ix)
        or L_PWR_H(ix)
        jr z, enemy_chase
        ld a, c
        xor #0xfe
        ld c, a
        ld a, (FRIGHT_DELTA_L)
        ld e, a
        ld a, (FRIGHT_DELTA_H)
        ld d, a
        jr enemy_store_dir
enemy_chase:
        ld a, (CHASE_DELTA_L)
        ld e, a
        ld a, (CHASE_DELTA_H)
        ld d, a
enemy_store_dir:
        ld a, c
        ld (ENEMY_MOVE_DIR), a
enemy_apply_move:
        ld l, L_EX_L(ix)
        ld h, L_EX_H(ix)
        ld a, (ENEMY_MOVE_DIR)
        cp #1
        jr z, enemy_move_right
        or a
        sbc hl, de
        jr nc, enemy_clamp_low
        ld hl, #0x0000
        jr enemy_store_pos
enemy_move_right:
        add hl, de
        ld a, h
        cp #0x64
        jr c, enemy_store_pos
        jr nz, enemy_clamp_high
        ld a, l
        or a
        jr z, enemy_store_pos
enemy_clamp_high:
        ld hl, #TRACK_MAX
        jr enemy_store_pos
enemy_clamp_low:
        ld a, h
        or l
        jr nz, enemy_store_pos
enemy_store_pos:
        ld L_EX_L(ix), l
        ld L_EX_H(ix), h
        ld a, L_EYE(ix)
        or a
        ret z
        cp #1
        jr z, enemy_eye_right_check
        ld a, L_EX_H(ix)
        cp #1
        ret nc
        xor a
        ld L_EYE(ix), a
        ret
enemy_eye_right_check:
        ld a, L_EX_H(ix)
        cp #99
        ret c
        xor a
        ld L_EYE(ix), a
        ret

check_enemy_hit:
        ld a, L_EYE(ix)
        or a
        ret nz
        ld a, L_PX_H(ix)
        ld c, L_EX_H(ix)
        call absdiff_player_enemy
        cp #4
        ret nc
        ld a, L_PWR_L(ix)
        or L_PWR_H(ix)
        jr z, enemy_kills_player
        ld a, L_PX_H(ix)
        bit 7, a
        jr nz, enemy_send_right
        cp #51
        jr c, enemy_send_right
        ld a, #0xff
        jr enemy_store_eye
enemy_send_right:
        ld a, #1
enemy_store_eye:
        ld L_EYE(ix), a
        xor a
        ld L_PWR_L(ix), a
        ld L_PWR_H(ix), a
        ld a, L_MULT(ix)
        inc a
        ld L_MULT(ix), a
        ld a, L_MULT(ix)
        ld c, #10
        call mul8x8
        ex de, hl
        call add_lane_score_de
        ret
enemy_kills_player:
        call init_active_lane
        ret

update_power_timer:
        ld a, L_PWR_L(ix)
        or L_PWR_H(ix)
        ret z
        ld l, L_PWR_L(ix)
        ld h, L_PWR_H(ix)
        ld a, (DIFF_FP_L)
        ld e, a
        ld a, (DIFF_FP_H)
        ld d, a
        or a
        sbc hl, de
        jr nc, power_store
        ld hl, #0x0000
power_store:
        ld L_PWR_L(ix), l
        ld L_PWR_H(ix), h
        ret

refill_dots_if_empty:
        ld a, L_DOTS_L(ix)
        or L_DOTS_H(ix)
        ret nz
        call add_dots
        ret

advance_difficulty:
        ld a, (GLOBAL_TICKS_L)
        inc a
        ld (GLOBAL_TICKS_L), a
        jr nz, tick_done
        ld a, (GLOBAL_TICKS_H)
        inc a
        ld (GLOBAL_TICKS_H), a
tick_done:
        ld a, (DIFF_PHASE_L)
        add a, #16
        ld (DIFF_PHASE_L), a
        cp #225
        jr c, diff_done
        sub #225
        ld (DIFF_PHASE_L), a
        ld a, (DIFF_FP_L)
        inc a
        ld (DIFF_FP_L), a
        jr nz, diff_recalc
        ld a, (DIFF_FP_H)
        inc a
        ld (DIFF_FP_H), a
diff_recalc:
        call recalc_deltas
diff_done:
        ret

recalc_deltas:
        ld a, (DIFF_FP_H)
        ld c, #128
        call mul8x8
        ld a, l
        ld (PLAYER_DELTA_L), a
        ld a, h
        ld (PLAYER_DELTA_H), a
        ld a, (DIFF_FP_L)
        ld c, #128
        call mul8x8
        ld a, h
        ld e, a
        ld a, (PLAYER_DELTA_L)
        add a, e
        ld (PLAYER_DELTA_L), a
        ld a, (PLAYER_DELTA_H)
        adc a, #0
        ld (PLAYER_DELTA_H), a

        ld a, (DIFF_FP_H)
        ld c, #141
        call mul8x8
        ld a, l
        ld (CHASE_DELTA_L), a
        ld a, h
        ld (CHASE_DELTA_H), a
        ld a, (DIFF_FP_L)
        ld c, #141
        call mul8x8
        ld a, h
        ld e, a
        ld a, (CHASE_DELTA_L)
        add a, e
        ld (CHASE_DELTA_L), a
        ld a, (CHASE_DELTA_H)
        adc a, #0
        ld (CHASE_DELTA_H), a

        ld a, (DIFF_FP_H)
        ld c, #64
        call mul8x8
        ld a, l
        ld (FRIGHT_DELTA_L), a
        ld a, h
        ld (FRIGHT_DELTA_H), a
        ld a, (DIFF_FP_L)
        ld c, #64
        call mul8x8
        ld a, h
        ld e, a
        ld a, (FRIGHT_DELTA_L)
        add a, e
        ld (FRIGHT_DELTA_L), a
        ld a, (FRIGHT_DELTA_H)
        adc a, #0
        ld (FRIGHT_DELTA_H), a

        ld a, (DIFF_FP_H)
        ld c, #192
        call mul8x8
        ld a, l
        ld (EYE_DELTA_L), a
        ld a, h
        ld (EYE_DELTA_H), a
        ld a, (DIFF_FP_L)
        ld c, #192
        call mul8x8
        ld a, h
        ld e, a
        ld a, (EYE_DELTA_L)
        add a, e
        ld (EYE_DELTA_L), a
        ld a, (EYE_DELTA_H)
        adc a, #0
        ld (EYE_DELTA_H), a
        ret

add_dots:
        ld a, #0xff
        ld L_DOTS_L(ix), a
        ld L_DOTS_H(ix), a
        ld a, L_PX_H(ix)
        bit 7, a
        jr nz, add_dots_left
        cp #51
        jr c, add_dots_left
        call random_mod5
        add a, #1
        jr add_dots_store
add_dots_left:
        call random_mod5
        add a, #10
add_dots_store:
        ld L_PWR_IDX(ix), a
        ld a, L_MULT(ix)
        inc a
        ld L_MULT(ix), a
        ret

add_multiplier_score:
        ld e, L_MULT(ix)
        ld d, #0
        jp add_lane_score_de

add_lane_score_de:
        ld a, L_SCORE_L(ix)
        add a, e
        ld L_SCORE_L(ix), a
        ld a, L_SCORE_H(ix)
        adc a, d
        jr nc, add_lane_score_store
        ld a, #0xff
        ld L_SCORE_L(ix), a
        ld L_SCORE_H(ix), a
        ret
add_lane_score_store:
        ld L_SCORE_H(ix), a
        ret

dot_is_active:
        ld hl, #dot_masks_lo
        ld a, b
        call get_byte_from_table
        ld c, a
        ld a, L_DOTS_L(ix)
        and c
        jr nz, dot_active_yes
        ld hl, #dot_masks_hi
        ld a, b
        call get_byte_from_table
        ld c, a
        ld a, L_DOTS_H(ix)
        and c
dot_active_yes:
        ret

clear_dot_bit:
        ld hl, #dot_masks_lo
        ld a, b
        call get_byte_from_table
        cpl
        ld c, a
        ld a, L_DOTS_L(ix)
        and c
        ld L_DOTS_L(ix), a
        ld hl, #dot_masks_hi
        ld a, b
        call get_byte_from_table
        cpl
        ld c, a
        ld a, L_DOTS_H(ix)
        and c
        ld L_DOTS_H(ix), a
        ret
