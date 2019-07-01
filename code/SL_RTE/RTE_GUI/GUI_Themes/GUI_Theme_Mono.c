/**
 * @file theme_templ.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"


#if GUI_USE_THEME_MONO

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
static style_t light_plain;
static style_t dark_plain;
static style_t light_frame;
static style_t dark_frame;

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
    style_copy(&def, &style_plain);  /*Initialize the default style*/
    def.body.main_color = GUI_COLOR_WHITE;
    def.body.grad_color = GUI_COLOR_WHITE;
    def.body.radius = 0;
    def.body.opa = GUI_OPA_COVER;
    def.body.padding.hor = GUI_DPI / 10;
    def.body.padding.ver = GUI_DPI / 10;
    def.body.padding.inner = GUI_DPI / 10;
    def.body.border.color = GUI_COLOR_BLACK;
    def.body.border.width = 1;
    def.body.border.opa = GUI_OPA_COVER;
    def.body.border.part = GUI_BORDER_FULL;

    def.text.font = _font;
    def.text.color = GUI_COLOR_BLACK;
    def.text.letter_space = 1;
    def.text.line_space = 1;

    def.line.color = GUI_COLOR_BLACK;
    def.line.opa = GUI_OPA_COVER;
    def.line.width = 1;

    def.image.color = GUI_COLOR_BLACK;
    def.image.intense = GUI_OPA_TRANSP;
    def.image.opa = GUI_OPA_COVER;

    style_copy(&light_plain, &def);

    style_copy(&light_frame, &light_plain);
    light_frame.body.radius = GUI_DPI / 20;

    style_copy(&dark_plain, &light_plain);
    dark_plain.body.main_color = GUI_COLOR_BLACK;
    dark_plain.body.grad_color = GUI_COLOR_BLACK;
    dark_plain.body.border.color = GUI_COLOR_WHITE;
    dark_plain.text.color = GUI_COLOR_WHITE;
    dark_plain.line.color = GUI_COLOR_WHITE;
    dark_plain.image.color = GUI_COLOR_WHITE;

    style_copy(&dark_frame, &dark_plain);
    dark_frame.body.radius = GUI_DPI / 20;

    theme.bg = &def;
    theme.panel = &light_frame;

}

static void cont_init(void)
{
#if GUI_USE_CONT != 0


    theme.cont = &def;
#endif
}

static void btn_init(void)
{
#if GUI_USE_BTN != 0


    theme.btn.rel = &light_frame;
    theme.btn.pr = &dark_frame;
    theme.btn.tgl_rel = &dark_frame;
    theme.btn.tgl_pr =  &light_frame;
    theme.btn.ina =  &light_frame;
#endif
}


static void label_init(void)
{
#if GUI_USE_LABEL != 0


    theme.label.prim = NULL;
    theme.label.sec = NULL;
    theme.label.hint = NULL;
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
    theme.line.decor = NULL;
#endif
}

static void led_init(void)
{
#if GUI_USE_LED != 0
    static style_t led;
    style_copy(&led, &light_frame);
    led.body.radius = GUI_RADIUS_CIRCLE;
    led.body.shadow.width = GUI_DPI / 8;
    led.body.shadow.color = GUI_COLOR_BLACK;
    led.body.shadow.type = GUI_SHADOW_FULL;

    theme.led = &led;
#endif
}

static void bar_init(void)
{
#if GUI_USE_BAR
    static style_t bar_bg;
    static style_t bar_indic;

    style_copy(&bar_bg, &light_frame);
    bar_bg.body.padding.hor = GUI_DPI / 15;
    bar_bg.body.padding.ver = GUI_DPI / 15;
    bar_bg.body.radius = GUI_RADIUS_CIRCLE;

    style_copy(&bar_indic, &dark_frame);
    bar_indic.body.padding.hor = GUI_DPI / 30;
    bar_indic.body.padding.ver = GUI_DPI / 30;
    bar_indic.body.radius = GUI_RADIUS_CIRCLE;

    theme.bar.bg = &bar_bg;
    theme.bar.indic = &bar_indic;
#endif
}

static void slider_init(void)
{
#if GUI_USE_SLIDER != 0
    static style_t slider_knob;
    style_copy(&slider_knob, &light_frame);
    slider_knob.body.radius = GUI_RADIUS_CIRCLE;
    slider_knob.body.padding.hor = GUI_DPI / 30;
    slider_knob.body.padding.ver = GUI_DPI / 30;

    theme.slider.bg = theme.bar.bg;
    theme.slider.indic = theme.bar.indic;
    theme.slider.knob = &slider_knob;
#endif
}

static void sw_init(void)
{
#if GUI_USE_SW != 0


    theme.sw.bg = theme.slider.bg;
    theme.sw.indic = theme.slider.indic;
    theme.sw.knob_off = theme.slider.knob;
    theme.sw.knob_on = theme.slider.knob;
#endif
}


static void lmeter_init(void)
{
#if GUI_USE_LMETER != 0
    static style_t lmeter_bg;
    style_copy(&lmeter_bg, &light_frame);
    lmeter_bg.body.empty = 1;
    lmeter_bg.body.main_color = GUI_COLOR_BLACK;
    lmeter_bg.body.grad_color = GUI_COLOR_BLACK;
    lmeter_bg.body.padding.hor = GUI_DPI / 20;
    lmeter_bg.body.padding.inner = GUI_DPI / 8;
    lmeter_bg.line.color = GUI_COLOR_WHITE;
    lmeter_bg.line.width = 1;

    theme.lmeter = &lmeter_bg;
#endif
}

static void gauge_init(void)
{
#if GUI_USE_GAUGE != 0
    static style_t gauge_bg;
    style_copy(&gauge_bg, theme.lmeter);
    gauge_bg.line.color = GUI_COLOR_BLACK;
    gauge_bg.line.width = 1;


    theme.gauge = &gauge_bg;
#endif
}

static void chart_init(void)
{
#if GUI_USE_CHART


    theme.chart = &light_frame;
#endif
}

static void cb_init(void)
{
#if GUI_USE_CB != 0


    theme.cb.bg = &style_transp;
    theme.cb.box.rel = &light_frame;
    theme.cb.box.pr = &dark_frame;
    theme.cb.box.tgl_rel = &dark_frame;
    theme.cb.box.tgl_pr = &light_frame;
    theme.cb.box.ina = &light_frame;
#endif
}


static void btnm_init(void)
{
#if GUI_USE_BTNM


    theme.btnm.bg = &light_frame;
    theme.btnm.btn.rel = &light_frame;
    theme.btnm.btn.pr = &dark_frame;
    theme.btnm.btn.tgl_rel = &dark_frame;
    theme.btnm.btn.tgl_pr = &light_frame;
    theme.btnm.btn.ina = &light_frame;
#endif
}

static void kb_init(void)
{
#if GUI_USE_KB
    theme.kb.bg = &style_transp_fit;
    theme.kb.btn.rel = &light_frame;
    theme.kb.btn.pr = &light_frame;
    theme.kb.btn.tgl_rel = &dark_frame;
    theme.kb.btn.tgl_pr = &dark_frame;
    theme.kb.btn.ina = &light_frame;
#endif

}

static void mbox_init(void)
{
#if GUI_USE_MBOX


    theme.mbox.bg = &dark_frame;
    theme.mbox.btn.bg = &style_transp_fit;
    theme.mbox.btn.rel = &light_frame;
    theme.mbox.btn.pr = &dark_frame;
#endif
}

static void page_init(void)
{
#if GUI_USE_PAGE


    theme.page.bg = &light_frame;
    theme.page.scrl = &light_frame;
    theme.page.sb = &dark_frame;
#endif
}

static void ta_init(void)
{
#if GUI_USE_TA


    theme.ta.area = &light_frame;
    theme.ta.oneline = &light_frame;
    theme.ta.cursor = NULL;     /*Let library to calculate the cursor's style*/
    theme.ta.sb = &dark_frame;
#endif
}

static void list_init(void)
{
#if GUI_USE_LIST != 0


    theme.list.sb = &dark_frame;
    theme.list.bg = &light_frame;
    theme.list.scrl = &style_transp_fit;
    theme.list.btn.rel = &light_plain;
    theme.list.btn.pr = &dark_plain;
    theme.list.btn.tgl_rel = &dark_plain;
    theme.list.btn.tgl_pr = &light_plain;
    theme.list.btn.ina = &light_plain;
#endif
}

static void ddlist_init(void)
{
#if GUI_USE_DDLIST != 0


    theme.ddlist.bg = &light_frame;
    theme.ddlist.sel = &dark_plain;
    theme.ddlist.sb = &dark_frame;
#endif
}

static void roller_init(void)
{
#if GUI_USE_ROLLER != 0


    theme.roller.bg = &light_frame;
    theme.roller.sel = &dark_frame;
#endif
}

static void tabview_init(void)
{
#if GUI_USE_TABVIEW != 0


    theme.tabview.bg = &light_frame;
    theme.tabview.indic = &light_plain;
    theme.tabview.btn.bg = &style_transp_fit;
    theme.tabview.btn.rel = &light_frame;
    theme.tabview.btn.pr = &dark_frame;
    theme.tabview.btn.tgl_rel = &dark_frame;
    theme.tabview.btn.tgl_pr = &light_frame;
#endif
}


static void win_init(void)
{
#if GUI_USE_WIN != 0
    static style_t win_header;
    style_copy(&win_header, &dark_plain);
    win_header.body.padding.hor = GUI_DPI / 30;
    win_header.body.padding.ver = GUI_DPI / 30;

    theme.win.bg = &light_frame;
    theme.win.sb = &dark_frame;
    theme.win.header = &win_header;
    theme.win.content.bg = &style_transp;
    theme.win.content.scrl = &style_transp;
    theme.win.btn.rel = &light_frame;
    theme.win.btn.pr = &dark_frame;
#endif
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/



/**
 * Initialize the mono theme
 * @param hue [0..360] hue value from HSV color space to define the theme's base color
 * @param font pointer to a font (NULL to use the default)
 * @return pointer to the initialized theme
 */
theme_t * theme_mono_init(uint16_t hue, font_t *font)
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
    cont_init();
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
theme_t * theme_get_mono(void)
{
    return &theme;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif

