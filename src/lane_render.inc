; Menu title rows and per-lane rendering helpers used by the current gameflow.

; The menu title art uses tiles 0..255 laid out directly across the first eight
; rows of SCREEN 2 page 0.
render_menu_title_rows:
        ld hl, #SCREEN_BUFFER
        xor a
        ld b, #8
render_menu_title_row_loop:
        ld c, #32
render_menu_title_col_loop:
        ld (hl), a
        inc hl
        inc a
        dec c
        jr nz, render_menu_title_col_loop
        djnz render_menu_title_row_loop
        ret

; Draw one live gameplay lane: label/score strip, border rails, and the dynamic
; tile row that carries pellet fragments.
render_one_lane:
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
        ld l, L_SCORE_L(ix)
        ld h, L_SCORE_H(ix)
        ld e, #3
        call draw_u16_5

        ld a, (LANE_ROW_TMP)
        ld d, a
        ld hl, #str_x
        ld e, #10
        call draw_string
        ld a, L_MULT(ix)
        ld c, a
        ld a, (LANE_ROW_TMP)
        ld d, a
        ld a, c
        ld e, #11
        call draw_u8_2

        ld a, (SERIES_TYPE_V)
        cp #SERIES_CHAMP
        jr z, render_lane_points
        cp #SERIES_LEAGUE
        jr nz, render_lane_dead
render_lane_points:
        ld hl, #str_pt
        ld a, (LANE_ROW_TMP)
        ld d, a
        ld e, #15
        call draw_string
        ld a, L_SERIES(ix)
        ld c, a
        ld a, (LANE_ROW_TMP)
        ld d, a
        ld a, c
        ld e, #17
        call draw_u8_2

render_lane_dead:
        ld a, L_STATUS(ix)
        and #ST_ALIVE
        jr nz, render_lane_track
        ld hl, #str_out
        ld a, (LANE_ROW_TMP)
        ld d, a
        ld e, #23
        call draw_string

render_lane_track:
        ld a, (LANE_ROW_TMP)
        add a, #2
        ld (PLAYFIELD_ROW_TMP), a
        dec a
        ld d, a
        ld e, #LANE_LEFT_COL
        ld a, #TILE_WALL_TOP
        ld b, #LANE_SPAN_COLS
        call fill_span
        ld a, (PLAYFIELD_ROW_TMP)
        inc a
        ld d, a
        ld e, #LANE_LEFT_COL
        ld a, #TILE_WALL_BOT
        ld b, #LANE_SPAN_COLS
        call fill_span
        call build_lane_playrow
        ret

; Each lane owns 26 dynamic 8x8 tiles. Clear that cache, stamp the tile IDs
; into the visible row, then OR pellet fragments into the tiles for every
; active dot.
build_lane_playrow:
        ld a, (LANE_INDEX_TMP)
        ld hl, #dynamic_tile_ptrs
        call get_word_from_table
        push hl
        ld (DYN_TILE_PTR_TMP), hl
        ld (hl), #0
        ld d, h
        ld e, l
        inc de
        ld bc, #DYN_TILE_BYTES - 1
        ldir
        pop hl
        ld (DYN_TILE_PTR_TMP), hl

        ld a, (PLAYFIELD_ROW_TMP)
        ld d, a
        ld e, #LANE_LEFT_COL
        call get_screen_ptr
        ex de, hl
        push de
        ld a, (LANE_INDEX_TMP)
        ld hl, #dynamic_tile_id_bases
        call get_byte_from_table
        ld c, a
        pop de
        ld a, c
        ld b, #DYN_TILES_PER_LANE
build_lane_tile_ids_loop:
        ld (de), a
        inc de
        inc a
        djnz build_lane_tile_ids_loop

        ld b, #0
build_lane_dot_loop:
        ld a, b
        cp #16
        ret z
        push bc
        call dot_is_active
        pop bc
        jr z, build_lane_dot_next

        ld a, b
        ld hl, #dot_tile_indices
        call get_byte_from_table
        ld c, a

        ld a, b
        cp L_PWR_IDX(ix)
        jr nz, build_lane_small
        ld a, L_ANM_H(ix)
        and #0x08
        jr z, build_lane_dot_next
        bit 0, b
        jr z, build_lane_power_single
        push bc
        ld a, c
        dec a
        ld de, #power_dot_phase0_left
        call or_pattern_into_lane_tile
        pop bc
        ld a, c
        ld de, #power_dot_phase0_right
        call or_pattern_into_lane_tile
        jr build_lane_dot_next
build_lane_power_single:
        ld a, c
        ld de, #power_dot_phase4
        call or_pattern_into_lane_tile
        jr build_lane_dot_next

build_lane_small:
        bit 0, b
        jr z, build_lane_small_single
        push bc
        ld a, c
        dec a
        ld de, #small_dot_phase0_left
        call or_pattern_into_lane_tile
        pop bc
        ld a, c
        ld de, #small_dot_phase0_right
        call or_pattern_into_lane_tile
        jr build_lane_dot_next
build_lane_small_single:
        ld a, c
        ld de, #small_dot_phase4
        call or_pattern_into_lane_tile

build_lane_dot_next:
        inc b
        jr build_lane_dot_loop

; Pellet shapes can straddle tile boundaries, so their 8-byte fragments are
; composited into the lane-local dynamic tile set instead of using fixed tiles.
or_pattern_into_lane_tile:
        push bc
        push de
        call get_current_lane_tile_ptr
        pop de
        ld b, #8
or_pattern_loop:
        ld a, (de)
        or (hl)
        ld (hl), a
        inc de
        inc hl
        djnz or_pattern_loop
        pop bc
        ret

get_current_lane_tile_ptr:
        add a, a
        add a, a
        add a, a
        ld e, a
        ld d, #0
        ld hl, (DYN_TILE_PTR_TMP)
        add hl, de
        ret

; Vertical lane spacing depends on the current 1p/2p/3p/4p split-screen layout.
get_lane_row:
        ld hl, #lane_rows_4p
        ld a, (PLAYER_COUNT_V)
        cp #1
        jr z, get_lane_row_pick
        cp #2
        jr z, get_lane_row_pick_2p
        cp #3
        jr nz, get_lane_row_have_table
        ld hl, #lane_rows_3p
        jr get_lane_row_have_table
get_lane_row_pick_2p:
        ld hl, #lane_rows_2p
        jr get_lane_row_have_table
get_lane_row_pick:
        ld hl, #lane_rows_1p
get_lane_row_have_table:
        ld a, (LANE_INDEX_TMP)
        call get_byte_from_table
        ret
