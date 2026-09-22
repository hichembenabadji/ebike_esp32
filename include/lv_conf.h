/**
 * @file lv_conf.h
 * LVGL 8.x configuration for the e-bike dashboard (Guition JC4827W543).
 *
 * Only the options that differ from the defaults are listed here; everything
 * else comes from lvgl/src/lv_conf_internal.h.
 * This file is found through -D LV_CONF_INCLUDE_SIMPLE and -I include.
 */
#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/*====================
   COLORS
 *====================*/
#define LV_COLOR_DEPTH 16

/* 1 = RGB565 pixels are stored in the byte order of the display bus, so the
 * LVGL draw buffer is sent as-is (no CPU conversion) through
 * gfx->draw16bitBeRGBBitmap() in main.cpp. */
#define LV_COLOR_16_SWAP 1

#define LV_COLOR_SCREEN_TRANSP 0
#define LV_COLOR_CHROMA_KEY lv_color_hex(0x00ff00)

/*=========================
   MEMORY
 *=========================*/
/* LVGL internal pool (TLSF). Allocated in PSRAM when available, with an
 * automatic fallback to internal RAM when PSRAM is missing. */
#define LV_MEM_CUSTOM 0
#define LV_MEM_SIZE (128U * 1024U)
#define LV_MEM_ADR 0
#define LV_MEM_POOL_INCLUDE <esp_heap_caps.h>
#define LV_MEM_POOL_ALLOC(size) heap_caps_malloc_prefer((size), 2, \
        MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)
#define LV_MEM_BUF_MAX_NUM 16
#define LV_MEMCPY_MEMSET_STD 0

/*====================
   HAL
 *====================*/
#define LV_DISP_DEF_REFR_PERIOD 20     /* ms between two screen refreshes */
#define LV_INDEV_DEF_READ_PERIOD 30    /* ms between two touch reads */

/* LVGL tick driven by millis(): no need to call lv_tick_inc() */
#define LV_TICK_CUSTOM 1
#define LV_TICK_CUSTOM_INCLUDE "Arduino.h"
#define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis())

#define LV_DPI_DEF 116                 /* 4.3" 480x272 is about 116 dpi */

/*=======================
   FEATURES
 *=======================*/
#define LV_DRAW_COMPLEX 1
#define LV_SHADOW_CACHE_SIZE 0
#define LV_CIRCLE_CACHE_SIZE 4
#define LV_LAYER_SIMPLE_BUF_SIZE (24 * 1024)

#define LV_USE_LOG 0
#define LV_USE_ASSERT_NULL 1
#define LV_USE_ASSERT_MALLOC 1
#define LV_USE_ASSERT_STYLE 0
#define LV_USE_ASSERT_MEM_INTEGRITY 0
#define LV_USE_ASSERT_OBJ 0

/* Set to 1 to show the FPS / CPU usage overlay in the bottom-right corner */
#define LV_USE_PERF_MONITOR 0
#define LV_USE_MEM_MONITOR 0

#define LV_SPRINTF_CUSTOM 0
#define LV_SPRINTF_USE_FLOAT 0

/*==================
   FONTS
 *==================*/
#define LV_FONT_MONTSERRAT_12 0
#define LV_FONT_MONTSERRAT_14 1   /* LVGL default font (fallback) */
#define LV_FONT_MONTSERRAT_16 0
#define LV_FONT_MONTSERRAT_20 0
#define LV_FONT_MONTSERRAT_28 0
#define LV_FONT_MONTSERRAT_48 0

#define LV_FONT_DEFAULT &lv_font_montserrat_14
#define LV_FONT_FMT_TXT_LARGE 0
#define LV_USE_FONT_COMPRESSED 0
#define LV_USE_FONT_SUBPX 0

/* Dashboard fonts generated with lv_font_conv into src/fonts/
 * (Space Grotesk Bold/Medium, Space Mono, FontAwesome 5 icons). */
#define LV_FONT_CUSTOM_DECLARE \
    LV_FONT_DECLARE(cb_speed_72)  \
    LV_FONT_DECLARE(cb_bold_34)   \
    LV_FONT_DECLARE(cb_bold_20)   \
    LV_FONT_DECLARE(cb_bold_16)   \
    LV_FONT_DECLARE(cb_medium_12) \
    LV_FONT_DECLARE(cb_mono_12)   \
    LV_FONT_DECLARE(cb_mono_20)

/*=================
   TEXT
 *=================*/
#define LV_TXT_ENC LV_TXT_ENC_UTF8
#define LV_USE_BIDI 0
#define LV_USE_ARABIC_PERSIAN_CHARS 0

/*==================
   WIDGETS
 *==================*/
#define LV_USE_ARC        1
#define LV_USE_BAR        1
#define LV_USE_BTN        1
#define LV_USE_BTNMATRIX  0
#define LV_USE_CANVAS     0
#define LV_USE_CHECKBOX   0
#define LV_USE_DROPDOWN   0
#define LV_USE_IMG        1
#define LV_USE_LABEL      1
#define LV_LABEL_TEXT_SELECTION 0
#define LV_LABEL_LONG_TXT_HINT 1
#define LV_USE_LINE       1
#define LV_USE_ROLLER     0
#define LV_USE_SLIDER     0
#define LV_USE_SWITCH     0
#define LV_USE_TEXTAREA   0
#define LV_USE_TABLE      0

/* "Extra" widgets */
#define LV_USE_ANIMIMG    0
#define LV_USE_CALENDAR   0
#define LV_USE_CHART      0
#define LV_USE_COLORWHEEL 0
#define LV_USE_IMGBTN     0
#define LV_USE_KEYBOARD   0
#define LV_USE_LED        1
#define LV_USE_LIST       0
#define LV_USE_MENU       0
#define LV_USE_METER      0
#define LV_USE_MSGBOX     0
#define LV_USE_SPAN       0
#define LV_USE_SPINBOX    0
#define LV_USE_SPINNER    0
#define LV_USE_TABVIEW    0
#define LV_USE_TILEVIEW   0
#define LV_USE_WIN        0

/*==================
   THEMES
 *==================*/
#define LV_USE_THEME_DEFAULT 1
#define LV_THEME_DEFAULT_DARK 1
#define LV_THEME_DEFAULT_GROW 0
#define LV_THEME_DEFAULT_TRANSITION_TIME 80
#define LV_USE_THEME_BASIC 0
#define LV_USE_THEME_MONO 0

/*==================
   LAYOUTS
 *==================*/
#define LV_USE_FLEX 1
#define LV_USE_GRID 1

/*==================
   MISC
 *==================*/
#define LV_USE_SNAPSHOT 0
#define LV_USE_MONKEY 0
#define LV_USE_GRIDNAV 0
#define LV_USE_FRAGMENT 0
#define LV_USE_IMGFONT 0
#define LV_USE_MSG 0
#define LV_USE_IME_PINYIN 0
#define LV_BUILD_EXAMPLES 0
#define LV_USE_DEMO_WIDGETS 0
#define LV_USE_DEMO_BENCHMARK 0
#define LV_USE_DEMO_STRESS 0
#define LV_USE_DEMO_MUSIC 0

#endif /* LV_CONF_H */
