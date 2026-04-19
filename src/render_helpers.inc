; Rendering helpers, VRAM transfer, sprite assembly, and shared math.

; Start each redraw from a space-filled RAM name table.
clear_screen_buffer:
        ld hl, #SCREEN_BUFFER
        ld (hl), #0x20
        ld de, #SCREEN_BUFFER + 1
        ld bc, #((SCREEN_WIDTH * SCREEN_HEIGHT) - 1)
        ldir
        ret

; Copy only the rows used by the current state. If the viewport shrank, also
; flush the previously visible tail once so stale rows disappear from VRAM.
flush_visible_rows:
        ld a, (VISIBLE_ROWS)
        or a
        ret z
        ld b, a
        ld a, (LAST_VISIBLE_ROWS)
        cp b
        jr nc, flush_rows_have_count
        ld a, b
flush_rows_have_count:
        ld (FLUSH_ROW_LIMIT), a
        ld a, b
        ld (LAST_VISIBLE_ROWS), a
        ld hl, #SCREEN_BUFFER
        ld de, #NAME_TABLE
        ld a, (FLUSH_ROW_LIMIT)
flush_rows_loop:
        push af
        push hl
        push de
        ld b, #SCREEN_WIDTH
        call vram_copy_block
        pop de
        pop hl
        pop af
        ld bc, #SCREEN_WIDTH
        add hl, bc
        ex de, hl
        add hl, bc
        ex de, hl
        dec a
        jr nz, flush_rows_loop
        ret

vram_copy_block:
        ld c, b
        ld b, #0
        call LDIRVM
        ret

vram_copy_bc:
        call LDIRVM
        ret

; Mark the sprite list as empty by placing the terminator in the first slot.
init_sprite_buffer:
        ld hl, #SPRITE_BUFFER
        ld a, #SPRITE_END_Y
        ld (hl), a
        ret

; Rebuild the software sprite list every frame and upload it to VRAM.
flush_actor_sprites:
        call build_actor_sprites
        ld hl, #SPRITE_BUFFER
        ld de, #SPRITE_ATTR_TABLE
        ld b, #SPRITE_BYTES
        call vram_copy_block
        ret

; Menu states spend the sprite budget on the title overlay. Gameplay uses the
; same buffer for per-lane player and ghost sprites.
build_actor_sprites:
        ld hl, #SPRITE_BUFFER
        ld (SPRITE_LIST_PTR), hl
        ld a, (STATE_ID)
        cp #STATE_MENU
        jr z, build_menu_title_sprites
        cp #STATE_SCORES
        jr z, build_menu_title_sprites
        cp #STATE_PLAY
        jr nz, terminate_sprite_list
build_lane_state_sprites:
        xor a
        ld c, a
        ld a, (PLAYER_COUNT_V)
        ld b, a
build_actor_lane_loop:
        push bc
        ld a, c
        ld (LANE_INDEX_TMP), a
        call set_ix_to_lane_a
        call calc_lane_sprite_y
        call append_player_sprite
        call append_enemy_sprite
        call append_power_pellet_sprite
        pop bc
        inc c
        djnz build_actor_lane_loop
        jr terminate_sprite_list

build_menu_title_sprites:
        ld hl, #menu_title_sprite_defs
        ld de, #SPRITE_BUFFER
        ld bc, #(MENU_TITLE_SPRITE_COUNT * 4)
        ldir
        ex de, hl
        ld (SPRITE_LIST_PTR), hl
terminate_sprite_list:
        ld hl, (SPRITE_LIST_PTR)
        ld a, #SPRITE_END_Y
        ld (hl), a
        inc hl
        xor a
        ld (hl), a
        inc hl
        ld (hl), a
        inc hl
        ld (hl), a
        ret

; Anchor 16x16 sprites inside the lane between the two blue border rails.
calc_lane_sprite_y:
        call get_lane_row
        add a, #2
        add a, a
        add a, a
        add a, a
        sub #5
        ld (SPRITE_Y_TMP), a
        ret

append_player_sprite:
        ld a, (LANE_INDEX_TMP)
        ld hl, #player_sprite_colors
        call get_byte_from_table
        ld c, a
        ld a, L_STATUS(ix)
        and #ST_ALIVE
        jr nz, player_sprite_live
        ld b, #SPR_PAT_PAC_DEAD
        ld c, #SPR_COLOR_DEAD
        jr player_sprite_ready
player_sprite_live:
        ld a, L_ANM_H(ix)
        and #0x08
        jr z, player_sprite_direction
        ld b, #SPR_PAT_PAC_FULL
        jr player_sprite_ready
player_sprite_direction:
        ld a, L_VX(ix)
        cp #1
        ld b, #SPR_PAT_PAC_R
        jr z, player_sprite_ready
        ld b, #SPR_PAT_PAC_L
player_sprite_ready:
        ld l, L_PX_L(ix)
        ld h, L_PX_H(ix)
        ld a, (SPRITE_Y_TMP)
        ld d, a
        call append_sprite_entry
        ret

append_enemy_sprite:
        ld b, #SPR_PAT_GHOST
        ld a, L_EYE(ix)
        or a
        jr z, enemy_sprite_not_eyes
        ld b, #SPR_PAT_EYES
        ld c, #SPR_COLOR_EYES
        jr enemy_sprite_ready
enemy_sprite_not_eyes:
        ld a, L_PWR_L(ix)
        or L_PWR_H(ix)
        jr z, enemy_sprite_chase
        ld c, #SPR_COLOR_FRIGHT
        jr enemy_sprite_ready
enemy_sprite_chase:
        ld c, #SPR_COLOR_GHOST
enemy_sprite_ready:
        ld l, L_EX_L(ix)
        ld h, L_EX_H(ix)
        ld a, (SPRITE_Y_TMP)
        ld d, a
        call append_sprite_entry
        ret

append_power_pellet_sprite:
        ld b, L_PWR_IDX(ix)
        push bc
        call dot_is_active
        pop bc
        ret z
        ld a, L_ANM_H(ix)
        and #0x08
        ret z
        ld a, b
        ld hl, #dot_positions
        call get_byte_from_table
        ld h, a
        ld l, #0
        ld b, #SPR_PAT_POWER
        ld c, #SPR_COLOR_PELLET
        ld a, (SPRITE_Y_TMP)
        ld d, a
        call append_sprite_entry
        ret

; Convert fixed-point lane X into a sprite X coordinate and append one 4-byte
; MSX sprite attribute entry.
append_sprite_entry:
        push bc
        push de
        call position_to_sprite_x
        pop de
        pop bc
        ld hl, (SPRITE_LIST_PTR)
        ld (hl), d
        inc hl
        ld (hl), a
        inc hl
        ld (hl), b
        inc hl
        ld (hl), c
        inc hl
        ld (SPRITE_LIST_PTR), hl
        ret

draw_string:
        push hl
        call get_screen_ptr
        ex de, hl
        pop hl
draw_string_loop:
        ld a, (hl)
        or a
        ret z
        ld (de), a
        inc hl
        inc de
        jr draw_string_loop

draw_char:
        push af
        call get_screen_ptr
        pop af
        ld (hl), a
        ret

fill_span:
        push af
        push bc
        call get_screen_ptr
        pop bc
        pop af
        ld c, a
fill_span_loop:
        ld a, c
        ld (hl), a
        inc hl
        djnz fill_span_loop
        ret

draw_u8_2:
        push af
        call get_screen_ptr
        ex de, hl
        pop af
        ld b, #0
draw_u8_tens:
        cp #10
        jr c, draw_u8_store
        sub #10
        inc b
        jr draw_u8_tens
draw_u8_store:
        ld c, a
        ld a, b
        add a, #0x30
        ld (de), a
        inc de
        ld a, c
        add a, #0x30
        ld (de), a
        ret

draw_u16_5:
        push hl
        call get_screen_ptr
        ex de, hl
        pop hl
        ld bc, #10000
        call write_digit_bc
        ld bc, #1000
        call write_digit_bc
        ld bc, #100
        call write_digit_bc
        ld bc, #10
        call write_digit_bc
        ld a, l
        add a, #0x30
        ld (de), a
        ret

write_digit_bc:
        ld a, #0x30
write_digit_loop:
        or a
        sbc hl, bc
        jr c, write_digit_done
        inc a
        jr write_digit_loop
write_digit_done:
        add hl, bc
        ld (de), a
        inc de
        ret

; Translate D=row, E=col into a pointer inside the RAM screen buffer.
get_screen_ptr:
        ld a, d
        add a, a
        ld c, a
        ld b, #0
        ld hl, #row_offsets
        add hl, bc
        ld a, (hl)
        inc hl
        ld h, (hl)
        ld l, a
        ld bc, #SCREEN_BUFFER
        add hl, bc
        ld a, e
        ld c, a
        ld b, #0
        add hl, bc
        ret

; Translate D=row, E=col into a VRAM name-table address.
get_name_ptr:
        ld a, d
        add a, a
        ld c, a
        ld b, #0
        ld hl, #row_offsets
        add hl, bc
        ld a, (hl)
        inc hl
        ld h, (hl)
        ld l, a
        ld bc, #NAME_TABLE
        add hl, bc
        ld a, e
        ld c, a
        ld b, #0
        add hl, bc
        ret

get_byte_from_table:
        ld e, a
        ld d, #0
        add hl, de
        ld a, (hl)
        ret

get_word_from_table:
        add a, a
        ld e, a
        ld d, #0
        add hl, de
        ld e, (hl)
        inc hl
        ld d, (hl)
        ex de, hl
        ret

set_ix_to_lane_a:
        ld hl, #lane_ptrs
        call get_word_from_table
        push hl
        pop ix
        ret

absdiff_a_c:
        cp c
        jr nc, absdiff_done
        push bc
        ld b, a
        ld a, c
        sub b
        pop bc
        ret
absdiff_done:
        sub c
        ret

absdiff_player_enemy:
        bit 7, a
        jr z, absdiff_a_c
        cpl
        inc a
        add a, c
        ret

div3_a:
        call divmod3_a
        ld a, b
        ret

divmod3_a:
        ld b, #0
divmod3_loop:
        cp #3
        ret c
        sub #3
        inc b
        jr divmod3_loop

x_to_column:
        call div3_a
        cp #SCREEN_WIDTH
        ret c
        ld a, #SCREEN_WIDTH - 1
        ret

; Map the signed 8.8 fixed-point lane position to sprite X without a divide.
; floor(pos_fp / 128) = 2 * signed(H) + bit7(L), so:
; top_left_x = 18 - 8 + 2 * signed(H) + bit7(L)
position_to_sprite_x:
        ld a, l
        add a, a
        ld a, h
        adc a, a
        add a, #SPRITE_LEFT_X - SPRITE_ANCHOR_X
        ret

random8:
        ld a, (RAND_STATE)
        rrca
        jr nc, random_store
        xor #0xb8
random_store:
        ld (RAND_STATE), a
        ret

random_mod5:
        call random8
random_mod5_loop:
        cp #5
        ret c
        sub #5
        jr random_mod5_loop

mul8x8:
        ld e, a
        ld d, #0
        ld h, #0
        ld l, #0
        ld b, #8
mul8_loop:
        srl c
        jr nc, mul8_no_add
        add hl, de
mul8_no_add:
        sla e
        rl d
        djnz mul8_loop
        ret
