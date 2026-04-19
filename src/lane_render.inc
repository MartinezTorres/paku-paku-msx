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

; Draw one live gameplay lane: label/score strip, border rails, and the fixed
; tile row that carries the small pellets.
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

        ld a, (PLAYER_COUNT_V)
        cp #2
        jr c, render_lane_points_gate
        ld a, (LANE_ROW_TMP)
        ld d, a
        ld e, #14
        ld a, #'B'
        call draw_char
        ld a, (LANE_ROW_TMP)
        ld d, a
        ld l, L_BEST_L(ix)
        ld h, L_BEST_H(ix)
        ld e, #15
        call draw_u16_5

render_lane_points_gate:
        ld a, (SERIES_TYPE_V)
        cp #SERIES_CHAMP
        jr z, render_lane_points
        cp #SERIES_LEAGUE
        jr nz, render_lane_dead
render_lane_points:
        ld a, (PLAYER_COUNT_V)
        cp #2
        jr nc, render_lane_dead
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

; The play row is now a fixed 26-tile strip. Small pellets become background
; tile IDs, while the power pellet is handled by the sprite path.
build_lane_playrow:
        ld a, (PLAYFIELD_ROW_TMP)
        ld d, a
        ld e, #LANE_LEFT_COL
        call get_screen_ptr
        ld (hl), #' '
        ld d, h
        ld e, l
        inc de
        ld bc, #LANE_SPAN_COLS - 1
        ldir

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
        jr z, build_lane_dot_next

        bit 0, b
        jr z, build_lane_small_single
        push bc
        ld a, c
        dec a
        call lane_tile_offset_to_ptr
        ld (hl), #TILE_DOT_SPLIT_L
        pop bc
        push bc
        ld a, c
        call lane_tile_offset_to_ptr
        ld (hl), #TILE_DOT_SPLIT_R
        pop bc
        jr build_lane_dot_next
build_lane_small_single:
        push bc
        ld a, c
        call lane_tile_offset_to_ptr
        ld (hl), #TILE_DOT
        pop bc

build_lane_dot_next:
        inc b
        jr build_lane_dot_loop

lane_tile_offset_to_ptr:
        push af
        ld a, (PLAYFIELD_ROW_TMP)
        ld d, a
        ld e, #LANE_LEFT_COL
        call get_screen_ptr
        pop af
        ld e, a
        ld d, #0
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
