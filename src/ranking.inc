; Score comparison and ranking helpers for winner selection and standings.

build_rankings:
        ld a, #0
        ld (RANKS_BASE), a
        ld a, #1
        ld (RANKS_BASE + 1), a
        ld a, #2
        ld (RANKS_BASE + 2), a
        ld a, #3
        ld (RANKS_BASE + 3), a
        ld a, (PLAYER_COUNT_V)
        cp #4
        jr z, build_rankings_4
        cp #3
        jr z, build_rankings_3
        cp #2
        jr z, build_rankings_2
        ret
build_rankings_2:
        ld b, #0
        ld c, #1
        call maybe_swap_bc
        ld a, b
        ld (RANKS_BASE), a
        ld a, c
        ld (RANKS_BASE + 1), a
        ret
build_rankings_3:
        call rank_swap_01
        call rank_swap_12
        call rank_swap_01
        ret
build_rankings_4:
        call rank_swap_01
        call rank_swap_23
        call rank_swap_02
        call rank_swap_13
        call rank_swap_12
        ret

rank_swap_01:
        ld a, (RANKS_BASE)
        ld b, a
        ld a, (RANKS_BASE + 1)
        ld c, a
        call maybe_swap_bc
        ld a, b
        ld (RANKS_BASE), a
        ld a, c
        ld (RANKS_BASE + 1), a
        ret

rank_swap_23:
        ld a, (RANKS_BASE + 2)
        ld b, a
        ld a, (RANKS_BASE + 3)
        ld c, a
        call maybe_swap_bc
        ld a, b
        ld (RANKS_BASE + 2), a
        ld a, c
        ld (RANKS_BASE + 3), a
        ret

rank_swap_02:
        ld a, (RANKS_BASE)
        ld b, a
        ld a, (RANKS_BASE + 2)
        ld c, a
        call maybe_swap_bc
        ld a, b
        ld (RANKS_BASE), a
        ld a, c
        ld (RANKS_BASE + 2), a
        ret

rank_swap_13:
        ld a, (RANKS_BASE + 1)
        ld b, a
        ld a, (RANKS_BASE + 3)
        ld c, a
        call maybe_swap_bc
        ld a, b
        ld (RANKS_BASE + 1), a
        ld a, c
        ld (RANKS_BASE + 3), a
        ret

rank_swap_12:
        ld a, (RANKS_BASE + 1)
        ld b, a
        ld a, (RANKS_BASE + 2)
        ld c, a
        call maybe_swap_bc
        ld a, b
        ld (RANKS_BASE + 1), a
        ld a, c
        ld (RANKS_BASE + 2), a
        ret

maybe_swap_bc:
        push bc
        call compare_scores_bc
        pop bc
        ret nc
        ld a, b
        ld b, c
        ld c, a
        ret

compare_scores_bc:
        ld a, b
        call set_ix_to_lane_a
        ld l, L_SCORE_L(ix)
        ld h, L_SCORE_H(ix)
        push hl
        ld a, c
        call set_ix_to_lane_a
        ld e, L_SCORE_L(ix)
        ld d, L_SCORE_H(ix)
        pop hl
        ld a, d
        cp h
        jr c, compare_scores_no
        jr nz, compare_scores_yes
        ld a, e
        cp l
        jr c, compare_scores_no
        jr z, compare_scores_no
compare_scores_yes:
        scf
        ret
compare_scores_no:
        or a
        ret
