; Boot/setup and frame scheduler.

; Enter BIOS graphics mode, capture the base assets we need, seed state, and
; upload the initial SCREEN 2 / sprite content before the main loop starts.
init:
        di
        ld sp, #STACK_TOP
        call INIT32
        call capture_font_patterns
        ld a, #15
        ld (FORCLR), a
        ld a, #1
        ld (BAKCLR), a
        ld (BDRCLR), a
        call INIGRP
        call enable_16x16_sprites
        ld a, #2
        call CHGCLR
        call ERAFNK
        call KILBUF
        ld sp, #STACK_TOP
        call seed_random
        call reset_globals
        call detect_refresh_hz
        ld a, (JIFFY)
        ld (LAST_JIFFY), a
        call init_audio_system
        call init_vram_assets
        call sync_page0_assets
        call flush_actor_sprites
        ei

; Outer frame loop: wait for the next BIOS tick, sample input once, convert the
; elapsed refresh time into virtual 60 Hz simulation steps, then render once.
main_loop:
        ei
        ld a, (LAST_JIFFY)
        ld c, a
        ld hl, #JIFFY
main_wait_tick:
        ld a, (hl)
        cp c
        jr z, main_wait_tick
        ld b, a
        sub c
        ld c, a
        ld a, b
        ld (LAST_JIFFY), a
        push bc
        call read_inputs
        pop bc
        ld a, c
        push bc
        call run_simulation_steps
        pop bc
        di
        call sync_audio_mode
        ld a, c
        call update_audio_frames
        call sync_page0_assets
        call render_state
        call flush_visible_rows
        call flush_actor_sprites
        jr main_loop

; Reset all global/session variables to the menu defaults expected at cold boot.
reset_globals:
        xor a
        ld (ATTRACT_PHASE), a
        ld (MENU_INDEX), a
        ld (INPUT_CURR), a
        ld (INPUT_PREV), a
        ld (INPUT_EDGE), a
        ld (HI_SCORE_L), a
        ld (HI_SCORE_H), a
        ld (STATE_ID), a
        ld (PAGE0_ASSET_MODE), a
        ld (STEP_ACCUM), a
        ld (LAST_JIFFY), a
        ld (MENU_ACTION_CURR), a
        ld (MENU_ACTION_PREV), a
        ld (MENU_ACTION_EDGE), a
        ld (ANY_KEY_CURR), a
        ld (ANY_KEY_EDGE), a
        ld (JOY_CURR), a
        ld (JOY_PREV), a
        ld (JOY_EDGE), a
        ld (MENU_IDLE_L), a
        ld (MENU_IDLE_H), a
        ld (ATTRACT_TICKS_L), a
        ld (ATTRACT_TICKS_H), a
        ld (DEMO_MODE), a
        ld (DEMO_TICKS_L), a
        ld (DEMO_TICKS_H), a
        ld (TIME_LIMIT_IDX), a
        ld (TIME_LIMIT_L), a
        ld (TIME_LIMIT_H), a
        ld (TIMER_REM_L), a
        ld (TIMER_REM_H), a
        ld (TIMER_FRAME), a
        ld (ASSIGN_INDEX), a
        ld (COUNTDOWN_TICKS), a
        ld (WINNER_INDEX), a
        ld (WINNER_TICKS), a
        ld (POSTGAME_SCAN), a
        ld (NAME_LANE), a
        ld (NAME_RANK), a
        ld (NAME_POS), a
        ld (NAME_PROMPT_KIND), a
        ld (HISCORE_COUNT), a
        ld a, #0xff
        ld (PENDING_KEY_CODE), a
        xor a
        ld (PENDING_KEY_ROW), a
        ld (PENDING_KEY_MASK), a
        ld (AUDIO_ACCUM), a
        ld (MENU_RELEASE_WAIT), a
        ld (QUIT_CONFIRM_TICKS), a
        ld (LAST_VISIBLE_ROWS), a
        ld (ASSIGN_RELEASE_WAIT), a
        ld (NAME_RELEASE_WAIT), a
        ld (MENU_SAVE_TIME_IDX), a
        ld (AUDIO_MODE), a
        ld (PLAYER_COUNT_V), a
        inc a
        ld (PLAYER_COUNT_V), a
        ld (MENU_SAVE_PLAYERS), a
        call clear_input_buffers
        call clear_temp_name_buffer
        call clear_high_scores
        ld a, #5
        ld (FORCLR), a
        call init_sprite_buffer
        ret

; Detect PAL vs NTSC so the gameplay scheduler can keep a consistent 60 Hz feel
; on both 50 Hz and 60 Hz machines.
detect_refresh_hz:
        ld a, (MSXVER)
        or a
        jr z, detect_msx1_refresh
        ld a, (RG10SAV)
        and #0x02
        jr z, detect_refresh_60
        ld a, #50
        ld (REFRESH_HZ), a
        ret
detect_msx1_refresh:
        ld a, (BASRVN)
        and #0x80
        jr z, detect_refresh_60
        ld a, #50
        ld (REFRESH_HZ), a
        ret
detect_refresh_60:
        ld a, #60
        ld (REFRESH_HZ), a
        ret

; Cache the BIOS font in RAM so gameplay tiles and page-0 assets can be rebuilt
; without needing to read VRAM again later.
capture_font_patterns:
        ld hl, #PATTERN_TABLE
        ld de, #FONT_BUFFER
        ld bc, #0x0800
        call LDIRMV
        ret

; Enable 16x16 hardware sprites for gameplay actors and the menu title overlay.
enable_16x16_sprites:
        ld a, (RG1SAV)
        and #0xfe
        or #0x02
        ld (RG1SAV), a
        out (VDP_CTRL), a
        ld a, #0x81
        out (VDP_CTRL), a
        ret

; Upload the shared pattern/color pages and sprite patterns once at boot.
init_vram_assets:
        call overlay_custom_patterns_buffer
        ld hl, #FONT_BUFFER
        ld de, #PATTERN_TABLE + 0x0000
        ld bc, #0x0800
        call vram_copy_bc
        ld hl, #FONT_BUFFER
        ld de, #PATTERN_TABLE + 0x0800
        ld bc, #0x0800
        call vram_copy_bc
        ld hl, #FONT_BUFFER
        ld de, #PATTERN_TABLE + 0x1000
        ld bc, #0x0800
        call vram_copy_bc

        call build_color_page_buffer
        ld hl, #FONT_BUFFER
        ld de, #COLOR_TABLE + 0x0000
        ld bc, #0x0800
        call vram_copy_bc
        ld hl, #FONT_BUFFER
        ld de, #COLOR_TABLE + 0x0800
        ld bc, #0x0800
        call vram_copy_bc
        ld hl, #FONT_BUFFER
        ld de, #COLOR_TABLE + 0x1000
        ld bc, #0x0800
        call vram_copy_bc
        ld hl, #sprite_patterns
        ld de, #SPRITE_PATTERN_TABLE
        ld bc, #224
        call vram_copy_bc
        ld hl, #menu_title_sprite_patterns
        ld de, #(SPRITE_PATTERN_TABLE + 224)
        ld bc, #(SPRITE_PATTERN_BYTES - 224)
        call vram_copy_bc
        ret

; Merge the custom gameplay/menu tiles over the cached BIOS font page.
overlay_custom_patterns_buffer:
        ld hl, #tile_patterns
        ld de, #FONT_BUFFER
        ld bc, #256
        ldir
        ret

; Reuse FONT_BUFFER as a temporary 0x800-byte color page. Start from the
; default white-on-blue color, then overwrite the custom tiles.
build_color_page_buffer:
        ld hl, #FONT_BUFFER
        ld (hl), #0xf1
        ld de, #FONT_BUFFER + 1
        ld bc, #0x07ff
        ldir

        ld hl, #FONT_BUFFER
        ld de, #tile_colors
        ld b, #32
build_color_page_tiles_loop:
        ld a, (de)
        push bc
        ld b, #8
build_color_page_tile_fill:
        ld (hl), a
        inc hl
        djnz build_color_page_tile_fill
        pop bc
        inc de
        djnz build_color_page_tiles_loop
        ret

; Page 0 alternates between menu-title art and gameplay tiles. Only rewrite it
; when the active state actually changes to keep VRAM traffic down.
sync_page0_assets:
        ld a, (STATE_ID)
        or a
        jr z, sync_page0_menu
        cp #STATE_SCORES
        jr z, sync_page0_menu
        ld a, (PAGE0_ASSET_MODE)
        cp #PAGE0_ASSET_GAME
        ret z
        call restore_gameplay_page0_assets
        xor a
        ld (PAGE0_ASSET_MODE), a
        ret

sync_page0_menu:
        ld a, (PAGE0_ASSET_MODE)
        cp #PAGE0_ASSET_MENU
        ret z
        ld hl, #menu_title_page0_patterns
        ld de, #PATTERN_TABLE
        ld bc, #0x0800
        call vram_copy_bc
        ld hl, #menu_title_page0_colors
        ld de, #COLOR_TABLE
        ld bc, #0x0800
        call vram_copy_bc
        ld a, #PAGE0_ASSET_MENU
        ld (PAGE0_ASSET_MODE), a
        ret

; Restore the gameplay version of SCREEN 2 page 0 from the copies already
; stored in pages 1 and 2.
restore_gameplay_page0_assets:
        ld hl, #(PATTERN_TABLE + 0x0800)
        ld de, #FONT_BUFFER
        ld bc, #0x0800
        call LDIRMV
        ld hl, #FONT_BUFFER
        ld de, #PATTERN_TABLE
        ld bc, #0x0800
        call vram_copy_bc
        ld hl, #(COLOR_TABLE + 0x0800)
        ld de, #FONT_BUFFER
        ld bc, #0x0800
        call LDIRMV
        ld hl, #FONT_BUFFER
        ld de, #COLOR_TABLE
        ld bc, #0x0800
        call vram_copy_bc
        ret

seed_random:
        ld a, (JIFFY)
        or a
        jr nz, seed_ok
        ld a, #0x5a
seed_ok:
        or #1
        ld (RAND_STATE), a
        ret

update_state:
        jp update_state_new

; Convert elapsed refresh frames into virtual 60 Hz update steps. Edge-triggered
; inputs are cleared after each consumed simulation step so a single key press
; cannot fire twice if more than one step runs before the next render.
run_simulation_steps:
        ld b, a
        ld a, b
        or a
        ret z
run_frame_budget_loop:
        ld a, (STEP_ACCUM)
        add a, #60
        ld (STEP_ACCUM), a
run_step_loop:
        ld c, a
        ld a, (REFRESH_HZ)
        ld d, a
        ld a, c
        cp d
        jr c, run_frame_budget_next
        sub d
        ld (STEP_ACCUM), a
        push bc
        call update_state
        pop bc
        xor a
        ld (INPUT_EDGE), a
        ld (MENU_ACTION_EDGE), a
        ld (ANY_KEY_EDGE), a
        ld (JOY_EDGE), a
        ld a, #0xff
        ld (PENDING_KEY_CODE), a
        ld a, (STEP_ACCUM)
        jr run_step_loop
run_frame_budget_next:
        djnz run_frame_budget_loop
        ret
