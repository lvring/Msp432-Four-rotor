/**
 * @file theme_night.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"

#if GUI_USE_THEME_NIGHT

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
static style_t bg, sb, panel;
static style_t prim, sec, hint;
static style_t btn_rel, btn_pr, btn_tgl_rel, btn_tgl_pr, btn_ina;
static style_t bar_bg, bar_indic;
static style_t slider_knob;
static style_t ddlist_bg, ddlist_sel;
static style_t lmeter_bg;

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
    def.text.font = _font;

    style_copy(&bg, &style_plain);
    bg.body.main_color = color_hsv_to_rgb(_hue, 11, 30);
    bg.body.grad_color = color_hsv_to_rgb(_hue, 11, 30);
    bg.text.color = color_hsv_to_rgb(_hue, 5, 95);
    bg.text.font = _font;

    style_copy(&sb, &def);
    sb.body.main_color = color_hsv_to_rgb(_hue, 30, 60);
    sb.body.grad_color = color_hsv_to_rgb(_hue, 30, 60);
    sb.body.border.width = 0;
    sb.body.padding.inner = GUI_DPI / 10;
    sb.body.padding.ver = 0;
    sb.body.padding.hor = 0;
    sb.body.radius = GUI_DPI / 30;
    sb.body.opa = GUI_OPA_COVER;

    style_copy(&panel, &bg);
    panel.body.main_color = color_hsv_to_rgb(_hue, 11, 18);
    panel.body.grad_color = color_hsv_to_rgb(_hue, 11, 18);
    panel.body.radius = GUI_DPI / 20;
    panel.body.border.color = color_hsv_to_rgb(_hue, 10, 25);
    panel.body.border.width = 1;
    panel.body.border.opa = GUI_OPA_COVER;
    panel.body.padding.ver = GUI_DPI / 10;
    panel.body.padding.hor = GUI_DPI / 10;
    panel.line.color = color_hsv_to_rgb(_hue, 20, 40);
    panel.line.width = 1;
    theme.bg = &bg;
    theme.panel = &def;
}

static void cont_init(void)
{
#if GUI_USE_CONT != 0


    theme.cont = &panel;
#endif
}
static void btn_init(void)
{
#if GUI_USE_BTN != 0

    style_copy(&btn_rel, &def);
    btn_rel.body.main_color = color_hsv_to_rgb(_hue, 10, 40);
    btn_rel.body.grad_color = color_hsv_to_rgb(_hue, 10, 20);
    btn_rel.body.border.color = GUI_COLOR_HEX3(0x111);
    btn_rel.body.border.width = 1;
    btn_rel.body.border.opa = GUI_OPA_70;
    btn_rel.body.padding.hor = GUI_DPI / 4;
    btn_rel.body.padding.ver = GUI_DPI / 8;
    btn_rel.body.shadow.type = GUI_SHADOW_BOTTOM;
    btn_rel.body.shadow.color = GUI_COLOR_HEX3(0x111);
    btn_rel.body.shadow.width = GUI_DPI / 30;
    btn_rel.text.color = GUI_COLOR_HEX3(0xeee);

    style_copy(&btn_pr, &btn_rel);
    btn_pr.body.main_color = color_hsv_to_rgb(_hue, 10, 30);
    btn_pr.body.grad_color = color_hsv_to_rgb(_hue, 10, 10);

    style_copy(&btn_tgl_rel, &btn_rel);
    btn_tgl_rel.body.main_color = color_hsv_to_rgb(_hue, 10, 20);
    btn_tgl_rel.body.grad_color = color_hsv_to_rgb(_hue, 10, 40);
    btn_tgl_rel.body.shadow.width = GUI_DPI / 40;
    btn_tgl_rel.text.color = GUI_COLOR_HEX3(0xddd);

    style_copy(&btn_tgl_pr, &btn_rel);
    btn_tgl_pr.body.main_color = color_hsv_to_rgb(_hue, 10, 10);
    btn_tgl_pr.body.grad_color = color_hsv_to_rgb(_hue, 10, 30);
    btn_tgl_pr.body.shadow.width = GUI_DPI / 30;
    btn_tgl_pr.text.color = GUI_COLOR_HEX3(0xddd);

    style_copy(&btn_ina, &btn_rel);
    btn_ina.body.main_color = color_hsv_to_rgb(_hue, 10, 20);
    btn_ina.body.grad_color = color_hsv_to_rgb(_hue, 10, 20);
    btn_ina.text.color = GUI_COLOR_HEX3(0xaaa);
    btn_ina.body.shadow.width = 0;

    theme.btn.rel = &btn_rel;
    theme.btn.pr = &btn_pr;
    theme.btn.tgl_rel = &btn_tgl_rel;
    theme.btn.tgl_pr =  &btn_tgl_pr;
    theme.btn.ina =  &btn_ina;
#endif
}


static void label_init(void)
{
#if GUI_USE_LABEL != 0

    style_copy(&prim, &bg);
    prim.text.color = color_hsv_to_rgb(_hue, 5, 95);

    style_copy(&sec, &bg);
    sec.text.color = color_hsv_to_rgb(_hue, 15, 65);

    style_copy(&hint, &bg);
    hint.text.color = color_hsv_to_rgb(_hue, 20, 55);

    theme.label.prim = &prim;
    theme.label.sec = &sec;
    theme.label.hint = &hint;
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
    style_copy(&led, &def);
    led.body.shadow.width = GUI_DPI / 10;
    led.body.radius = GUI_RADIUS_CIRCLE;
    led.body.border.width= GUI_DPI / 30;
    led.body.border.opa = GUI_OPA_30;
    led.body.main_color = color_hsv_to_rgb(_hue, 100, 100);
    led.body.grad_color = color_hsv_to_rgb(_hue, 100, 40);
    led.body.border.color = color_hsv_to_rgb(_hue, 60, 60);
    led.body.shadow.color = color_hsv_to_rgb(_hue, 100, 100);

    theme.led = &led;
#endif
}

static void bar_init(void)
{
#if GUI_USE_BAR
    style_copy(&bar_bg, &panel);
    bar_bg.body.padding.ver = GUI_DPI / 16;
    bar_bg.body.padding.hor = GUI_DPI / 16;
    bar_bg.body.radius = GUI_RADIUS_CIRCLE;

    style_copy(&bar_indic, &def);
    bar_indic.body.main_color = color_hsv_to_rgb(_hue, 80, 70);
    bar_indic.body.grad_color = color_hsv_to_rgb(_hue, 80, 70);
    bar_indic.body.border.color = color_hsv_to_rgb(_hue, 20, 15);
    bar_indic.body.border.width = 1;
    bar_indic.body.border.opa = GUI_OPA_COVER;
    bar_indic.body.radius = GUI_RADIUS_CIRCLE;
    bar_indic.body.padding.hor = 0;
    bar_indic.body.padding.ver = 0;

    theme.bar.bg = &bar_bg;
    theme.bar.indic = &bar_indic;
#endif
}

static void slider_init(void)
{
#if GUI_USE_SLIDER != 0

    style_copy(&slider_knob, &btn_rel);
    slider_knob.body.radius = GUI_RADIUS_CIRCLE;

    theme.slider.bg = &bar_bg;
    theme.slider.indic = &bar_indic;
    theme.slider.knob = &slider_knob;
#endif
}

static void sw_init(void)
{
#if GUI_USE_SW != 0


    theme.sw.bg = &bar_bg;
    theme.sw.indic = &bar_indic;
    theme.sw.knob_off = &slider_knob;
    theme.sw.knob_on = &slider_knob;
#endif
}


static void lmeter_init(void)
{
#if GUI_USE_LMETER != 0
    style_copy(&lmeter_bg, &def);
    lmeter_bg.body.main_color = color_hsv_to_rgb(_hue, 10, 70);
    lmeter_bg.body.grad_color = color_hsv_to_rgb(_hue, 95, 90);
    lmeter_bg.body.padding.hor = GUI_DPI / 10;           /*Scale line length*/
    lmeter_bg.body.padding.inner = GUI_DPI / 10;         /*Text padding*/
    lmeter_bg.body.border.color = GUI_COLOR_HEX3(0x333);
    lmeter_bg.line.color = GUI_COLOR_HEX3(0x555);
    lmeter_bg.line.width = 1;
    lmeter_bg.text.color = GUI_COLOR_HEX3(0xddd);

    theme.lmeter = &lmeter_bg;
#endif
}

static void gauge_init(void)
{
#if GUI_USE_GAUGE != 0
    static style_t gauge_bg;
    style_copy(&gauge_bg, &def);
    gauge_bg.body.main_color = color_hsv_to_rgb(_hue, 10, 70);
    gauge_bg.body.grad_color = gauge_bg.body.main_color;
    gauge_bg.line.color = color_hsv_to_rgb(_hue, 80, 75);
    gauge_bg.line.width = 1;
    gauge_bg.text.color = GUI_COLOR_HEX3(0xddd);

    theme.gauge = &gauge_bg;
#endif
}

static void chart_init(void)
{
#if GUI_USE_CHART

    theme.chart = &panel;
#endif
}

static void cb_init(void)
{
#if GUI_USE_CB != 0

    static style_t rel, pr, tgl_rel, tgl_pr, ina;

    style_copy(&rel, &def);
    rel.body.radius = GUI_DPI / 20;
    rel.body.main_color = color_hsv_to_rgb(_hue, 10, 95);
    rel.body.grad_color = color_hsv_to_rgb(_hue, 10, 95);
    rel.body.border.color = color_hsv_to_rgb(_hue, 10, 50);
    rel.body.border.width = 2;;

    style_copy(&pr, &rel);
    pr.body.main_color = color_hsv_to_rgb(_hue, 10, 80);
    pr.body.grad_color = color_hsv_to_rgb(_hue, 10, 80);
    pr.body.border.color = color_hsv_to_rgb(_hue, 10, 20);
    pr.body.border.width = 1;;

    style_copy(&tgl_rel, &rel);
    tgl_rel.body.main_color = color_hsv_to_rgb(_hue, 80, 90);
    tgl_rel.body.grad_color = color_hsv_to_rgb(_hue, 80, 90);
    tgl_rel.body.border.color = color_hsv_to_rgb(_hue, 80, 50);

    style_copy(&tgl_pr, &tgl_rel);
    tgl_pr.body.main_color = color_hsv_to_rgb(_hue, 80, 70);
    tgl_pr.body.grad_color = color_hsv_to_rgb(_hue, 80, 70);
    tgl_pr.body.border.color = color_hsv_to_rgb(_hue, 80, 30);
    tgl_pr.body.border.width = 1;;


    style_copy(&ina, &rel);
    ina.body.main_color = GUI_COLOR_HEX3(0x777);
    ina.body.grad_color = GUI_COLOR_HEX3(0x777);
    ina.body.border.width = 0;

    theme.cb.bg = &style_transp;
    theme.cb.box.rel = &rel;
    theme.cb.box.pr = &pr;
    theme.cb.box.tgl_rel = &tgl_rel;
    theme.cb.box.tgl_pr = &tgl_pr;
    theme.cb.box.ina = &def;
#endif
}


static void btnm_init(void)
{
#if GUI_USE_BTNM
    static style_t btnm_bg, rel, pr, tgl_rel, tgl_pr, ina;

    style_copy(&btnm_bg, &btn_rel);
    btnm_bg.body.padding.hor = 2;
    btnm_bg.body.padding.ver = 2;
    btnm_bg.body.padding.inner = 0;
    btnm_bg.body.border.width =  1;

    style_copy(&rel, &btn_rel);
    rel.body.border.part = GUI_BORDER_RIGHT;
    rel.body.border.width = 2;
    rel.body.radius = 0;

    style_copy(&pr, &btn_pr);
    pr.body.border.part = GUI_BORDER_RIGHT;
    pr.body.border.width = 2;
    pr.body.radius = 0;

    style_copy(&tgl_rel, &btn_tgl_rel);
    tgl_rel.body.border.part = GUI_BORDER_RIGHT;
    tgl_rel.body.border.width = 2;
    tgl_rel.body.radius = 0;

    style_copy(&tgl_pr, &btn_tgl_pr);
    tgl_pr.body.border.part = GUI_BORDER_RIGHT;
    tgl_pr.body.border.width = 2;
    tgl_pr.body.radius = 0;

    style_copy(&ina, &btn_ina);
    ina.body.border.part = GUI_BORDER_RIGHT;
    ina.body.border.width = 2;
    ina.body.radius = 0;

    theme.btnm.bg = &btnm_bg;
    theme.btnm.btn.rel = &rel;
    theme.btnm.btn.pr = &pr;
    theme.btnm.btn.tgl_rel = &tgl_rel;
    theme.btnm.btn.tgl_pr = &tgl_pr;
    theme.btnm.btn.ina = &ina;
#endif
}

static void kb_init(void)
{
#if GUI_USE_KB
    theme.kb.bg = &bg;
    theme.kb.btn.rel = &btn_rel;
    theme.kb.btn.pr = &btn_pr;
    theme.kb.btn.tgl_rel = &btn_tgl_rel;
    theme.kb.btn.tgl_pr = &btn_tgl_pr;
    theme.kb.btn.ina = &btn_ina;
#endif

}

static void mbox_init(void)
{
#if GUI_USE_MBOX
    static style_t mbox_bg;
    style_copy(&mbox_bg, &bg);
    mbox_bg.body.main_color =  color_hsv_to_rgb(_hue, 30, 30);
    mbox_bg.body.grad_color =  color_hsv_to_rgb(_hue, 30, 30);
    mbox_bg.body.border.color =  color_hsv_to_rgb(_hue, 11, 20);
    mbox_bg.body.border.width = 1;
    mbox_bg.body.shadow.width = GUI_DPI / 10;
    mbox_bg.body.shadow.color = GUI_COLOR_HEX3(0x222);
    mbox_bg.body.radius = GUI_DPI / 20;
    theme.mbox.bg = &mbox_bg;
    theme.mbox.btn.bg = &style_transp;
    theme.mbox.btn.rel = &btn_rel;
    theme.mbox.btn.pr = &btn_pr;
#endif
}

static void page_init(void)
{
#if GUI_USE_PAGE

    static style_t page_scrl;
    style_copy(&page_scrl, &bg);
    page_scrl.body.main_color = color_hsv_to_rgb(_hue, 10, 40);
    page_scrl.body.grad_color = color_hsv_to_rgb(_hue, 10, 40);
    page_scrl.body.border.color = GUI_COLOR_HEX3(0x333);
    page_scrl.body.border.width = 1;
    page_scrl.body.radius = GUI_DPI / 20;

    theme.page.bg = &panel;
    theme.page.scrl = &page_scrl;
    theme.page.sb = &sb;
#endif
}

static void ta_init(void)
{
#if GUI_USE_TA
    theme.ta.area = &panel;
    theme.ta.oneline = &panel;
    theme.ta.cursor = NULL;
    theme.ta.sb = &def;
#endif
}

static void list_init(void)
{
#if GUI_USE_LIST != 0
    static style_t list_bg, list_btn_rel, list_btn_pr, list_btn_tgl_rel, list_btn_tgl_pr;

    style_copy(&list_bg, &panel);
    list_bg.body.padding.ver = 0;
    list_bg.body.padding.hor = 0;
    list_bg.body.padding.inner = 0;

    style_copy(&list_btn_rel, &bg);
    list_btn_rel.body.empty = 1;
    list_btn_rel.body.border.part = GUI_BORDER_BOTTOM;
    list_btn_rel.body.border.color = color_hsv_to_rgb(_hue, 10, 5);
    list_btn_rel.body.border.width = 1;
    list_btn_rel.body.radius = GUI_DPI / 10;
    list_btn_rel.text.color = color_hsv_to_rgb(_hue, 5, 80);
    list_btn_rel.body.padding.ver = GUI_DPI / 6;
    list_btn_rel.body.padding.hor = GUI_DPI / 8;

    style_copy(&list_btn_pr, &btn_pr);
    list_btn_pr.body.main_color = btn_pr.body.grad_color;
    list_btn_pr.body.grad_color = btn_pr.body.main_color;
    list_btn_pr.body.border.color = color_hsv_to_rgb(_hue, 10, 5);
    list_btn_pr.body.border.width = 0;
    list_btn_pr.body.radius = GUI_DPI / 30;
    list_btn_pr.body.padding.ver = GUI_DPI / 6;
    list_btn_pr.body.padding.hor = GUI_DPI / 8;
    list_btn_pr.text.color = color_hsv_to_rgb(_hue, 5, 80);

    style_copy(&list_btn_tgl_rel, &list_btn_rel);
    list_btn_tgl_rel.body.empty = 0;
    list_btn_tgl_rel.body.main_color = color_hsv_to_rgb(_hue, 10, 8);
    list_btn_tgl_rel.body.grad_color = color_hsv_to_rgb(_hue, 10, 8);
    list_btn_tgl_rel.body.radius = GUI_DPI / 30;

    style_copy(&list_btn_tgl_pr, &list_btn_tgl_rel);
    list_btn_tgl_pr.body.main_color = btn_tgl_pr.body.main_color;
    list_btn_tgl_pr.body.grad_color = btn_tgl_pr.body.grad_color;

    theme.list.sb = &sb;
    theme.list.bg = &list_bg;
    theme.list.scrl = &style_transp_tight;
    theme.list.btn.rel = &list_btn_rel;
    theme.list.btn.pr = &list_btn_pr;
    theme.list.btn.tgl_rel = &list_btn_tgl_rel;
    theme.list.btn.tgl_pr = &list_btn_tgl_pr;
    theme.list.btn.ina = &def;
#endif
}

static void ddlist_init(void)
{
#if GUI_USE_DDLIST != 0
    style_copy(&ddlist_bg, &btn_rel);
    ddlist_bg.text.line_space = GUI_DPI / 8;
    ddlist_bg.body.padding.ver = GUI_DPI / 8;
    ddlist_bg.body.padding.hor = GUI_DPI / 8;
    ddlist_bg.body.radius = GUI_DPI / 30;

    style_copy(&ddlist_sel, &btn_rel);
    ddlist_sel.body.main_color = color_hsv_to_rgb(_hue, 20, 50);
    ddlist_sel.body.grad_color = color_hsv_to_rgb(_hue, 20, 50);
    ddlist_sel.body.radius = 0;

    theme.ddlist.bg = &ddlist_bg;
    theme.ddlist.sel = &ddlist_sel;
    theme.ddlist.sb = &def;
#endif
}

static void roller_init(void)
{
#if GUI_USE_ROLLER != 0
    static style_t roller_bg;

    style_copy(&roller_bg, &ddlist_bg);
    roller_bg.body.main_color = color_hsv_to_rgb(_hue, 10, 20);
    roller_bg.body.grad_color = color_hsv_to_rgb(_hue, 10, 40);
    roller_bg.text.color = color_hsv_to_rgb(_hue, 5, 70);
    roller_bg.text.opa = GUI_OPA_60;

    theme.roller.bg = &roller_bg;
    theme.roller.sel = &ddlist_sel;
#endif
}

static void tabview_init(void)
{
#if GUI_USE_TABVIEW != 0
    theme.tabview.bg = &bg;
    theme.tabview.indic = &style_transp;
    theme.tabview.btn.bg = &style_transp;
    theme.tabview.btn.rel = &btn_rel;
    theme.tabview.btn.pr = &btn_pr;
    theme.tabview.btn.tgl_rel = &btn_tgl_rel;
    theme.tabview.btn.tgl_pr = &btn_tgl_pr;
#endif
}


static void win_init(void)
{
#if GUI_USE_WIN != 0
    static style_t win_bg;
    style_copy(&win_bg, &bg);
    win_bg.body.border.color = GUI_COLOR_HEX3(0x333);
    win_bg.body.border.width = 1;

    static style_t win_header;
    style_copy(&win_header, &win_bg);
    win_header.body.main_color = color_hsv_to_rgb(_hue, 10, 20);
    win_header.body.grad_color = color_hsv_to_rgb(_hue, 10, 20);
    win_header.body.radius = 0;
    win_header.body.padding.hor = 0;
    win_header.body.padding.ver = 0;
    win_header.body.padding.ver = 0;

    static style_t win_btn_pr;
    style_copy(&win_btn_pr, &def);
    win_btn_pr.body.main_color = color_hsv_to_rgb(_hue, 10, 10);
    win_btn_pr.body.grad_color = color_hsv_to_rgb(_hue, 10, 10);
    win_btn_pr.text.color = GUI_COLOR_HEX3(0xaaa);

    theme.win.bg = &win_bg;
    theme.win.sb = &sb;
    theme.win.header = &win_header;
    theme.win.content.bg = &style_transp;
    theme.win.content.scrl = &style_transp;
    theme.win.btn.rel = &style_transp;
    theme.win.btn.pr = &win_btn_pr;
#endif
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/



/**
 * Initialize the night theme
 * @param hue [0..360] hue value from HSV color space to define the theme's base color
 * @param font pointer to a font (NULL to use the default)
 * @return pointer to the initialized theme
 */
theme_t * theme_night_init(uint16_t hue, font_t *font)
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
theme_t * theme_get_deafult(void)
{
    return &theme;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif

