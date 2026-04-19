; Keyboard and joystick scanning, plus player/menu input decoding.

; Clear both cooked input state and the raw matrix snapshots used for edge
; detection between scans.
clear_input_buffers:
        xor a
        ld (INPUT_CURR), a
        ld (INPUT_PREV), a
        ld (INPUT_EDGE), a
        ld (MENU_ACTION_CURR), a
        ld (MENU_ACTION_PREV), a
        ld (MENU_ACTION_EDGE), a
        ld (ANY_KEY_CURR), a
        ld (ANY_KEY_EDGE), a
        ld (JOY_CURR), a
        ld (JOY_PREV), a
        ld (JOY_EDGE), a
        ld (MENU_RELEASE_WAIT), a
        ld (QUIT_CONFIRM_TICKS), a
        ld (PENDING_KEY_ROW), a
        ld (PENDING_KEY_MASK), a
        ld (INPUT_LOCKS), a
        ld hl, #RAW_MATRIX_CURR
        ld (hl), a
        ld de, #RAW_MATRIX_CURR + 1
        ld bc, #(22 - 1)
        ldir
        ld a, #0xff
        ld (PENDING_KEY_CODE), a
        ret

; Consume one-shot edge/pending-key latches while preserving the current held
; keyboard state. Release-gated states use this after a transition so a key
; that remains physically held cannot re-trigger on the next scan.
clear_input_edges_keep_hold_new:
        xor a
        ld (INPUT_EDGE), a
        ld (MENU_ACTION_EDGE), a
        ld (ANY_KEY_EDGE), a
        ld (JOY_EDGE), a
        ld (PENDING_KEY_ROW), a
        ld (PENDING_KEY_MASK), a
        ld a, #0xff
        ld (PENDING_KEY_CODE), a
        ret

read_inputs:
        jp read_inputs_new

; Scan the keyboard matrix once, preserve the previous rows, and latch the
; first newly pressed key for binding/name-entry flows.
read_inputs_new:
        ld a, (INPUT_CURR)
        ld (INPUT_PREV), a
        ld a, (MENU_ACTION_CURR)
        ld (MENU_ACTION_PREV), a
        ld hl, #RAW_MATRIX_CURR
        ld de, #RAW_MATRIX_PREV
        ld bc, #11
        ldir
        xor a
        ld (INPUT_CURR), a
        ld (INPUT_EDGE), a
        ld (MENU_ACTION_CURR), a
        ld (MENU_ACTION_EDGE), a
        ld (ANY_KEY_CURR), a
        ld (ANY_KEY_EDGE), a
        ld (PENDING_KEY_ROW), a
        ld (PENDING_KEY_MASK), a
        ld a, #0xff
        ld (PENDING_KEY_CODE), a
        ld hl, #RAW_MATRIX_CURR
        ld de, #RAW_MATRIX_PREV
        ld b, #11
        ld c, #0
read_inputs_scan_loop:
        push bc
        push hl
        push de
        ld a, c
        call SNSMAT
        cpl
        pop de
        pop hl
        ld (hl), a
        ld b, a
        ld a, (ANY_KEY_CURR)
        or b
        ld (ANY_KEY_CURR), a
        ld a, (de)
        cpl
        and b
        ld b, a
        ld a, (ANY_KEY_EDGE)
        or b
        ld (ANY_KEY_EDGE), a
        ld a, (PENDING_KEY_CODE)
        cp #0xff
        jr nz, read_inputs_scan_next
        ld a, b
        or a
        jr z, read_inputs_scan_next
        ld a, c
        ld (PENDING_KEY_ROW), a
        ld a, b
        ld (PENDING_KEY_MASK), a
        call first_key_code_from_mask_new
        ld (PENDING_KEY_CODE), a
read_inputs_scan_next:
        inc hl
        inc de
        pop bc
        inc c
        djnz read_inputs_scan_loop
        call build_menu_actions_new
        call read_joystick_any_new
        ld b, a
        ld a, (JOY_CURR)
        ld (JOY_PREV), a
        ld a, b
        ld (JOY_CURR), a
        ld c, a
        ld a, (JOY_PREV)
        cpl
        and c
        ld (JOY_EDGE), a
        call build_player_inputs_new
        ret

; Menus always use the fixed cursor/space/enter/esc bindings, independent of
; the gameplay key assignments stored per player.
build_menu_actions_new:
        ld a, #8
        call get_raw_curr_row_new
        ld b, a
        ld a, #7
        call get_raw_curr_row_new
        ld c, a
        xor a
        bit 5, b
        jr z, build_menu_no_up
        or #M_ACT_UP
build_menu_no_up:
        bit 6, b
        jr z, build_menu_no_down
        or #M_ACT_DOWN
build_menu_no_down:
        bit 4, b
        jr z, build_menu_no_left
        or #M_ACT_LEFT
build_menu_no_left:
        bit 7, b
        jr z, build_menu_no_right
        or #M_ACT_RIGHT
build_menu_no_right:
        bit 0, b
        jr z, build_menu_no_space
        or #M_ACT_SELECT
build_menu_no_space:
        bit 7, c
        jr z, build_menu_no_ret
        or #M_ACT_SELECT
build_menu_no_ret:
        bit 2, c
        jr z, build_menu_store
        or #M_ACT_ESC
build_menu_store:
        ld (MENU_ACTION_CURR), a
        xor a
        ld (MENU_ACTION_EDGE), a
        ld a, #8
        call get_raw_edge_row_new
        ld b, a
        ld a, (MENU_ACTION_EDGE)
        bit 5, b
        jr z, build_menu_edge_no_up
        or #M_ACT_UP
build_menu_edge_no_up:
        bit 6, b
        jr z, build_menu_edge_no_down
        or #M_ACT_DOWN
build_menu_edge_no_down:
        bit 4, b
        jr z, build_menu_edge_no_left
        or #M_ACT_LEFT
build_menu_edge_no_left:
        bit 7, b
        jr z, build_menu_edge_no_right
        or #M_ACT_RIGHT
build_menu_edge_no_right:
        bit 0, b
        jr z, build_menu_edge_no_space
        or #M_ACT_SELECT
build_menu_edge_no_space:
        ld (MENU_ACTION_EDGE), a
        ld a, #7
        call get_raw_edge_row_new
        ld b, a
        ld a, (MENU_ACTION_EDGE)
        bit 7, b
        jr z, build_menu_edge_no_ret
        or #M_ACT_SELECT
build_menu_edge_no_ret:
        bit 2, b
        jr z, build_menu_edge_store
        or #M_ACT_ESC
build_menu_edge_store:
        ld (MENU_ACTION_EDGE), a
        ret

; Single-player treats any keyboard/joystick activity as player 1 input.
; Multiplayer rebuilds the held/edge masks from the per-player key bindings.
build_player_inputs_new:
        xor a
        ld (INPUT_CURR), a
        ld (INPUT_EDGE), a
        ld a, (PLAYER_COUNT_V)
        cp #1
        jr nz, build_player_inputs_multi
        ld a, (ANY_KEY_CURR)
        ld b, a
        ld a, (JOY_CURR)
        or b
        jr z, build_player_inputs_single_curr_zero
        ld a, #IN_P1
        ld (INPUT_CURR), a
build_player_inputs_single_curr_zero:
        ld a, (ANY_KEY_EDGE)
        ld b, a
        ld a, (JOY_EDGE)
        or b
        ret z
        ld a, #IN_P1
        ld (INPUT_EDGE), a
        ret

; Check each player's bound row/mask pair against both current and previous
; scans to build held and edge-triggered input bits per lane.
build_player_inputs_multi:
        xor a
        ld c, a
        ld a, (PLAYER_COUNT_V)
        ld b, a
build_player_inputs_multi_loop:
        push bc
        ld a, c
        ld hl, #BIND_ROWS_BASE
        call get_offset_ptr_a_hl
        ld b, (hl)
        ld a, c
        ld hl, #BIND_MASKS_BASE
        call get_offset_ptr_a_hl
        ld a, (hl)
        push af
        ld a, b
        call get_raw_curr_row_new
        push af
        ld a, b
        call get_raw_prev_row_new
        ld d, a
        pop af
        ld h, a
        pop af
        ld e, a
        ld a, d
        cpl
        and e
        and h
        ld d, a
        ld a, h
        and e
        jr z, build_player_inputs_no_curr
        call get_player_bit_from_index_new
        ld b, a
        ld a, (INPUT_CURR)
        or b
        ld (INPUT_CURR), a
build_player_inputs_no_curr:
        ld a, d
        jr z, build_player_inputs_next
        call get_player_bit_from_index_new
        ld e, a
        ld a, (INPUT_EDGE)
        or e
        ld (INPUT_EDGE), a
build_player_inputs_next:
        pop bc
        inc c
        djnz build_player_inputs_multi_loop
        ret

get_player_bit_from_index_new:
        ld a, c
        or a
        ld a, #1
        ret z
        ld b, c
get_player_bit_shift_loop:
        add a, a
        djnz get_player_bit_shift_loop
        ret

; Menus and single-player only care whether some joystick activity happened, so
; any direction or trigger maps to a single digital flag.
read_joystick_any_new:
        ld a, #1
        call GTSTCK
        or a
        jr nz, read_joystick_yes
        ld a, #2
        call GTSTCK
        or a
        jr nz, read_joystick_yes
        ld a, #1
        call GTTRIG
        or a
        jr nz, read_joystick_yes
        ld a, #2
        call GTTRIG
        or a
        jr nz, read_joystick_yes
        xor a
        ret
read_joystick_yes:
        ld a, #1
        ret

; Convert the first set bit in the pending row mask into a compact
; row*8+column key code.
first_key_code_from_mask_new:
        ld a, (PENDING_KEY_MASK)
        ld b, #0
first_key_code_shift_loop:
        rrca
        jr c, first_key_code_found
        inc b
        jr first_key_code_shift_loop
first_key_code_found:
        ld a, (PENDING_KEY_ROW)
        add a, a
        add a, a
        add a, a
        add a, b
        ret

get_raw_curr_row_new:
        ld hl, #RAW_MATRIX_CURR
        jp get_byte_from_table

get_raw_prev_row_new:
        ld hl, #RAW_MATRIX_PREV
        jp get_byte_from_table

get_raw_edge_row_new:
        push bc
        push af
        call get_raw_curr_row_new
        ld b, a
        pop af
        call get_raw_prev_row_new
        cpl
        and b
        pop bc
        ret

; Name entry uses a lookup table that maps compact key codes to ASCII/control.
translate_pending_key_new:
        ld a, (PENDING_KEY_CODE)
        ld hl, #name_entry_keymap
        call get_byte_from_table
        ret
