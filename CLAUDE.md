# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this repo is

A ZMK firmware configuration for **nerdboard** — a custom wireless split keyboard using nice_nano (nRF52840) controllers. The right half has a PMW3389 trackball connected via SPI.

## Building firmware

Firmware is built via GitHub Actions. Push to `main` (or open a PR) to trigger a build. The workflow uses ZMK's official reusable workflow at `zmkfirmware/zmk/.github/workflows/build-user-config.yml@v0.3`. Artifacts (`.uf2` files) are downloadable from the Actions run.

`build.yaml` defines the CI matrix — currently builds `nerdboard_left` and `nerdboard_right` for the `nice_nano` board.

For local builds, a full ZMK west workspace is required (not included in this repo). See the ZMK docs for environment setup.

## Repository structure

```
config/
  west.yml          # west manifest — pulls ZMK + PMW3389 driver
  nerdboard.conf    # Kconfig (mouse, SPI, PMW3389 enabled)
  nerdboard.keymap  # Keymap (3 layers: default, lower, raise)

boards/shields/nerdboard/
  nerdboard.overlay       # Shared: matrix transform, base kscan node, SPI pinctrl
  nerdboard_left.overlay  # Left GPIO pin assignments
  nerdboard_right.overlay # Right GPIO pins + SPI/trackball node
  Kconfig.shield          # Shield selection config

build.yaml          # CI matrix (board + shield combinations)
zephyr/module.yml   # Declares this repo as a Zephyr module (board_root: .)
```

## Key dependencies (west.yml)

- **ZMK**: `zmkfirmware/zmk` @ `main`
- **PMW3389 driver**: `teamspatzenhirn/pmw3389_zephyr_driver` @ `main`

## Hardware layout

- 8 columns × 4 rows matrix; 29 keys (24 alpha + 5 thumb keys)
- Split: left and right halves each have 4 rows × 4 columns
- Right half only: PMW3389 trackball on SPI0, CS on Pro Micro pin 10 (B6)
- SPI pins (nerdboard.overlay pinctrl): SCK→P0.20, MOSI→P0.17, MISO→P0.15
- The left half overlay sets the GPIO pins; the right adds `col-offset = <4>` on the transform

## How overlays compose

`nerdboard_left.overlay` and `nerdboard_right.overlay` both `#include "nerdboard.overlay"` and extend the shared nodes. The shared overlay defines the base `kscan0` node with empty GPIO arrays that each side fills in. The right overlay also enables the SPI bus and attaches the trackball.

## Keymap layers

- **Layer 0 (default)**: Dvorak-derived alpha layout
- **Layer 1 (lower)**: Numbers on left, navigation/symbols on right; activated by left thumb `mo 1`
- **Layer 2 (raise)**: Same as lower currently; activated by right thumb `mo 2`
