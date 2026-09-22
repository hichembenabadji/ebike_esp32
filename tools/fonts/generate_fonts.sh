#!/usr/bin/env bash
# Regenerates the dashboard LVGL fonts into src/fonts/.
# Requirements: Node.js (npx) and a project built at least once
# (the FontAwesome file is taken from .pio/libdeps/.../lvgl/scripts/built_in_font/).
set -e
cd "$(dirname "$0")/../.."
FA=$(ls .pio/libdeps/*/lvgl/scripts/built_in_font/FontAwesome5-Solid+Brands+Regular.woff | head -1)
SRC=tools/fonts
OUT=src/fonts
LATIN="0x20-0x7E,0xB0,0xC0-0xFF"
ICO_A="0xF185,0xF3E5,0xF064,0xF071,0xF0A1,0xF06C,0xF206,0xF0E7,0xF058,0xF054"
ICO_B="0xF06C,0xF241,0xF545,0xF2F2,0xF054,0xF0E7,0xF06A,0xF058,0xF00C,0xF206"
gen() { name=$1; size=$2; shift 2
  npx --yes lv_font_conv@1 --bpp 4 --size "$size" --format lvgl --no-compress \
      --lv-include lvgl.h --lv-font-name "$name" -o "$OUT/$name.c" "$@"
  echo "  $name genere"; }
gen cb_speed_72  72 --font $SRC/SpaceGrotesk-Bold.ttf   -r 0x2D,0x30-0x39
gen cb_bold_34   34 --font $SRC/SpaceGrotesk-Bold.ttf   -r $LATIN
gen cb_bold_20   20 --font $SRC/SpaceGrotesk-Bold.ttf   -r $LATIN --font "$FA" -r $ICO_A
gen cb_bold_16   16 --font $SRC/SpaceGrotesk-Bold.ttf   -r $LATIN
gen cb_medium_12 12 --font $SRC/SpaceGrotesk-Medium.ttf -r $LATIN --font "$FA" -r $ICO_B
gen cb_mono_12   12 --font $SRC/SpaceMono-Regular.ttf   -r 0x20-0x7E
gen cb_mono_20   20 --font $SRC/SpaceMono-Bold.ttf      -r 0x20-0x7E
echo "Termine."
