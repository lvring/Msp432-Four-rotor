#include "RTE_Include.h"
#if RTE_USE_GUI == 1

/*********************
 *      DEFINES
 *********************/
#if GUI_USE_ANIMATION
#define GUI_STYLE_ANIM_RES       256
#define GUI_STYLE_ANIM_SHIFT     8      /*log2(GUI_STYLE_ANIM_RES)*/

#define VAL_PROP(v1, v2, r)   v1 + (((v2-v1) * r) >> GUI_STYLE_ANIM_SHIFT)
#define STYLE_ATTR_ANIM(attr, r)   if(start->attr != end->attr) act->attr = VAL_PROP(start->attr, end->attr, r)
#endif

/**********************
 *      TYPEDEFS
 **********************/
#if GUI_USE_ANIMATION
typedef struct {
    style_t style_start;   /*Save not only pointers because can be same as 'style_anim' then it will be modified too*/
    style_t style_end;
    style_t *style_anim;
    void (*end_cb)(void *);
}style_anim_dsc_t;
#endif

/**********************
 *  STATIC PROTOTYPES
 **********************/
#if GUI_USE_ANIMATION
static void style_animator(style_anim_dsc_t * dsc, int32_t val);
static void style_animation_common_end_cb(void *ptr);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/
style_t style_scr;
style_t style_transp;
style_t style_transp_fit;
style_t style_transp_tight;
style_t style_plain;
style_t style_plain_color;
style_t style_pretty;
style_t style_pretty_color;
style_t style_btn_rel;
style_t style_btn_pr;
style_t style_btn_tgl_rel;
style_t style_btn_tgl_pr;
style_t style_btn_ina;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 *  Init the basic styles
 */
void style_init (void)
{
    /* Not White/Black/Gray colors are created by HSV model with
     * HUE = 210*/

    /*Screen style*/
    style_scr.glass = 0;
    style_scr.body.opa = GUI_OPA_COVER;
    style_scr.body.main_color = GUI_COLOR_WHITE;
    style_scr.body.grad_color = GUI_COLOR_WHITE;
    style_scr.body.radius = 0;
    style_scr.body.padding.ver = GUI_DPI / 12;
    style_scr.body.padding.hor = GUI_DPI / 12;
    style_scr.body.padding.inner = GUI_DPI / 12;

    style_scr.body.border.color = GUI_COLOR_BLACK;
    style_scr.body.border.opa = GUI_OPA_COVER;
    style_scr.body.border.width = 0;
    style_scr.body.border.part = GUI_BORDER_FULL;

    style_scr.body.shadow.color = GUI_COLOR_GRAY;
    style_scr.body.shadow.type = GUI_SHADOW_FULL;
    style_scr.body.shadow.width = 0;

    style_scr.text.opa = GUI_OPA_COVER;
    style_scr.text.color = GUI_COLOR_MAKE(0x30, 0x30, 0x30);
    style_scr.text.font = GUI_FONT_DEFAULT;
    style_scr.text.letter_space = 2;
    style_scr.text.line_space = 2;

    style_scr.image.opa = GUI_OPA_COVER;
    style_scr.image.color = GUI_COLOR_MAKE(0x20, 0x20, 0x20);
    style_scr.image.intense = GUI_OPA_TRANSP;

    style_scr.line.opa = GUI_OPA_COVER;
    style_scr.line.color = GUI_COLOR_MAKE(0x20, 0x20, 0x20);
    style_scr.line.width = 1;

    /*Plain style (by default near the same as the screen style)*/
    memcpy(&style_plain, &style_scr, sizeof(style_t));

    /*Plain color style*/
    memcpy(&style_plain_color, &style_plain, sizeof(style_t));
    style_plain_color.text.color = GUI_COLOR_MAKE(0xf0, 0xf0, 0xf0);
    style_plain_color.image.color = GUI_COLOR_MAKE(0xf0, 0xf0, 0xf0);
    style_plain_color.line.color = GUI_COLOR_MAKE(0xf0, 0xf0, 0xf0);
    style_plain_color.body.main_color = GUI_COLOR_MAKE(0x55, 0x96, 0xd8);
    style_plain_color.body.grad_color = style_plain_color.body.main_color;

    /*Pretty style */
    memcpy(&style_pretty, &style_plain, sizeof(style_t));
    style_pretty.text.color = GUI_COLOR_MAKE(0x20, 0x20, 0x20);
    style_pretty.image.color = GUI_COLOR_MAKE(0x20, 0x20, 0x20);
    style_pretty.line.color = GUI_COLOR_MAKE(0x20, 0x20, 0x20);
    style_pretty.body.main_color = GUI_COLOR_WHITE;
    style_pretty.body.grad_color = GUI_COLOR_SILVER;
    style_pretty.body.radius = GUI_DPI / 15;
    style_pretty.body.border.color = GUI_COLOR_MAKE(0x40, 0x40, 0x40);
    style_pretty.body.border.width = GUI_DPI / 50 >= 1 ? GUI_DPI / 50  : 1;
    style_pretty.body.border.opa = GUI_OPA_30;

    /*Pretty color style*/
    memcpy(&style_pretty_color, &style_pretty, sizeof(style_t));
    style_pretty_color.text.color = GUI_COLOR_MAKE(0xe0, 0xe0, 0xe0);
    style_pretty_color.image.color = GUI_COLOR_MAKE(0xe0, 0xe0, 0xe0);
    style_pretty_color.line.color = GUI_COLOR_MAKE(0xc0, 0xc0, 0xc0);
    style_pretty_color.body.main_color = GUI_COLOR_MAKE(0x6b, 0x9a, 0xc7);
    style_pretty_color.body.grad_color = GUI_COLOR_MAKE(0x2b, 0x59, 0x8b);
    style_pretty_color.body.border.color = GUI_COLOR_MAKE(0x15, 0x2c, 0x42);

    /*Transparent style*/
    memcpy(&style_transp, &style_plain, sizeof(style_t));
    style_transp.body.empty = 1;
    style_transp.glass = 1;
    style_transp.body.border.width = 0;

    /*Transparent tight style*/
    memcpy(&style_transp_fit, &style_transp, sizeof(style_t));
    style_transp_fit.body.padding.hor = 0;
    style_transp_fit.body.padding.ver = 0;

    /*Transparent fitting size*/
    memcpy(&style_transp_tight, &style_transp_fit, sizeof(style_t));
    style_transp_tight.body.padding.inner = 0;

    /*Button released style*/
    memcpy(&style_btn_rel, &style_plain, sizeof(style_t));
    style_btn_rel.body.main_color = GUI_COLOR_MAKE(0x76, 0xa2, 0xd0);
    style_btn_rel.body.grad_color = GUI_COLOR_MAKE(0x19, 0x3a, 0x5d);
    style_btn_rel.body.radius = GUI_DPI / 15;
    style_btn_rel.body.padding.hor = GUI_DPI / 4;
    style_btn_rel.body.padding.ver = GUI_DPI / 6;
    style_btn_rel.body.padding.inner = GUI_DPI / 10;
    style_btn_rel.body.border.color = GUI_COLOR_MAKE(0x0b, 0x19, 0x28);
    style_btn_rel.body.border.width = GUI_DPI / 50 >= 1 ? GUI_DPI / 50  : 1;
    style_btn_rel.body.border.opa = GUI_OPA_70;
    style_btn_rel.text.color = GUI_COLOR_MAKE(0xff, 0xff, 0xff);
    style_btn_rel.body.shadow.color = GUI_COLOR_GRAY;
    style_btn_rel.body.shadow.width = 0;

    /*Button pressed style*/
    memcpy(&style_btn_pr, &style_btn_rel, sizeof(style_t));
    style_btn_pr.body.main_color = GUI_COLOR_MAKE(0x33, 0x62, 0x94);
    style_btn_pr.body.grad_color = GUI_COLOR_MAKE(0x10, 0x26, 0x3c);
    style_btn_pr.text.color = GUI_COLOR_MAKE(0xa4, 0xb5, 0xc6);
    style_btn_pr.image.color = GUI_COLOR_MAKE(0xa4, 0xb5, 0xc6);
    style_btn_pr.line.color = GUI_COLOR_MAKE(0xa4, 0xb5, 0xc6);

    /*Button toggle released style*/
    memcpy(&style_btn_tgl_rel, &style_btn_rel, sizeof(style_t));
    style_btn_tgl_rel.body.main_color = GUI_COLOR_MAKE(0x0a, 0x11, 0x22);
    style_btn_tgl_rel.body.grad_color = GUI_COLOR_MAKE(0x37, 0x62, 0x90);
    style_btn_tgl_rel.body.border.color = GUI_COLOR_MAKE(0x01, 0x07, 0x0d);
    style_btn_tgl_rel.text.color = GUI_COLOR_MAKE(0xc8, 0xdd, 0xf4);
    style_btn_tgl_rel.image.color = GUI_COLOR_MAKE(0xc8, 0xdd, 0xf4);
    style_btn_tgl_rel.line.color = GUI_COLOR_MAKE(0xc8, 0xdd, 0xf4);

    /*Button toggle pressed style*/
    memcpy(&style_btn_tgl_pr, &style_btn_tgl_rel, sizeof(style_t));
    style_btn_tgl_pr.body.main_color = GUI_COLOR_MAKE(0x02, 0x14, 0x27);
    style_btn_tgl_pr.body.grad_color = GUI_COLOR_MAKE(0x2b, 0x4c, 0x70);
    style_btn_tgl_pr.text.color = GUI_COLOR_MAKE(0xa4, 0xb5, 0xc6);
    style_btn_tgl_pr.image.color = GUI_COLOR_MAKE(0xa4, 0xb5, 0xc6);
    style_btn_tgl_pr.line.color = GUI_COLOR_MAKE(0xa4, 0xb5, 0xc6);

    /*Button inactive style*/
    memcpy(&style_btn_ina, &style_btn_rel, sizeof(style_t));
    style_btn_ina.body.main_color = GUI_COLOR_MAKE(0xd8, 0xd8, 0xd8);
    style_btn_ina.body.grad_color = GUI_COLOR_MAKE(0xd8, 0xd8, 0xd8);
    style_btn_ina.body.border.color = GUI_COLOR_MAKE(0x90, 0x90, 0x90);
    style_btn_ina.text.color = GUI_COLOR_MAKE(0x70, 0x70, 0x70);
    style_btn_ina.image.color = GUI_COLOR_MAKE(0x70, 0x70, 0x70);
    style_btn_ina.line.color = GUI_COLOR_MAKE(0x70, 0x70, 0x70);
}


/**
 * Copy a style to an other
 * @param dest pointer to the destination style
 * @param src pointer to the source style
 */
void style_copy(style_t * dest, const style_t * src)
{
    memcpy(dest, src, sizeof(style_t));
}

#if GUI_USE_ANIMATION
/**
 * Create an animation from a pre-configured 'style_anim_t' variable
 * @param anim pointer to a pre-configured 'style_anim_t' variable (will be copied)
 */
void style_anim_create(style_anim_t * anim)
{
    style_anim_dsc_t * dsc;
    dsc = RTE_BGet( MEM_RTE ,sizeof(style_anim_dsc_t));
    dsc->style_anim = anim->style_anim;
    memcpy(&dsc->style_start, anim->style_start, sizeof(style_t));
    memcpy(&dsc->style_end, anim->style_end, sizeof(style_t));
    dsc->end_cb = anim->end_cb;


    anim_t a;
    a.var = (void*)dsc;
    a.start = 0;
    a.end = GUI_STYLE_ANIM_RES;
    a.fp = (anim_fp_t)style_animator;
    a.path = anim_path_linear;
    a.end_cb = style_animation_common_end_cb;
    a.act_time = anim->act_time;
    a.time = anim->time;
    a.playback = anim->playback;
    a.playback_pause = anim->playback_pause;
    a.repeat = anim->repeat;
    a.repeat_pause = anim->repeat_pause;

    anim_create(&a);
}
#endif
/**********************
 *   STATIC FUNCTIONS
 **********************/
#if GUI_USE_ANIMATION
/**
 * Used by the style animations to set the values of a style according to start and end style.
 * @param dsc the 'animated variable' set by style_anim_create()
 * @param val the current state of the animation between 0 and GUI_STYLE_ANIM_RES
 */
static void style_animator(style_anim_dsc_t * dsc, int32_t val)
{
    const style_t * start = &dsc->style_start;
    const style_t * end = &dsc->style_end;
    style_t * act = dsc->style_anim;

    STYLE_ATTR_ANIM(body.opa, val);
    STYLE_ATTR_ANIM(body.radius, val);
    STYLE_ATTR_ANIM(body.border.width, val);
    STYLE_ATTR_ANIM(body.shadow.width, val);
    STYLE_ATTR_ANIM(body.padding.hor, val);
    STYLE_ATTR_ANIM(body.padding.ver, val);
    STYLE_ATTR_ANIM(body.padding.inner, val);
    STYLE_ATTR_ANIM(text.line_space, val);
    STYLE_ATTR_ANIM(text.letter_space, val);
    STYLE_ATTR_ANIM(text.opa, val);
    STYLE_ATTR_ANIM(line.width, val);
    STYLE_ATTR_ANIM(line.opa, val);
    STYLE_ATTR_ANIM(image.intense, val);
    STYLE_ATTR_ANIM(image.opa, val);

    opa_t opa = val == GUI_STYLE_ANIM_RES ? GUI_OPA_COVER : val;

    act->body.main_color = color_mix(end->body.main_color, start->body.main_color, opa);
    act->body.grad_color = color_mix(end->body.grad_color, start->body.grad_color, opa);
    act->body.border.color = color_mix(end->body.border.color, start->body.border.color, opa);
    act->body.shadow.color = color_mix(end->body.shadow.color, start->body.shadow.color, opa);
    act->text.color = color_mix(end->text.color, start->text.color, opa);
    act->image.color = color_mix(end->image.color, start->image.color, opa);
    act->line.color = color_mix(end->line.color, start->line.color, opa);

    if(val == 0) {
        act->body.empty = start->body.empty;
        act->glass = start->glass;
        act->text.font = start->text.font;
        act->body.shadow.type = start->body.shadow.type;
    }

    if(val == GUI_STYLE_ANIM_RES) {
        act->body.empty = end->body.empty;
        act->glass = end->glass;
        act->text.font = end->text.font;
        act->body.shadow.type = end->body.shadow.type;
    }

    obj_report_style_mod(dsc->style_anim);
}

/**
 * Called when a style animation is ready
 * It called the user defined call back and free the allocated memories
 * @param ptr the 'animated variable' set by style_anim_create()
 */
static void style_animation_common_end_cb(void *ptr)
{
    style_anim_dsc_t *dsc = ptr;     /*To avoid casting*/

    if(dsc->end_cb) dsc->end_cb(dsc);

    RTE_BRel(MEM_RTE , dsc);
}

#endif

#endif
