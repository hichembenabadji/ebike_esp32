
#include <Arduino.h>
#include <lvgl.h>
#include <esp_heap_caps.h>
#include <math.h>

#include <Arduino_GFX_Library.h>
#include <TAMC_GT911.h>

SET_LOOP_TASK_STACK_SIZE(16 * 1024);

#define JC4827_LCD_CS      45
#define JC4827_LCD_SCK     47
#define JC4827_LCD_D0      21
#define JC4827_LCD_D1      48
#define JC4827_LCD_D2      40
#define JC4827_LCD_D3      39
#define JC4827_LCD_BL       1

#define JC4827_TP_SDA       8
#define JC4827_TP_SCL       4
#define JC4827_TP_INT       3
#define JC4827_TP_RST      38

#define JC4827_LCD_WIDTH  480
#define JC4827_LCD_HEIGHT 272
#define JC4827_LCD_ROTATION 0

#ifndef JC4827_LCD_SPI_FREQ
#define JC4827_LCD_SPI_FREQ 40000000
#endif

#ifndef JC4827_LCD_IPS
#define JC4827_LCD_IPS true
#endif

#ifndef JC4827_TP_FLIP_X
#define JC4827_TP_FLIP_X 0
#endif
#ifndef JC4827_TP_FLIP_Y
#define JC4827_TP_FLIP_Y 0
#endif
#ifndef JC4827_TP_SWAP_XY
#define JC4827_TP_SWAP_XY 0
#endif

static Arduino_DataBus* bus = new Arduino_ESP32QSPI(
    JC4827_LCD_CS , JC4827_LCD_SCK ,
    JC4827_LCD_D0 , JC4827_LCD_D1 , JC4827_LCD_D2 , JC4827_LCD_D3);

static Arduino_GFX* gfx = new Arduino_NV3041A(
    bus, GFX_NOT_DEFINED , JC4827_LCD_ROTATION , JC4827_LCD_IPS );

static TAMC_GT911 touch(JC4827_TP_SDA, JC4827_TP_SCL, JC4827_TP_INT, JC4827_TP_RST,
                        JC4827_LCD_WIDTH, JC4827_LCD_HEIGHT);

#define DEMO_MODE 1
#define USE_MPH   0

static constexpr uint16_t SCREEN_W         = JC4827_LCD_WIDTH;
static constexpr uint16_t SCREEN_H         = JC4827_LCD_HEIGHT;
static constexpr uint16_t LV_BUF_LINES     = 68;
static constexpr uint8_t  BATT_LOW_PCT     = 20;
static constexpr uint8_t  BATT_CRIT_PCT    = 10;

#if USE_MPH
static constexpr float UNIT_FACTOR    = 0.621371f;
static constexpr int   SPEED_MAX      = 40;
static constexpr int   SPEED_RED_FROM = 30;
static constexpr int   SPEED_TICK     = 10;
#define UNIT_SPEED "MPH"
#define UNIT_DIST  "mi"
#define UNIT_AVG   "mph"
#else
static constexpr float UNIT_FACTOR    = 1.0f;
static constexpr int   SPEED_MAX      = 60;
static constexpr int   SPEED_RED_FROM = 45;
static constexpr int   SPEED_TICK     = 10;
#define UNIT_SPEED "KM/H"
#define UNIT_DIST  "km"
#define UNIT_AVG   "km/h"
#endif

static constexpr lv_coord_t M           = 6;
static constexpr lv_coord_t TB_Y        = 6,   TB_H = 56;
static constexpr lv_coord_t TB_BTN_W    = 88,  TB_BTN_H = 46, TB_GAP = 4;
static constexpr lv_coord_t MAIN_Y      = 68,  MAIN_H = 198;
static constexpr lv_coord_t SIDE_W      = 126;
static constexpr lv_coord_t LEFT_X      = 6,   RIGHT_X = 348;
static constexpr lv_coord_t GAUGE_X     = 138, GAUGE_W = 210;
static constexpr lv_coord_t TOP_H       = 94,  BOT_Y = 168, BOT_H = 98;
static constexpr lv_coord_t CARD_RADIUS = 12,  CARD_PAD = 7;
static constexpr lv_coord_t ARC_R       = 80,  ARC_W = 14;
static constexpr lv_coord_t TICK_R_IN   = 62,  TICK_R_OUT = 84, TICK_LBL_R = 92;
static constexpr lv_coord_t GAUGE_CX    = 104, GAUGE_CY = 105;

#define HC_BG        lv_color_hex(0x000000)
#define HC_CARD      lv_color_hex(0x000000)
#define HC_CARD_HI   lv_color_hex(0x333333)
#define HC_BORDER    lv_color_hex(0x4A4A4A)
#define HC_TEXT      lv_color_hex(0xFFE600)
#define HC_LABEL     lv_color_hex(0xFFC800)
#define HC_GREEN     lv_color_hex(0x00E676)
#define HC_ORANGE    lv_color_hex(0xFF8C00)
#define HC_RED       lv_color_hex(0xFF1744)
#define HC_BLUE      lv_color_hex(0x007BFF)
#define HC_TRACK     lv_color_hex(0x2E2E2E)
#define HC_DASH_OFF  lv_color_hex(0x3A3A3A)
#define HC_ROW_SEL   lv_color_hex(0x0B2308)

#define HC_LIGHT_ON  HC_GREEN

#define ICO_SUN       "\xEF\x86\x85"
#define ICO_TURN_L    "\xEF\x8F\xA5"
#define ICO_TURN_R    "\xEF\x81\xA4"
#define ICO_WARNING   "\xEF\x81\xB1"
#define ICO_HORN      "\xEF\x82\xA1"
#define ICO_LEAF      "\xEF\x81\xAC"
#define ICO_BIKE      "\xEF\x88\x86"
#define ICO_BOLT      "\xEF\x83\xA7"
#define ICO_BATTERY   "\xEF\x89\x81"
#define ICO_RULER     "\xEF\x95\x85"
#define ICO_STOPWATCH "\xEF\x8B\xB2"
#define ICO_CHEVRON   "\xEF\x81\x94"

#define F_SPEED   (&cb_speed_72)
#define F_BOLD34  (&cb_bold_34)
#define F_BOLD20  (&cb_bold_20)
#define F_BOLD16  (&cb_bold_16)
#define F_MED12   (&cb_medium_12)
#define F_MONO12  (&cb_mono_12)
#define F_MONO20  (&cb_mono_20)

#define TXT_ASSIST        "ASSIST"
#define TXT_BATTERY       "BATTERY"
#define TXT_STATUS        "STATUS"
#define TXT_STATUS_OK     "OK"
#define TXT_STATUS_LOW    "LOW"
#define TXT_STATUS_CRIT   "CRITICAL"
#define TXT_TRIP          "TRIP"
#define TXT_TIME          "TIME"
#define TXT_AVG           "AVG"
#define TXT_ODO           "ODO"
#define TXT_MODE_TITLE    "ASSIST MODE"
#define TXT_CURRENT       "CURRENT"
#define TXT_CANCEL        "CANCEL"

enum AssistMode : uint8_t { MODE_ECO = 0, MODE_TOUR, MODE_SPORT, MODE_COUNT };

struct ModeInfo {
    const char* name;
    const char* desc;
    const char* icon;
    uint8_t     level;
};
static constexpr uint8_t MODE_LEVEL_MAX = 3;
static const ModeInfo MODES[MODE_COUNT] = {
    { "ECO",   "Maximum range",          ICO_LEAF, 1 },
    { "TOUR",  "Balanced power / range", ICO_BIKE, 2 },
    { "SPORT", "Maximum power",          ICO_BOLT, 3 },
};

enum ToolbarBtn : uint8_t { TB_HEADLIGHT = 0, TB_LEFT, TB_RIGHT, TB_HAZARD, TB_HORN, TB_COUNT };

struct BikeData {
    float      speed_kmh   = 0.0f;
    uint8_t    battery_pct = 100;
    AssistMode mode        = MODE_TOUR;
    float      trip_km     = 0.0f;
    float      odo_km      = 0.0f;
    uint32_t   ride_time_s = 0;
    bool       headlight   = false;
    bool       turn_left   = false;
    bool       turn_right  = false;
    bool       hazard      = false;
    bool       horn        = false;
};
static BikeData g_bike;

static void hw_set_headlight(bool on)
{
    Serial.printf("[HW] Headlight: %s\n", on ? "ON" : "OFF");
}
static void hw_set_turn_signal(bool left, bool right)
{
    Serial.printf("[HW] Turn signals: left=%d right=%d\n", left, right);
}
static void hw_set_hazard(bool on)
{
    Serial.printf("[HW] Hazard lights: %s\n", on ? "ON" : "OFF");
}
static void hw_set_horn(bool on)
{
    Serial.printf("[HW] Horn: %s\n", on ? "ON" : "OFF");
}
static void hw_set_assist_mode(AssistMode m)
{
    Serial.printf("[HW] Assist mode: %s\n", MODES[m].name);
}

static lv_disp_draw_buf_t s_draw_buf;
static lv_disp_drv_t      s_disp_drv;
static lv_indev_drv_t     s_indev_drv;
static lv_color_t*        s_lv_buf = nullptr;

struct UiObjects {
    lv_obj_t* tb_btn[TB_COUNT]  = {};
    lv_obj_t* tb_icon[TB_COUNT] = {};
    lv_obj_t* tb_lbl[TB_COUNT]  = {};

    lv_obj_t* arc_speed = nullptr;
    lv_obj_t* speed_lbl = nullptr;
    lv_obj_t* odo_lbl   = nullptr;

    lv_obj_t* mode_card = nullptr;
    lv_obj_t* mode_lbl  = nullptr;
    lv_obj_t* mode_dash[MODE_LEVEL_MAX] = {};

    lv_obj_t* batt_lbl    = nullptr;
    lv_obj_t* batt_bar    = nullptr;
    lv_obj_t* batt_status = nullptr;

    lv_obj_t* trip_lbl = nullptr;
    lv_obj_t* time_lbl = nullptr;
    lv_obj_t* avg_lbl  = nullptr;

    lv_obj_t* overlay = nullptr;
    lv_obj_t* row[MODE_COUNT]     = {};
    lv_obj_t* row_tag[MODE_COUNT] = {};
};
static UiObjects ui;

static lv_point_t s_tick_pts[SPEED_MAX / SPEED_TICK + 1][2];

static void lvgl_flush_cb(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* px)
{
    const int16_t w = lv_area_get_width(area);
    const int16_t h = lv_area_get_height(area);

#if LV_COLOR_16_SWAP
    gfx->draw16bitBeRGBBitmap(area->x1, area->y1, reinterpret_cast<uint16_t*>(&px->full), w, h);
#else
    gfx->draw16bitRGBBitmap(area->x1, area->y1, reinterpret_cast<uint16_t*>(&px->full), w, h);
#endif

    lv_disp_flush_ready(drv);
}

static void lvgl_touch_cb(lv_indev_drv_t*, lv_indev_data_t* data)
{
    static bool was_pressed = false;

    touch.read();
    if (!touch.isTouched) {
        data->state = LV_INDEV_STATE_RELEASED;
        was_pressed = false;
        return;
    }

    int32_t x = touch.points[0].x;
    int32_t y = touch.points[0].y;
    const int32_t raw_x = x, raw_y = y;
#if JC4827_TP_SWAP_XY
    { const int32_t t = x; x = y; y = t; }
#endif
#if JC4827_TP_FLIP_X
    x = (SCREEN_W - 1) - x;
#endif
#if JC4827_TP_FLIP_Y
    y = (SCREEN_H - 1) - y;
#endif
    if (x < 0) x = 0;  if (x >= SCREEN_W) x = SCREEN_W - 1;
    if (y < 0) y = 0;  if (y >= SCREEN_H) y = SCREEN_H - 1;

    if (!was_pressed) {
        Serial.printf("[TOUCH] raw=(%d,%d) -> screen=(%d,%d)\n", (int)raw_x, (int)raw_y, (int)x, (int)y);
        was_pressed = true;
    }
    data->state   = LV_INDEV_STATE_PRESSED;
    data->point.x = x;
    data->point.y = y;
}

static lv_obj_t* label_create(lv_obj_t* parent, const char* txt, const lv_font_t* font,
                              lv_color_t color, lv_coord_t letter_space = 0)
{
    lv_obj_t* l = lv_label_create(parent);
    lv_label_set_text(l, txt);
    lv_obj_set_style_text_font(l, font, 0);
    lv_obj_set_style_text_color(l, color, 0);
    if (letter_space) lv_obj_set_style_text_letter_space(l, letter_space, 0);
    return l;
}

static lv_obj_t* box_create(lv_obj_t* parent, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h)
{
    lv_obj_t* o = lv_obj_create(parent);
    lv_obj_remove_style_all(o);
    lv_obj_set_pos(o, x, y);
    lv_obj_set_size(o, w, h);
    lv_obj_clear_flag(o, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    return o;
}

static lv_obj_t* card_create(lv_obj_t* parent, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h)
{
    lv_obj_t* c = box_create(parent, x, y, w, h);
    lv_obj_set_style_bg_color(c, HC_CARD, 0);
    lv_obj_set_style_bg_opa(c, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(c, CARD_RADIUS, 0);
    lv_obj_set_style_border_color(c, HC_BORDER, 0);
    lv_obj_set_style_border_width(c, 1, 0);
    lv_obj_set_style_pad_all(c, CARD_PAD, 0);
    return c;
}

static void caption_create(lv_obj_t* parent, const char* icon, const char* text)
{
    lv_obj_t* i = label_create(parent, icon, F_MED12, HC_LABEL);
    lv_obj_set_pos(i, 0, 0);
    lv_obj_t* t = label_create(parent, text, F_MED12, HC_LABEL, 1);
    lv_obj_set_pos(t, 18, 0);
}

static lv_obj_t* value_row_create(lv_obj_t* parent, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h)
{
    lv_obj_t* r = box_create(parent, x, y, w, h);
    lv_obj_set_flex_flow(r, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(r, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END);
    lv_obj_set_style_pad_column(r, 4, 0);
    return r;
}

static void dashes_create(lv_obj_t* parent, lv_obj_t** out, uint8_t count,
                          lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h, lv_coord_t gap)
{
    for (uint8_t k = 0; k < count; ++k) {
        lv_obj_t* d = box_create(parent, x + k * (w + gap), y, w, h);
        lv_obj_set_style_bg_color(d, HC_DASH_OFF, 0);
        lv_obj_set_style_bg_opa(d, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(d, h / 2, 0);
        out[k] = d;
    }
}

static void dashes_set_level(lv_obj_t** dashes, uint8_t count, uint8_t level)
{
    for (uint8_t k = 0; k < count; ++k) {
        lv_obj_set_style_bg_color(dashes[k], (k < level) ? HC_GREEN : HC_DASH_OFF, 0);
    }
}

static void toolbar_event_cb(lv_event_t* e);
static void assist_card_event_cb(lv_event_t* e);
static void overlay_close_event_cb(lv_event_t* e);
static void overlay_row_event_cb(lv_event_t* e);
static void toolbar_refresh(void);
static void overlay_refresh(void);

struct TbDef { const char* icon; const char* label; bool checkable; };
static const TbDef TB_DEFS[TB_COUNT] = {
    { ICO_SUN,     "LIGHT",   true  },
    { ICO_TURN_L,  "LEFT",    true  },
    { ICO_TURN_R,  "RIGHT",   true  },
    { ICO_WARNING, "WARNING", true  },
    { ICO_HORN,    "HORN",    false },
};

static void ui_build_toolbar(lv_obj_t* scr)
{
    lv_obj_t* bar = card_create(scr, M, TB_Y, SCREEN_W - 2 * M, TB_H);
    lv_obj_set_style_pad_all(bar, 4, 0);

    for (int i = 0; i < TB_COUNT; ++i) {
        lv_obj_t* b = box_create(bar, i * (TB_BTN_W + TB_GAP), 0, TB_BTN_W, TB_BTN_H);
        lv_obj_add_flag(b, LV_OBJ_FLAG_CLICKABLE);
        if (TB_DEFS[i].checkable) lv_obj_add_flag(b, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_set_style_radius(b, 10, 0);
        lv_obj_set_style_bg_color(b, HC_CARD_HI, LV_STATE_PRESSED);
        lv_obj_set_style_bg_opa(b, LV_OPA_COVER, LV_STATE_PRESSED);
        lv_obj_add_event_cb(b, toolbar_event_cb, LV_EVENT_ALL, (void*)(intptr_t)i);

        ui.tb_icon[i] = label_create(b, TB_DEFS[i].icon, F_BOLD20, HC_LABEL);
        lv_obj_align(ui.tb_icon[i], LV_ALIGN_TOP_MID, 0, 3);
        ui.tb_lbl[i] = label_create(b, TB_DEFS[i].label, F_MED12, HC_LABEL, 1);
        lv_obj_align(ui.tb_lbl[i], LV_ALIGN_BOTTOM_MID, 0, -4);
        ui.tb_btn[i] = b;
    }
}

static void toolbar_set_color(int i, lv_color_t c)
{
    lv_obj_set_style_text_color(ui.tb_icon[i], c, 0);
    lv_obj_set_style_text_color(ui.tb_lbl[i], c, 0);
}

static void toolbar_refresh(void)
{
    toolbar_set_color(TB_HEADLIGHT, g_bike.headlight ? HC_LIGHT_ON : HC_LABEL);
    toolbar_set_color(TB_LEFT,   (g_bike.turn_left  || g_bike.hazard) ? HC_ORANGE : HC_LABEL);
    toolbar_set_color(TB_RIGHT,  (g_bike.turn_right || g_bike.hazard) ? HC_ORANGE : HC_LABEL);
    toolbar_set_color(TB_HAZARD, g_bike.hazard ? HC_ORANGE : HC_LABEL);
    toolbar_set_color(TB_HORN,   g_bike.horn   ? HC_TEXT   : HC_LABEL);
}

static void toolbar_event_cb(lv_event_t* e)
{
    lv_obj_t*       b    = lv_event_get_target(e);
    const int       idx  = (int)(intptr_t)lv_event_get_user_data(e);
    lv_event_code_t code = lv_event_get_code(e);

    if (idx == TB_HORN) {
        if (code == LV_EVENT_PRESSED) {
            g_bike.horn = true;  hw_set_horn(true);  toolbar_refresh();
        } else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
            g_bike.horn = false; hw_set_horn(false); toolbar_refresh();
        }
        return;
    }
    if (code != LV_EVENT_VALUE_CHANGED) return;

    const bool on = lv_obj_has_state(b, LV_STATE_CHECKED);
    switch (idx) {
        case TB_HEADLIGHT:
            g_bike.headlight = on;
            hw_set_headlight(on);
            break;
        case TB_LEFT:
            g_bike.turn_left = on;
            if (on) { g_bike.turn_right = false; lv_obj_clear_state(ui.tb_btn[TB_RIGHT], LV_STATE_CHECKED); }
            hw_set_turn_signal(g_bike.turn_left, g_bike.turn_right);
            break;
        case TB_RIGHT:
            g_bike.turn_right = on;
            if (on) { g_bike.turn_left = false; lv_obj_clear_state(ui.tb_btn[TB_LEFT], LV_STATE_CHECKED); }
            hw_set_turn_signal(g_bike.turn_left, g_bike.turn_right);
            break;
        case TB_HAZARD:
            g_bike.hazard = on;
            hw_set_hazard(on);
            break;
        default:
            break;
    }
    toolbar_refresh();
}

static void icon_blink(int i, bool blinking, bool phase)
{
    if (blinking && !phase) lv_obj_add_flag(ui.tb_icon[i], LV_OBJ_FLAG_HIDDEN);
    else                    lv_obj_clear_flag(ui.tb_icon[i], LV_OBJ_FLAG_HIDDEN);
}

static void blink_timer_cb(lv_timer_t* /*t*/)
{
    static bool phase = false;
    phase = !phase;
    icon_blink(TB_LEFT,   g_bike.turn_left  || g_bike.hazard, phase);
    icon_blink(TB_RIGHT,  g_bike.turn_right || g_bike.hazard, phase);
    icon_blink(TB_HAZARD, g_bike.hazard, phase);
}

static lv_obj_t* arc_create(lv_obj_t* parent)
{
    lv_obj_t* a = lv_arc_create(parent);
    lv_obj_remove_style(a, nullptr, LV_PART_KNOB);
    lv_obj_clear_flag(a, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(a, 2 * ARC_R, 2 * ARC_R);
    lv_obj_set_pos(a, GAUGE_CX - ARC_R, GAUGE_CY - ARC_R);
    lv_obj_set_style_pad_all(a, 0, 0);
    lv_arc_set_rotation(a, 0);
    lv_arc_set_bg_angles(a, 135, 45);
    lv_obj_set_style_arc_width(a, ARC_W, LV_PART_MAIN);
    lv_obj_set_style_arc_rounded(a, true, LV_PART_MAIN);
    lv_obj_set_style_arc_width(a, ARC_W, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(a, true, LV_PART_INDICATOR);
    return a;
}

static void ui_build_gauge(lv_obj_t* scr)
{
    lv_obj_t* card = card_create(scr, GAUGE_X, MAIN_Y, GAUGE_W, MAIN_H);
    lv_obj_set_style_pad_all(card, 0, 0);
    const lv_coord_t content_h = MAIN_H - 2;
    const lv_coord_t mid_y     = content_h / 2;

    lv_obj_t* track = arc_create(card);
    lv_obj_set_style_arc_color(track, HC_TRACK, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(track, LV_OPA_TRANSP, LV_PART_INDICATOR);

    lv_obj_t* red = arc_create(card);
    const uint16_t red_start = (uint16_t)(135 + lroundf(270.0f * SPEED_RED_FROM / SPEED_MAX)) % 360;
    lv_arc_set_bg_angles(red, red_start, 45);
    lv_obj_set_style_arc_color(red, HC_RED, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(red, LV_OPA_TRANSP, LV_PART_INDICATOR);

    ui.arc_speed = arc_create(card);
    lv_obj_set_style_arc_opa(ui.arc_speed, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_color(ui.arc_speed, HC_GREEN, LV_PART_INDICATOR);
    lv_arc_set_range(ui.arc_speed, 0, SPEED_MAX);
    lv_arc_set_value(ui.arc_speed, 0);

    const int n = SPEED_MAX / SPEED_TICK + 1;
    for (int i = 0; i < n; ++i) {
        const int   v   = i * SPEED_TICK;
        const float ang = (135.0f + 270.0f * v / SPEED_MAX) * (float)M_PI / 180.0f;
        const float c = cosf(ang), s = sinf(ang);

        s_tick_pts[i][0].x = (lv_coord_t)lroundf(GAUGE_CX + TICK_R_IN  * c);
        s_tick_pts[i][0].y = (lv_coord_t)lroundf(GAUGE_CY + TICK_R_IN  * s);
        s_tick_pts[i][1].x = (lv_coord_t)lroundf(GAUGE_CX + TICK_R_OUT * c);
        s_tick_pts[i][1].y = (lv_coord_t)lroundf(GAUGE_CY + TICK_R_OUT * s);

        lv_obj_t* line = lv_line_create(card);
        lv_line_set_points(line, s_tick_pts[i], 2);
        lv_obj_clear_flag(line, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_line_width(line, 2, 0);
        lv_obj_set_style_line_color(line, HC_LABEL, 0);
        lv_obj_set_style_line_rounded(line, false, 0);

        lv_obj_t* lbl = label_create(card, "", F_MONO12, (v >= SPEED_RED_FROM) ? HC_RED : HC_LABEL);
        lv_label_set_text_fmt(lbl, "%d", v);
        lv_obj_set_size(lbl, 30, 18);
        lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_pos(lbl, (lv_coord_t)lroundf(GAUGE_CX + TICK_LBL_R * c) - 15,
                            (lv_coord_t)lroundf(GAUGE_CY + TICK_LBL_R * s) - 9);
    }

    ui.speed_lbl = label_create(card, "0", F_SPEED, HC_TEXT);
    lv_obj_align(ui.speed_lbl, LV_ALIGN_CENTER, 0, (GAUGE_CY - 16) - mid_y);

    lv_obj_t* unit = label_create(card, UNIT_SPEED, F_BOLD16, HC_LABEL, 2);
    lv_obj_align(unit, LV_ALIGN_CENTER, 0, (GAUGE_CY + 19) - mid_y);

    ui.odo_lbl = label_create(card, TXT_ODO " 0 " UNIT_DIST, F_MONO12, HC_LABEL);
    lv_obj_align(ui.odo_lbl, LV_ALIGN_CENTER, 0, (GAUGE_CY + 37) - mid_y);
}

static void ui_build_assist(lv_obj_t* scr)
{
    lv_obj_t* card = card_create(scr, LEFT_X, MAIN_Y, SIDE_W, TOP_H);
    lv_obj_add_flag(card, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(card, HC_CARD_HI, LV_STATE_PRESSED);
    lv_obj_add_event_cb(card, assist_card_event_cb, LV_EVENT_CLICKED, nullptr);
    ui.mode_card = card;

    caption_create(card, ICO_LEAF, TXT_ASSIST);
    lv_obj_t* chev = label_create(card, ICO_CHEVRON, F_MED12, HC_LABEL);
    lv_obj_align(chev, LV_ALIGN_TOP_RIGHT, 0, 0);

    ui.mode_lbl = label_create(card, "---", F_BOLD34, HC_TEXT);
    lv_obj_set_pos(ui.mode_lbl, 0, 16);

    dashes_create(card, ui.mode_dash, MODE_LEVEL_MAX, 0, 66, 22, 4, 5);
}

static void ui_build_battery(lv_obj_t* scr)
{
    lv_obj_t* card = card_create(scr, LEFT_X, BOT_Y, SIDE_W, BOT_H);
    caption_create(card, ICO_BATTERY, TXT_BATTERY);

    lv_obj_t* row = value_row_create(card, 0, 14, 110, 44);
    ui.batt_lbl = label_create(row, "--", F_BOLD34, HC_TEXT);
    lv_obj_t* pct = label_create(row, "%", F_BOLD16, HC_LABEL);
    lv_obj_set_style_translate_y(pct, -6, 0);

    ui.batt_bar = lv_bar_create(card);
    lv_obj_remove_style_all(ui.batt_bar);
    lv_obj_set_pos(ui.batt_bar, 0, 59);
    lv_obj_set_size(ui.batt_bar, 110, 6);
    lv_bar_set_range(ui.batt_bar, 0, 100);
    lv_bar_set_value(ui.batt_bar, 0, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(ui.batt_bar, HC_TRACK, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ui.batt_bar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(ui.batt_bar, 3, LV_PART_MAIN);
    lv_obj_set_style_bg_color(ui.batt_bar, HC_GREEN, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(ui.batt_bar, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_radius(ui.batt_bar, 3, LV_PART_INDICATOR);
    lv_obj_set_style_anim_time(ui.batt_bar, 300, 0);

    lv_obj_t* st = label_create(card, TXT_STATUS, F_MED12, HC_LABEL, 1);
    lv_obj_align(st, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    ui.batt_status = label_create(card, TXT_STATUS_OK, F_MED12, HC_GREEN, 1);
    lv_obj_align(ui.batt_status, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
}

static void ui_build_trip(lv_obj_t* scr)
{
    lv_obj_t* card = card_create(scr, RIGHT_X, MAIN_Y, SIDE_W, TOP_H);
    caption_create(card, ICO_RULER, TXT_TRIP);

    lv_obj_t* row = value_row_create(card, 0, 22, 110, 44);
    ui.trip_lbl = label_create(row, "0.0", F_BOLD34, HC_TEXT);
    lv_obj_t* unit = label_create(row, UNIT_DIST, F_BOLD16, HC_LABEL);
    lv_obj_set_style_translate_y(unit, -6, 0);
}

static void ui_build_time(lv_obj_t* scr)
{
    lv_obj_t* card = card_create(scr, RIGHT_X, BOT_Y, SIDE_W, BOT_H);
    caption_create(card, ICO_STOPWATCH, TXT_TIME);

    ui.time_lbl = label_create(card, "00:00:00", F_MONO20, HC_TEXT);
    lv_obj_set_pos(ui.time_lbl, 0, 18);

    lv_obj_t* avg = label_create(card, TXT_AVG, F_MED12, HC_LABEL, 1);
    lv_obj_align(avg, LV_ALIGN_BOTTOM_LEFT, 0, -2);
    ui.avg_lbl = label_create(card, "-- " UNIT_AVG, F_BOLD16, HC_TEXT);
    lv_obj_align(ui.avg_lbl, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
}

static void ui_build_overlay(lv_obj_t* scr)
{
    lv_obj_t* ov = box_create(scr, 0, 0, SCREEN_W, SCREEN_H);
    lv_obj_add_flag(ov, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(ov, HC_BG, 0);
    lv_obj_set_style_bg_opa(ov, LV_OPA_COVER, 0);
    lv_obj_add_event_cb(ov, overlay_close_event_cb, LV_EVENT_CLICKED, nullptr);
    ui.overlay = ov;

    const lv_coord_t SHEET_W = 300, SHEET_H = 256, ROW_W = 282, ROW_H = 52, ROW_GAP = 6;
    lv_obj_t* sheet = box_create(ov, (SCREEN_W - SHEET_W) / 2, 8, SHEET_W, SHEET_H);
    lv_obj_add_flag(sheet, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(sheet, HC_CARD, 0);
    lv_obj_set_style_bg_opa(sheet, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(sheet, 16, 0);
    lv_obj_set_style_border_color(sheet, HC_LABEL, 0);
    lv_obj_set_style_border_width(sheet, 1, 0);
    lv_obj_set_style_pad_all(sheet, 8, 0);

    lv_obj_t* title = label_create(sheet, TXT_MODE_TITLE, F_BOLD16, HC_TEXT, 1);
    lv_obj_set_pos(title, 0, 0);

    for (int i = 0; i < MODE_COUNT; ++i) {
        lv_obj_t* row = box_create(sheet, 0, 24 + i * (ROW_H + ROW_GAP), ROW_W, ROW_H);
        lv_obj_add_flag(row, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_color(row, HC_CARD, 0);
        lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
        lv_obj_set_style_bg_color(row, HC_CARD_HI, LV_STATE_PRESSED);
        lv_obj_set_style_radius(row, 10, 0);
        lv_obj_set_style_border_color(row, HC_BORDER, 0);
        lv_obj_set_style_border_width(row, 1, 0);
        lv_obj_add_event_cb(row, overlay_row_event_cb, LV_EVENT_CLICKED, (void*)(intptr_t)i);
        ui.row[i] = row;

        lv_obj_t* icon = label_create(row, MODES[i].icon, F_BOLD20, HC_LABEL);
        lv_obj_align(icon, LV_ALIGN_LEFT_MID, 10, 0);
        lv_obj_t* name = label_create(row, MODES[i].name, F_BOLD20, HC_TEXT);
        lv_obj_set_pos(name, 44, 5);
        lv_obj_t* desc = label_create(row, MODES[i].desc, F_MED12, HC_LABEL);
        lv_obj_set_pos(desc, 44, 31);

        ui.row_tag[i] = label_create(row, TXT_CURRENT, F_MED12, HC_GREEN, 1);
        lv_obj_align(ui.row_tag[i], LV_ALIGN_RIGHT_MID, -10, -11);

        lv_obj_t* dashes[MODE_LEVEL_MAX];
        lv_obj_t* dcont = box_create(row, 0, 0, MODE_LEVEL_MAX * 14 + (MODE_LEVEL_MAX - 1) * 4, 4);
        lv_obj_align(dcont, LV_ALIGN_RIGHT_MID, -10, 10);
        dashes_create(dcont, dashes, MODE_LEVEL_MAX, 0, 0, 14, 4, 4);
        dashes_set_level(dashes, MODE_LEVEL_MAX, MODES[i].level);
    }

    lv_obj_t* cancel = box_create(sheet, 0, 24 + MODE_COUNT * (ROW_H + ROW_GAP), ROW_W, 40);
    lv_obj_add_flag(cancel, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_radius(cancel, 10, 0);
    lv_obj_set_style_border_color(cancel, HC_LABEL, 0);
    lv_obj_set_style_border_width(cancel, 1, 0);
    lv_obj_set_style_bg_color(cancel, HC_CARD_HI, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cancel, LV_OPA_COVER, LV_STATE_PRESSED);
    lv_obj_add_event_cb(cancel, overlay_close_event_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_t* cl = label_create(cancel, TXT_CANCEL, F_BOLD16, HC_TEXT, 2);
    lv_obj_center(cl);
}

static void overlay_refresh(void)
{
    for (int i = 0; i < MODE_COUNT; ++i) {
        const bool sel = (i == g_bike.mode);
        lv_obj_set_style_border_color(ui.row[i], sel ? HC_GREEN : HC_BORDER, 0);
        lv_obj_set_style_border_width(ui.row[i], sel ? 2 : 1, 0);
        lv_obj_set_style_bg_color(ui.row[i], sel ? HC_ROW_SEL : HC_CARD, 0);
        if (sel) lv_obj_clear_flag(ui.row_tag[i], LV_OBJ_FLAG_HIDDEN);
        else     lv_obj_add_flag(ui.row_tag[i], LV_OBJ_FLAG_HIDDEN);
    }
}

static void overlay_show(void)
{
    overlay_refresh();
    lv_obj_move_foreground(ui.overlay);
    lv_obj_clear_flag(ui.overlay, LV_OBJ_FLAG_HIDDEN);
}

static void overlay_hide(void)
{
    lv_obj_add_flag(ui.overlay, LV_OBJ_FLAG_HIDDEN);
}

static bool overlay_is_open(void)
{
    return !lv_obj_has_flag(ui.overlay, LV_OBJ_FLAG_HIDDEN);
}

static void ui_create(void)
{
    lv_obj_t* scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, HC_BG, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    ui_build_toolbar(scr);
    ui_build_assist(scr);
    ui_build_battery(scr);
    ui_build_gauge(scr);
    ui_build_trip(scr);
    ui_build_time(scr);
    ui_build_overlay(scr);

    lv_timer_create(blink_timer_cb, 350, nullptr);
}

static void ui_set_speed(float kmh)
{
    int v = (int)lroundf(kmh * UNIT_FACTOR);
    if (v < 0) v = 0;
    if (v > SPEED_MAX) v = SPEED_MAX;

    static int last = -1;
    if (v == last) return;
    last = v;

    lv_arc_set_value(ui.arc_speed, v);
    lv_label_set_text_fmt(ui.speed_lbl, "%d", v);
}

static void ui_set_battery(uint8_t pct)
{
    if (pct > 100) pct = 100;
    const lv_color_t c   = (pct > BATT_LOW_PCT) ? HC_GREEN : (pct > BATT_CRIT_PCT) ? HC_ORANGE : HC_RED;
    const char*      txt = (pct > BATT_LOW_PCT) ? TXT_STATUS_OK : (pct > BATT_CRIT_PCT) ? TXT_STATUS_LOW : TXT_STATUS_CRIT;

    lv_bar_set_value(ui.batt_bar, pct, LV_ANIM_ON);
    lv_obj_set_style_bg_color(ui.batt_bar, c, LV_PART_INDICATOR);
    lv_label_set_text_fmt(ui.batt_lbl, "%d", pct);
    lv_label_set_text(ui.batt_status, txt);
    lv_obj_set_style_text_color(ui.batt_status, c, 0);
}

static void ui_set_mode(AssistMode m)
{
    if (m >= MODE_COUNT) m = MODE_ECO;
    lv_label_set_text(ui.mode_lbl, MODES[m].name);
    dashes_set_level(ui.mode_dash, MODE_LEVEL_MAX, MODES[m].level);
    overlay_refresh();
}

static void ui_set_trip(float km)
{
    if (km < 0) km = 0;
    const int tenths = (int)lroundf(km * UNIT_FACTOR * 10.0f);
    static int last = -1;
    if (tenths == last) return;
    last = tenths;
    lv_label_set_text_fmt(ui.trip_lbl, "%d.%d", tenths / 10, tenths % 10);
}

static void ui_set_odo(float km)
{
    const int whole = (km < 0) ? 0 : (int)(km * UNIT_FACTOR);
    static int last = -1;
    if (whole == last) return;
    last = whole;
    lv_label_set_text_fmt(ui.odo_lbl, TXT_ODO " %d " UNIT_DIST, whole);
}

static void ui_set_ride_time(uint32_t s)
{
    lv_label_set_text_fmt(ui.time_lbl, "%02u:%02u:%02u",
                          (unsigned)(s / 3600), (unsigned)((s / 60) % 60), (unsigned)(s % 60));
}

static void ui_update_average(void)
{
    if (g_bike.ride_time_s >= 5 && g_bike.trip_km > 0.0f) {
        const int avg = (int)lroundf(g_bike.trip_km * UNIT_FACTOR / (g_bike.ride_time_s / 3600.0f));
        lv_label_set_text_fmt(ui.avg_lbl, "%d " UNIT_AVG, avg);
    } else {
        lv_label_set_text(ui.avg_lbl, "-- " UNIT_AVG);
    }
}

static void ui_apply(const BikeData& d)
{
    ui_set_speed(d.speed_kmh);
    ui_set_battery(d.battery_pct);
    ui_set_mode(d.mode);
    ui_set_trip(d.trip_km);
    ui_set_odo(d.odo_km);
    ui_set_ride_time(d.ride_time_s);
    ui_update_average();
    toolbar_refresh();
}

void bike_set_speed(float kmh)        { g_bike.speed_kmh = kmh;     ui_set_speed(kmh); }
void bike_set_battery(uint8_t pct)    { g_bike.battery_pct = pct;   ui_set_battery(pct); }
void bike_set_trip(float km)          { g_bike.trip_km = km;        ui_set_trip(km); }
void bike_set_odo(float km)           { g_bike.odo_km = km;         ui_set_odo(km); }
void bike_set_mode(AssistMode m)
{
    if (m >= MODE_COUNT) m = MODE_ECO;
    if (m != g_bike.mode) hw_set_assist_mode(m);
    g_bike.mode = m;
    ui_set_mode(m);
}
void bike_next_mode(void)             { bike_set_mode((AssistMode)((g_bike.mode + 1) % MODE_COUNT)); }

static void assist_card_event_cb(lv_event_t* /*e*/)
{
    overlay_show();
}

static void overlay_close_event_cb(lv_event_t* /*e*/)
{
    overlay_hide();
}

static void overlay_row_event_cb(lv_event_t* e)
{
    const int i = (int)(intptr_t)lv_event_get_user_data(e);
    bike_set_mode((AssistMode)i);
    overlay_hide();
}

static void ride_time_timer_cb(lv_timer_t* /*t*/)
{
    if (g_bike.speed_kmh > 0.5f) {
        g_bike.ride_time_s++;
        ui_set_ride_time(g_bike.ride_time_s);
    }
    ui_update_average();
}

#if DEMO_MODE
static constexpr float DEMO_ODO_BASE_KM = 19018.0f;

static void demo_speed_anim_cb(void* /*var*/, int32_t v)
{
    g_bike.speed_kmh = v / 10.0f;
    ui_set_speed(g_bike.speed_kmh);
}

static void demo_trip_timer_cb(lv_timer_t* t)
{
    const float dt_h = t->period / 3600000.0f;
    g_bike.trip_km += g_bike.speed_kmh * dt_h;
    g_bike.odo_km   = DEMO_ODO_BASE_KM + g_bike.trip_km;
    ui_set_trip(g_bike.trip_km);
    ui_set_odo(g_bike.odo_km);
}

static void demo_battery_timer_cb(lv_timer_t* /*t*/)
{
    g_bike.battery_pct = (g_bike.battery_pct > 0) ? g_bike.battery_pct - 1 : 100;
    ui_set_battery(g_bike.battery_pct);
}

static void demo_mode_timer_cb(lv_timer_t* /*t*/)
{
    if (!overlay_is_open()) bike_next_mode();
}

static void demo_start(void)
{
    g_bike.odo_km = DEMO_ODO_BASE_KM;
    ui_set_odo(g_bike.odo_km);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, nullptr);
    lv_anim_set_exec_cb(&a, demo_speed_anim_cb);
    lv_anim_set_values(&a, 0, 520);
    lv_anim_set_time(&a, 4000);
    lv_anim_set_playback_time(&a, 4000);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_start(&a);

    lv_timer_create(demo_trip_timer_cb,    100,   nullptr);
    lv_timer_create(demo_battery_timer_cb, 3000,  nullptr);
    lv_timer_create(demo_mode_timer_cb,    12000, nullptr);
}
#endif

static void print_memory_info(void)
{
    Serial.printf("[MEM] PSRAM: %s, %u KB (%u KB free)\n", psramFound() ? "found" : "NOT FOUND",
                  (unsigned)(ESP.getPsramSize() / 1024), (unsigned)(ESP.getFreePsram() / 1024));
    Serial.printf("[MEM] Internal RAM: %u KB free, DMA-capable: %u KB free\n",
                  (unsigned)(heap_caps_get_free_size(MALLOC_CAP_INTERNAL) / 1024),
                  (unsigned)(heap_caps_get_free_size(MALLOC_CAP_DMA) / 1024));
}

static void display_self_test(void)
{
    const uint16_t black  = gfx->color565(0x00, 0x00, 0x00);
    const uint16_t yellow = gfx->color565(0xFF, 0xE6, 0x00);
    const uint16_t bars[4] = { gfx->color565(0xFF, 0x00, 0x00), gfx->color565(0x00, 0xFF, 0x00),
                               gfx->color565(0x00, 0x00, 0xFF), yellow };
    const int bw = SCREEN_W / 4;
    for (int i = 0; i < 4; ++i) gfx->fillRect(i * bw, 0, bw, SCREEN_H / 2, bars[i]);
    gfx->fillRect(0, SCREEN_H / 2, SCREEN_W, SCREEN_H / 2, black);

    gfx->setTextColor(yellow, black);
    gfx->setTextSize(3);
    gfx->setCursor(24, 150);
    gfx->print("NV3041A QSPI SELF-TEST");
    gfx->setTextSize(2);
    gfx->setCursor(24, 190);
    gfx->printf("Arduino_GFX  %u MHz  IPS %d", (unsigned)(JC4827_LCD_SPI_FREQ / 1000000UL), (int)JC4827_LCD_IPS);
    gfx->setCursor(24, 214);
    gfx->printf("PSRAM %s   FLASH %u MB", psramFound() ? "OK" : "NOT FOUND",
                (unsigned)(ESP.getFlashChipSize() / (1024UL * 1024UL)));
    gfx->setCursor(24, 238);
    gfx->print("LVGL starting...");
    delay(1500);
    gfx->fillScreen(black);
}

static bool lvgl_setup(void)
{
    lv_init();

    const size_t buf_px = (size_t)SCREEN_W * LV_BUF_LINES;
    s_lv_buf = static_cast<lv_color_t*>(
        heap_caps_malloc(buf_px * sizeof(lv_color_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL));
    if (!s_lv_buf) {
        Serial.println("[LVGL] ERROR: cannot allocate the draw buffer");
        return false;
    }
    lv_disp_draw_buf_init(&s_draw_buf, s_lv_buf, nullptr, buf_px);

    lv_disp_drv_init(&s_disp_drv);
    s_disp_drv.hor_res  = SCREEN_W;
    s_disp_drv.ver_res  = SCREEN_H;
    s_disp_drv.flush_cb = lvgl_flush_cb;
    s_disp_drv.draw_buf = &s_draw_buf;
    lv_disp_drv_register(&s_disp_drv);

    lv_indev_drv_init(&s_indev_drv);
    s_indev_drv.type    = LV_INDEV_TYPE_POINTER;
    s_indev_drv.read_cb = lvgl_touch_cb;
    lv_indev_drv_register(&s_indev_drv);

    lv_mem_monitor_t mon;
    lv_mem_monitor(&mon);
    Serial.printf("[LVGL] v%d.%d.%d, draw buffer %u px (%u KB), pool %u KB (%u%% used)\n",
                  lv_version_major(), lv_version_minor(), lv_version_patch(),
                  (unsigned)buf_px, (unsigned)(buf_px * sizeof(lv_color_t) / 1024),
                  (unsigned)(mon.total_size / 1024), (unsigned)mon.used_pct);
    return true;
}

void setup()
{
    pinMode(JC4827_LCD_BL, OUTPUT);
    digitalWrite(JC4827_LCD_BL, HIGH);

    Serial.begin(115200);
    const uint32_t t0 = millis();
    while (!Serial && millis() - t0 < 2000) { delay(10); }
    delay(200);
    Serial.println();
    Serial.println("=== E-bike dashboard (high-contrast) - JC4827W543 ===");
    Serial.printf("[SYS] Flash %u MB, CPU %u MHz, loop task stack %u B free\n",
                  (unsigned)(ESP.getFlashChipSize() / (1024UL * 1024UL)), (unsigned)ESP.getCpuFreqMHz(),
                  (unsigned)uxTaskGetStackHighWaterMark(nullptr));
    print_memory_info();

    const bool lcd_ok = gfx->begin(JC4827_LCD_SPI_FREQ);
    if (!lcd_ok) Serial.println("[LCD] ERROR: gfx->begin() failed");
    gfx->fillScreen(gfx->color565(0, 0, 0));
    if (gfx->width() != SCREEN_W) Serial.println("[LCD] WARNING: display is not 480 px wide, check JC4827_LCD_ROTATION");
    Serial.printf("[LCD] NV3041A QSPI (Arduino_GFX) init %s, %dx%d landscape, rotation %d, SPI_MODE0 @ %u MHz, IPS %d, backlight GPIO %d HIGH\n",
                  lcd_ok ? "OK" : "FAILED", gfx->width(), gfx->height(), JC4827_LCD_ROTATION,
                  (unsigned)(JC4827_LCD_SPI_FREQ / 1000000UL), (int)JC4827_LCD_IPS, JC4827_LCD_BL);
    display_self_test();

    touch.begin();
    touch.setRotation(ROTATION_INVERTED);
    Serial.println("[TOUCH] GT911 initialised on I2C (SDA 8, SCL 4), address 0x5D");

    if (!lvgl_setup()) {
        Serial.println("[LVGL] FATAL: halted");
        while (true) { delay(1000); }
    }

    ui_create();
    ui_apply(g_bike);
    lv_timer_create(ride_time_timer_cb, 1000, nullptr);

#if DEMO_MODE
    demo_start();
    Serial.println("[DEMO] Simulation running (DEMO_MODE=1)");
#endif

    print_memory_info();
    Serial.println("[OK] Ready");
}

void loop()
{
    lv_timer_handler();
    delay(5);
}
