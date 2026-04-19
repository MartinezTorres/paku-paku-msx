; PAKU PAKU MSX
; An MSX cartridge port that keeps the original one-button lane mechanics
; and extends them with score modes for 1, 2 and 4 simultaneous players.

; BIOS entry points and helper routines.
CHGCLR      .equ 0x0062
INIT32      .equ 0x006f
INIGRP      .equ 0x0072
ERAFNK      .equ 0x00cc
KILBUF      .equ 0x0156
SNSMAT      .equ 0x0141
GTSTCK      .equ 0x00d5
GTTRIG      .equ 0x00d8
FILVRM      .equ 0x0056
LDIRMV      .equ 0x0059
LDIRVM      .equ 0x005c

; BIOS work area, system variables, and raw VDP ports.
BASRVN      .equ 0x002b
MSXVER      .equ 0x002d
FORCLR      .equ 0xf3e9
BAKCLR      .equ 0xf3ea
BDRCLR      .equ 0xf3eb
RG1SAV      .equ 0xf3e0
RG10SAV     .equ 0xffe9
JIFFY       .equ 0xfc9e
VDP_DATA    .equ 0x98
VDP_CTRL    .equ 0x99

; SCREEN 2 VRAM layout plus general display geometry.
NAME_TABLE      .equ 0x1800
PATTERN_TABLE   .equ 0x0000
COLOR_TABLE     .equ 0x2000
SPRITE_ATTR_TABLE   .equ 0x1b00
SPRITE_PATTERN_TABLE .equ 0x3800
SCREEN_WIDTH    .equ 32
SCREEN_HEIGHT   .equ 24
SPRITE_BUFFER   .equ 0xc300
SPRITE_BYTES    .equ 52
SPRITE_PATTERN_BYTES .equ 448
SPRITE_END_Y    .equ 208
LANE_LEFT_COL   .equ 2
LANE_SPAN_COLS  .equ 26
SPRITE_LEFT_X   .equ 18
SPRITE_ANCHOR_X .equ 8

; Stack top kept above the gameplay work area.
STACK_TOP   .equ 0xdff0

; High-level game states.
STATE_MENU      .equ 0
STATE_ASSIGN    .equ 1
STATE_COUNTDOWN .equ 2
STATE_PLAY      .equ 3
STATE_WINNER    .equ 4
STATE_NAME      .equ 5
STATE_SCORES    .equ 6
STATE_ROUND     .equ STATE_WINNER
STATE_SERIES    .equ STATE_NAME

; Attract cycle phases for the idle menu flow.
ATTRACT_TO_SCORES .equ 0
ATTRACT_TO_DEMO   .equ 1

; Match type identifiers.
SERIES_ENDLESS  .equ 0
SERIES_SPRINT   .equ 1
SERIES_CHAMP    .equ 2
SERIES_LEAGUE   .equ 3

; Menu and player input bitmasks.
MODE_COUNT      .equ 5
MENU_ITEM_COUNT .equ 3
IN_P1       .equ 0x01
IN_P2       .equ 0x02
IN_P3       .equ 0x04
IN_P4       .equ 0x08
M_ACT_UP    .equ 0x01
M_ACT_DOWN  .equ 0x02
M_ACT_LEFT  .equ 0x04
M_ACT_RIGHT .equ 0x08
M_ACT_SELECT .equ 0x10
M_ACT_ESC   .equ 0x20
IN_UP       .equ M_ACT_UP
IN_DOWN     .equ M_ACT_DOWN
IN_SELECT   .equ M_ACT_SELECT

; Background and UI tile IDs.
TILE_PAC_R      .equ 0
TILE_PAC_L      .equ 1
TILE_PAC_OPEN   .equ 2
TILE_PAC_DEAD   .equ 3
TILE_DOT        .equ 4
TILE_POWER      .equ 5
TILE_CURSOR     .equ 6
TILE_STAR       .equ 7
TILE_GHOST      .equ 8
TILE_GHOST_BLUE .equ 16
TILE_EYES       .equ 17
TILE_CUP        .equ 18
TILE_WALL_TOP   .equ 24
TILE_WALL_BOT   .equ 25
TILE_DOT_SPLIT_R .equ 12
TILE_DOT_SPLIT_L .equ 13

; Sprite pattern indices inside the hardware sprite pattern table.
SPR_PAT_PAC_R       .equ 0
SPR_PAT_PAC_L       .equ 4
SPR_PAT_PAC_FULL    .equ 8
SPR_PAT_PAC_DEAD    .equ 12
SPR_PAT_GHOST       .equ 16
SPR_PAT_EYES        .equ 20
SPR_PAT_POWER       .equ 24
SPR_PAT_MENU_Y_TOP  .equ 28
SPR_PAT_MENU_Y_MID  .equ 32
SPR_PAT_MENU_Y_GHOST_HI .equ 36
SPR_PAT_MENU_Y_GHOST_LO .equ 40
SPR_PAT_MENU_Y_EDGE_HI  .equ 44
SPR_PAT_MENU_Y_EDGE_LO  .equ 48
SPR_PAT_MENU_B_EYE  .equ 52

; Page 0 asset modes and menu title sprite count.
PAGE0_ASSET_GAME    .equ 0
PAGE0_ASSET_MENU    .equ 1
MENU_TITLE_SPRITE_COUNT .equ 10

; Sprite colors.
SPR_COLOR_GHOST     .equ 8
SPR_COLOR_FRIGHT    .equ 7
SPR_COLOR_EYES      .equ 15
SPR_COLOR_DEAD      .equ 14
SPR_COLOR_PELLET    .equ 11

; Arkos stand-alone sound-effects player copied into page-2 RAM at boot.
SE_PLAYER_BASE      .equ 0x8000
SE_WRAPPER_SIZE     .equ 670
SE_SFX_BANK_OFS     .equ 0x0000
SE_INIT_SFX_OFS     .equ 0x00da
SE_PLAY_SFX_OFS     .equ 0x00ea
SE_STOP_SFX_OFS     .equ 0x0112
SE_PLAY_FRAME_OFS   .equ 0x0133

; Audio modes, channels, and the gameplay-oriented effect mapping.
AUDIO_MODE_STOPPED .equ 0
AUDIO_MODE_ACTIVE   .equ 1
AUDIO_CH_UI         .equ 0
AUDIO_CH_GAME       .equ 1
AUDIO_CH_MAJOR      .equ 2
SFX_PELLET          .equ 1
SFX_DEATH           .equ 2
SFX_REFILL          .equ 3
SFX_POWER           .equ 4
SFX_GHOST           .equ 5

; Gameplay-space positions and timers in 8.8 fixed point.
TRACK_MAX   .equ 0x6400
PLAYER_START .equ 0x2800
PLAYER_WRAP_MIN .equ 0xfd00
PLAYER_WRAP_MAX .equ 0x6700
ENEMY_START  .equ 0x6400
POWER_INIT   .equ 0x7800

; Lane status flags.
ST_PART     .equ 0x01
ST_ALIVE    .equ 0x02

; Per-lane struct layout.
L_PX_L      .equ 0
L_PX_H      .equ 1
L_VX        .equ 2
L_EX_L      .equ 3
L_EX_H      .equ 4
L_EYE       .equ 5
L_PWR_L     .equ 6
L_PWR_H     .equ 7
L_ANM_L     .equ 8
L_ANM_H     .equ 9
L_MULT      .equ 10
L_DOTS_L    .equ 11
L_DOTS_H    .equ 12
L_PWR_IDX   .equ 13
L_SCORE_L   .equ 14
L_SCORE_H   .equ 15
L_STATUS    .equ 16
L_SERIES    .equ 17
L_BEST_L    .equ 18
L_BEST_H    .equ 19

LANE_SIZE   .equ 20

; Main screen buffer in RAM, copied to the VRAM name table each frame.
SCREEN_BUFFER   .equ 0xc000

; Core session state, attract/demo bookkeeping, and timing work area.
ATTRACT_PHASE   .equ 0xda00
STATE_ID        .equ 0xda01
MENU_INDEX      .equ 0xda02
INPUT_CURR      .equ 0xda03
INPUT_PREV      .equ 0xda04
INPUT_EDGE      .equ 0xda05
PLAYER_COUNT_V  .equ 0xda06
ATTRACT_TICKS_L .equ 0xda07
ATTRACT_TICKS_H .equ 0xda08
DEMO_MODE       .equ 0xda09
DEMO_TICKS_L    .equ 0xda0a
DEMO_TICKS_H    .equ 0xda0b
SERIES_TYPE_V   .equ 0xda0c
VISIBLE_ROWS    .equ 0xda0d
GLOBAL_TICKS_L  .equ 0xda0e
GLOBAL_TICKS_H  .equ 0xda0f
DIFF_FP_L       .equ 0xda10
DIFF_FP_H       .equ 0xda11
DIFF_PHASE_L    .equ 0xda12
DIFF_PHASE_H    .equ 0xda13
PLAYER_DELTA_L  .equ 0xda14
PLAYER_DELTA_H  .equ 0xda15
CHASE_DELTA_L   .equ 0xda16
CHASE_DELTA_H   .equ 0xda17
FRIGHT_DELTA_L  .equ 0xda18
FRIGHT_DELTA_H  .equ 0xda19
EYE_DELTA_L     .equ 0xda1a
EYE_DELTA_H     .equ 0xda1b
HI_SCORE_L      .equ 0xda1c
HI_SCORE_H      .equ 0xda1d
RAND_STATE      .equ 0xda1e
STEP_ACCUM      .equ 0xda1f
; Shared scratch slots. These aliases intentionally overlap when used in
; separate routines, so prefer the most specific name available at each use.
LANE_INDEX_TMP      .equ 0xda20
ACTIVE_LANE_COUNT   .equ 0xda20
ENEMY_MOVE_DIR      .equ 0xda20
HISCORE_CANDIDATE_L .equ 0xda20
LANE_ROW_TMP        .equ 0xda21
HISCORE_CANDIDATE_H .equ 0xda21
PLAYFIELD_ROW_TMP   .equ 0xda22
UNUSED_SCRATCH      .equ 0xda23
SPRITE_Y_TMP        .equ 0xda24
SPRITE_LIST_PTR     .equ 0xda25
FLUSH_ROW_LIMIT     .equ 0xda26
REFRESH_HZ      .equ 0xda28
LAST_JIFFY      .equ 0xda29
PAGE0_ASSET_MODE .equ 0xda2a
ASSIGN_RELEASE_WAIT .equ 0xda2b
NAME_RELEASE_WAIT .equ 0xda2c
INPUT_LOCKS     .equ 0xda2f
RANKS_BASE      .equ 0xda30

; Menu navigation, joystick state, and match configuration.
MENU_ACTION_CURR    .equ 0xda40
MENU_ACTION_PREV    .equ 0xda41
MENU_ACTION_EDGE    .equ 0xda42
ANY_KEY_CURR        .equ 0xda43
ANY_KEY_EDGE        .equ 0xda44
JOY_CURR            .equ 0xda45
JOY_PREV            .equ 0xda46
JOY_EDGE            .equ 0xda47
MENU_IDLE_L         .equ 0xda48
MENU_IDLE_H         .equ 0xda49
TIME_LIMIT_IDX      .equ 0xda4a
TIME_LIMIT_L        .equ 0xda4b
TIME_LIMIT_H        .equ 0xda4c
TIMER_REM_L         .equ 0xda4d
TIMER_REM_H         .equ 0xda4e
TIMER_FRAME         .equ 0xda4f
ASSIGN_INDEX        .equ 0xda50
COUNTDOWN_TICKS     .equ 0xda51
WINNER_INDEX        .equ 0xda52
WINNER_TICKS        .equ 0xda53
POSTGAME_SCAN       .equ 0xda54

; Name-entry, high-score insertion, and raw keyboard matrix state.
NAME_LANE           .equ 0xda55
NAME_RANK           .equ 0xda56
NAME_POS            .equ 0xda57
NAME_PROMPT_KIND    .equ 0xda58
HISCORE_COUNT       .equ 0xda59
PENDING_KEY_CODE    .equ 0xda5a
PENDING_KEY_ROW     .equ 0xda5b
PENDING_KEY_MASK    .equ 0xda5c
MENU_RELEASE_WAIT   .equ 0xda5d
QUIT_CONFIRM_TICKS  .equ 0xda5e
LAST_VISIBLE_ROWS   .equ 0xda5f
RAW_MATRIX_CURR     .equ 0xda60
RAW_MATRIX_PREV     .equ 0xda6b
BIND_ROWS_BASE      .equ 0xda76
BIND_MASKS_BASE     .equ 0xda7a
TEMP_NAME_BUF       .equ 0xda7e
AUDIO_MODE          .equ 0xda86
AUDIO_ACCUM         .equ 0xda87
HISCORE_TABLE       .equ 0xdab0
HISCORE_ENTRY_SIZE  .equ 10
HISCORE_NAME_LEN    .equ 8
MENU_SAVE_PLAYERS   .equ 0xda2d
MENU_SAVE_TIME_IDX  .equ 0xda2e

; Cached font patterns and sprite attribute staging.
FONT_BUFFER     .equ 0xc600

; Per-player lane structs in RAM.
LANES_BASE      .equ 0xc500
LANE0           .equ LANES_BASE
LANE1           .equ LANES_BASE + LANE_SIZE
LANE2           .equ LANES_BASE + (LANE_SIZE * 2)
LANE3           .equ LANES_BASE + (LANE_SIZE * 3)

        .module paku

        .area _HEADER (ABS)
        .org 0x4000

        .db 0x41, 0x42
        .dw init
        .dw 0x0000
        .dw 0x0000
        .dw 0x0000
        .ds 6

        .area _ROM (ABS)
        .org 0x4010

        ; Boot, refresh timing, and VRAM/bootstrap setup.
        .include "startup.inc"

        ; High-level state flow, menu flow, and input binding logic.
        .include "gameflow.inc"

        ; Arkos audio glue plus the active simulation/rendering modules.
        .include "audio.inc"
        .include "simulation.inc"
        .include "lane_render.inc"
        .include "ranking.inc"

        ; Shared rendering helpers, table data, and art assets.
        .include "render_helpers.inc"
        .include "data.inc"
        .include "assets.inc"
