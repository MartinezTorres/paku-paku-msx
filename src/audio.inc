; Arkos stand-alone sound-effects integration.
;
; The original web game only uses gameplay sounds:
; - pellet: "hit"
; - power pellet: "jump"
; - ghost eaten: "powerUp"
; - death: "explosion"
; - field refill: "coin"
;
; Menu/countdown sounds stay silent for now so the audio path is focused on the
; core gameplay events.

init_audio_system:
        ld hl, #arkos_se_blob
        ld de, #SE_PLAYER_BASE
        ld bc, #SE_WRAPPER_SIZE
        ldir
        ld hl, #se_sfx_bank
        call se_init_sfx_entry
        ld a, #AUDIO_MODE_ACTIVE
        ld (AUDIO_MODE), a
        xor a
        ld (AUDIO_ACCUM), a
        ret

sync_audio_mode:
        ret

update_audio_frames:
        ld a, (AUDIO_MODE)
        or a
        ret z
        call se_play_frame_entry
        ret

play_sfx_new:
        ld a, (AUDIO_MODE)
        or a
        ret z
        ld b, #0
        jp se_play_sfx_entry

play_ui_move_sfx_new:
        ret

play_pellet_sfx_new:
        ld a, #SFX_PELLET
        ld c, #AUDIO_CH_GAME
        jp play_sfx_new

play_refill_sfx_new:
        ld a, #SFX_REFILL
        ld c, #AUDIO_CH_MAJOR
        jp play_sfx_new

play_power_sfx_new:
        ld a, #SFX_POWER
        ld c, #AUDIO_CH_MAJOR
        jp play_sfx_new

play_countdown_sfx_new:
        ret

play_start_sfx_new:
        ret

play_ghost_sfx_new:
        ld a, #SFX_GHOST
        ld c, #AUDIO_CH_MAJOR
        jp play_sfx_new

play_death_sfx_new:
        ld a, #SFX_DEATH
        ld c, #AUDIO_CH_MAJOR
        jp play_sfx_new

play_winner_sfx_new:
        ret

se_init_sfx_entry:
        jp SE_PLAYER_BASE + SE_INIT_SFX_OFS

se_play_sfx_entry:
        jp SE_PLAYER_BASE + SE_PLAY_SFX_OFS

se_stop_sfx_entry:
        jp SE_PLAYER_BASE + SE_STOP_SFX_OFS

se_play_frame_entry:
        jp SE_PLAYER_BASE + SE_PLAY_FRAME_OFS

se_sfx_bank .equ SE_PLAYER_BASE + SE_SFX_BANK_OFS

arkos_se_blob:
        .incbin "src/arkos_se_wrapper.bin"
