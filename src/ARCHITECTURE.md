# Architecture

This project is an MSX cartridge in Z80 assembly. The code is split by responsibility rather than by machine feature.

## Module Layout

- `paku.asm`
  - Root file.
  - Defines BIOS constants, VRAM constants, RAM work-area layout, lane struct layout, and include order.
- `startup.inc`
  - Boot path, refresh-rate detection, VRAM asset upload, and main loop.
  - Owns the fixed-timestep scheduler that maps 50 Hz or 60 Hz VBlank to a 60 Hz gameplay simulation.
- `gameflow.inc`
  - Umbrella include for the active game-flow modules below.
- `state_flow.inc`
  - State machine for menu, key binding, countdown, gameplay, winner screen, name entry, and score screen.
  - Also owns high-score insertion and menu idle-to-scores behavior.
- `input_scan.inc`
  - Keyboard matrix scan, joystick scan, pending-key capture, menu action decode, and per-player input bitmask decode.
- `ui_flow.inc`
  - Menu, scores, countdown, winner, and name-entry rendering for the active flow.
- `simulation.inc`
  - Lane initialization, player movement, ghost movement, dot hit detection, power mode, score updates, and difficulty ramp.
- `lane_render.inc`
  - Per-lane playfield rendering, lane-row selection, and dynamic pellet tile composition.
- `ranking.inc`
  - Score comparison and winner/ranking order helpers.
- `render_helpers.inc`
  - Screen-buffer flushing, dynamic tile upload, sprite list assembly, VRAM copy helpers, and shared math/table helpers.
- `data.inc`
  - Static tables and strings.
- `assets.inc`
  - Menu title include plus tile/sprite art.
- `menu_title.inc`
  - Generated menu title bitmap/sprite data from `pakupaku.png`.

## Frame Flow

The frame loop lives in `startup.inc`:

1. Wait for the next `JIFFY` tick.
2. Scan keyboard and joystick input.
3. Run enough 60 Hz simulation steps to catch up with the detected machine refresh.
4. Render the current state into the RAM screen buffer.
5. Flush visible rows, dynamic lane tiles, and sprites to VRAM.

The important design point is that simulation and rendering are separate:

- `state_flow.inc` decides what the game is doing.
- `simulation.inc` updates gameplay state when the current state is `STATE_PLAY`.
- `ui_flow.inc` and `lane_render.inc` convert that state into screen contents.

## RAM Layout

The RAM work area is declared in `paku.asm`. It is intentionally centralized there.

The main groups are:

- Core session state and timers
- Menu/input state
- Name-entry and high-score state
- Shared scratch aliases
- Dynamic tile buffers and sprite staging
- Per-lane structs

If you need a new persistent variable, add it in `paku.asm` near the related subsystem instead of scattering addresses across includes.

## Editing Guidelines

- Change `paku.asm` when you need new constants, RAM variables, or include order changes.
- Change `simulation.inc` for mechanics.
- Change `state_flow.inc` for menus, state transitions, and post-game flow.
- Change `ui_flow.inc` or `lane_render.inc` for what appears on screen.
- Change `render_helpers.inc` only for low-level VRAM/sprite/buffer behavior shared by multiple modules.

## Build Note

`Makefile` now depends on all `src/*.asm` and `src/*.inc` files, so edits to include files correctly trigger a rebuild.
