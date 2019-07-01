/**
 * @file theme_default.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"

#if GUI_USE_THEME_DEFAULT

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static theme_t theme;
static style_t def;

/*Static style definitions*/
static style_t sb;
static style_t plain_bordered;
static style_t label_prim;
static style_t label_sec;
static style_t label_hint;
static style_t slider_bg;
static style_t sw_bg;
static style_t lmeter;

/*Saved input parameters*/
static uint16_t _hue;
static font_t * _font;

/**********************
 *      MACROS
 **********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void basic_init(void)
{
    style_copy(&def, &style_pretty);  /*Initialize the default style*/

    style_copy(&sb, &style_pretty_color);
    sb.body.grad_color = sb.body.main_color;
    sb.body.padding.hor = sb.body.padding.hor / 2;      /*Make closer to the edges*/
    sb.body.padding.ver = sb.body.padding.ver / 2;

    style_copy(&plain_bordered, &style_plain);
    plain_bordered.body.border.width = 2;
    plain_bordered.body.border.color= GUI_COLOR_HEX3(0xbbb);

    theme.bg = &style_plain;
    theme.panel = &style_pretty;

}

static void btn_init(void)
{
#if GUI_USE_BTN != 0
    theme.btn.rel = &style_btn_rel;
    theme.btn.pr = &style_btn_pr;
    theme.btn.tgl_rel = &style_btn_tgl_rel;
    theme.btn.tgl_pr =  &style_btn_tgl_pr;
    theme.btn.ina =  &style_btn_ina;
#endif
}

static void label_init(void)
{
#if GUI_USE_LABEL != 0

    style_copy(&label_prim, &style_plain);
    style_copy(&label_sec, &style_plain);
    style_copy(&label_hint, &style_plain);

    label_prim.text.color = GUI_COLOR_HEX3(0x111);
    label_sec.text.color = GUI_COLOR_HEX3(0x888);
    label_hint.text.color = GUI_COLOR_HEX3(0xaaa);


    theme.label.prim = &label_prim;
    theme.label.sec = &label_sec;
    theme.label.hint = &label_hint;
#endif
}


static void img_init(void)
{
#if GUI_USE_IMG != 0


    theme.img.light = &def;
    theme.img.dark = &def;
#endif
}

static void line_init(void)
{
#if GUI_USE_LINE != 0

    theme.line.decor = &def;
#endif
}

static void led_init(void)
{
#if GUI_USE_LED != 0
    static style_t led;

    style_copy(&led, &style_pretty_color);
    led.body.shadow.width = GUI_DPI / 10;
    led.body.radius = GUI_RADIUS_CIRCLE;
    led.body.border.width= GUI_DPI / 30;
    led.body.border.opa = GUI_OPA_30;
    led.body.shadow.color = led.body.main_color;


    theme.led = &led;
#endif
}

static void bar_init(void)
{
#if GUI_USE_BAR

    theme.bar.bg = &style_pretty;
    theme.bar.indic = &style_pretty_color;
#endif
}

static void slider_init(void)
{
#if GUI_USE_SLIDER != 0
    style_copy(&slider_bg, &style_pretty);
    slider_bg.body.padding.hor = GUI_DPI / 20;
    slider_bg.body.padding.ver = GUI_DPI / 20;

    theme.slider.bg = &slider_bg;
    theme.slider.indic = &style_pretty_color;
    theme.slider.knob = &style_pretty;
#endif
}

static void sw_init(void)
{
#if GUI_USE_SW != 0
    style_copy(&sw_bg, &style_pretty);
    sw_bg.body.padding.hor = 3;
    sw_bg.body.padding.ver = 3;

    theme.sw.bg = &sw_bg;
    theme.sw.indic = &style_pretty_color;
    theme.sw.knob_off = &style_pretty;
    theme.sw.knob_on = &style_pretty;
#endif
}


static void lmeter_init(void)
{
#if GUI_USE_LMETER != 0

    style_copy(&lmeter, &style_pretty_color);
    lmeter.line.color = GUI_COLOR_HEX3(0xddd);
    lmeter.line.width = 2;
    lmeter.body.main_color = color_mix(lmeter.body.main_color, GUI_COLOR_WHITE, GUI_OPA_50);
    lmeter.body.grad_color = color_mix(lmeter.body.grad_color, GUI_COLOR_BLACK, GUI_OPA_50);

    theme.lmeter = &lmeter;
#endif
}

static void gauge_init(void)
{
#if GUI_USE_GAUGE != 0
    static style_t gauge;
    style_copy(&gauge, &lmeter);
    gauge.line.color = lmeter.body.grad_color;
    gauge.line.width = 2;
    gauge.body.main_color = GUI_COLOR_HEX3(0x888);
    gauge.body.grad_color = lmeter.body.main_color;
    gauge.text.color = GUI_COLOR_HEX3(0x888);

    theme.gauge = &gauge;
#endif
}

static void chart_init(void)
{
#if GUI_USE_CHART


    theme.chart = &style_pretty;
#endif
}

static void cb_init(void)
{
#if GUI_USE_CB != 0


    theme.cb.bg = &style_transp;
    theme.cb.box.rel = &style_pretty;
    theme.cb.box.pr = &style_btn_pr;
    theme.cb.box.tgl_rel = &style_btn_tgl_rel;
    theme.cb.box.tgl_pr = &style_btn_tgl_pr;
    theme.cb.box.ina = &style_btn_ina;
#endif
}


static void btnm_init(void)
{
#if GUI_USE_BTNM


    theme.btnm.bg = &style_pretty;
    theme.btnm.btn.rel = &style_btn_rel;
    theme.btnm.btn.pr = &style_btn_pr;
    theme.btnm.btn.tgl_rel = &style_btn_tgl_rel;
    theme.btnm.btn.tgl_pr = &style_btn_tgl_pr;
    theme.btnm.btn.ina = &style_btn_ina;
#endif
}

static void kb_init(void)
{
#if GUI_USE_KB


    theme.kb.bg = &style_pretty;
    theme.kb.btn.rel = &style_btn_rel;
    theme.kb.btn.pr = &style_btn_pr;
    theme.kb.btn.tgl_rel = &style_btn_tgl_rel;
    theme.kb.btn.tgl_pr = &style_btn_tgl_pr;
    theme.kb.btn.ina = &style_btn_ina;
#endif

}

static void mbox_init(void)
{
#if GUI_USE_MBOX


    theme.mbox.bg = &style_pretty;
    theme.mbox.btn.bg = &style_transp;
    theme.mbox.btn.rel = &style_btn_rel;
    theme.mbox.btn.pr = &style_btn_tgl_pr;
#endif
}

static void page_init(void)
{
#if GUI_USE_PAGE


    theme.page.bg = &style_pretty;
    theme.page.scrl = &style_transp_tight;
    theme.page.sb = &sb;
#endif
}

static void ta_init(void)
{
#if GUI_USE_TA


    theme.ta.area = &style_pretty;
    theme.ta.oneline = &style_pretty;
    theme.ta.cursor = NULL;
    theme.ta.sb = &sb;
#endif
}

static void list_init(void)
{
#if GUI_USE_LIST != 0

    theme.list.bg = &style_pretty;
    theme.list.scrl = &style_transp_fit;
    theme.list.sb = &sb;
    theme.list.btn.rel = &style_btn_rel;
    theme.list.btn.pr = &style_btn_pr;
    theme.list.btn.tgl_rel = &style_btn_tgl_rel;
    theme.list.btn.tgl_pr = &style_btn_tgl_pr;
    theme.list.btn.ina = &style_btn_ina;
#endif
}

static void ddlist_init(void)
{
#if GUI_USE_DDLIST != 0


    theme.ddlist.bg = &style_pretty;
    theme.ddlist.sel = &style_plain_color;
    theme.ddlist.sb = &sb;
#endif
}

static void roller_init(void)
{
#if GUI_USE_ROLLER != 0


    theme.roller.bg = &style_pretty;
    theme.roller.sel = &style_plain_color;
#endif
}

static void tabview_init(void)
{
#if GUI_USE_TABVIEW != 0


    theme.tabview.bg = &plain_bordered;
    theme.tabview.indic = &style_plain_color;
    theme.tabview.btn.bg = &style_transp;
    theme.tabview.btn.rel = &style_btn_rel;
    theme.tabview.btn.pr = &style_btn_pr;
    theme.tabview.btn.tgl_rel = &style_btn_tgl_rel;
    theme.tabview.btn.tgl_pr = &style_btn_tgl_pr;
#endif
}


static void win_init(void)
{
#if GUI_USE_WIN != 0


    theme.win.bg = &plain_bordered;
    theme.win.sb = &sb;
    theme.win.header = &style_plain_color;
    theme.win.content.bg = &style_transp;
    theme.win.content.scrl = &style_transp;
    theme.win.btn.rel = &style_btn_rel;
    theme.win.btn.pr = &style_btn_pr;
#endif
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/



/**
 * Initialize the default theme
 * @param hue [0..360] hue value from HSV color space to define the theme's base color
 * @param font pointer to a font (NULL to use the default)
 * @return pointer to the initialized theme
 */
theme_t * theme_default_init(uint16_t hue, font_t *font)
{
    if(font == NULL) font = GUI_FONT_DEFAULT;

    _hue = hue;
    _font = font;

    /*For backward compatibility initialize all theme elements with a default style */
    uint16_t i;
    style_t **style_p = (style_t**) &theme;
    for(i = 0; i < sizeof(theme_t) / sizeof(style_t*); i++) {
        *style_p = &def;
        style_p++;
    }

    basic_init();
    btn_init();
    label_init();
    img_init();
    line_init();
    led_init();
    bar_init();
    slider_init();
    sw_init();
    lmeter_init();
    gauge_init();
    chart_init();
    cb_init();
    btnm_init();
    kb_init();
    mbox_init();
    page_init();
    ta_init();
    list_init();
    ddlist_init();
    roller_init();
    tabview_init();
    win_init();

    return &theme;
}

/**
 * Get a pointer to the theme
 * @return pointer to the theme
 */
theme_t * theme_get_default(void)
{
    return &theme;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif

