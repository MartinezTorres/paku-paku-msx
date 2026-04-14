# PAKU PAKU MSX

An MSX cartridge prototype in Z80 assembly based on ABA Games' `PAKU PAKU`.

This version keeps the core lane mechanics:

- one-button reverse
- player-only wraparound
- 16 dots with 1 power dot
- one ghost per lane with chase, frightened, and eyes-return states
- multiplier-based scoring
- difficulty that ramps over time

It extends the original with configurable player count and time-limit modes.

## Build

```sh
make all
```

This produces [build/paku.rom](/home/claude/repos/paku-paku/build/paku.rom).

## Run

```sh
make run
```

## Controls

Menu:

- `UP` / `DOWN` selects `START`, `NUMBER OF PLAYERS`, or `TIME LIMIT`
- `LEFT` / `RIGHT` / `SPACE` / `RETURN` cycle the current setting
- `SPACE` or `RETURN` on `START` begins the session

Gameplay:

- `1P`: any key or joystick input reverses direction
- `2P-4P`: each player binds one key before the countdown, then uses that key to reverse direction
- `ESC`, then `ESC` again, quits an active session

## Notes

- The current build uses MSX `SCREEN 2` with a custom tile set, dynamic lane tiles for pellets, and hardware sprites for the actors.
- Sound is not implemented yet.
- High score is tracked in RAM for the current session.
- Architecture notes are in [src/ARCHITECTURE.md](/home/claude/repos/paku-paku/src/ARCHITECTURE.md).
