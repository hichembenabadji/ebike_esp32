# Dashboard LVGL fonts

4 bpp bitmap fonts generated with `lv_font_conv` (npm) from:

- **Space Grotesk** Bold / Medium (SIL OFL 1.1, see LICENSE-SpaceGrotesk-OFL.txt)
- **Space Mono** Regular / Bold (SIL OFL 1.1, see LICENSE-SpaceMono-OFL.txt)
- **FontAwesome 5** icons (file `FontAwesome5-Solid+Brands+Regular.woff`
  shipped with LVGL in `lvgl/scripts/built_in_font/`)

| File             | Usage                                     | Glyph range                 |
|------------------|-------------------------------------------|-----------------------------|
| cb_speed_72.c    | speed (digits)                            | `-` and `0-9`               |
| cb_bold_34.c     | assist mode, battery %, trip              | ASCII + Latin-1 (accents)   |
| cb_bold_20.c     | toolbar icons, mode selector titles       | ASCII + Latin-1 + icons     |
| cb_bold_16.c     | KM/H, units, buttons                      | ASCII + Latin-1             |
| cb_medium_12.c   | uppercase captions, icons                 | ASCII + Latin-1 + icons     |
| cb_mono_12.c     | gauge tick labels, odometer               | ASCII                       |
| cb_mono_20.c     | ride time                                 | ASCII                       |

The source TTF files live in `tools/fonts/`, and `tools/fonts/generate_fonts.sh`
regenerates everything. The exact options are recorded at the top of each `.c`
file (`Opts:` line). To regenerate a single font, for example:

```
npx lv_font_conv --bpp 4 --size 72 --format lvgl --no-compress --lv-include lvgl.h \
    --lv-font-name cb_speed_72 --font SpaceGrotesk-Bold.ttf -r 0x2D,0x30-0x39 -o cb_speed_72.c
```

The fonts are declared in `include/lv_conf.h` through `LV_FONT_CUSTOM_DECLARE`.
