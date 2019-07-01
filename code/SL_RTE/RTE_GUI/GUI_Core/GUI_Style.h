#ifndef __GUI_STYLE_H
#define __GUI_STYLE_H
#ifdef __cplusplus
extern "C" {
#endif
#include "RTE_Include.h"
/*********************
 *      DEFINES
 *********************/
#define GUI_RADIUS_CIRCLE  (GUI_COORD_MAX)    /*A very big radius to always draw as circle*/
/**********************
 *      TYPEDEFS
 **********************/
/*Border types (Use 'OR'ed values)*/
typedef enum
{
    GUI_BORDER_NONE =   0x00,
    GUI_BORDER_BOTTOM = 0x01,
    GUI_BORDER_TOP =    0x02,
    GUI_BORDER_LEFT =   0x04,
    GUI_BORDER_RIGHT =  0x08,
    GUI_BORDER_FULL =   0x0F,
}border_part_t;

/*Shadow types*/
typedef enum
{
    GUI_SHADOW_BOTTOM = 0,
    GUI_SHADOW_FULL,
}shadow_type_t;
	
typedef struct
{
    uint8_t glass :1;   /*1: Do not inherit this style*/

    struct {
        color_t main_color;
        color_t grad_color;
        coord_t radius;
        opa_t opa;

        struct {
            color_t color;
            coord_t width;
            border_part_t part;
            opa_t opa;
        }border;

        struct {
            color_t color;
            coord_t width;
            uint8_t type;
        }shadow;

        struct {
            coord_t ver;
            coord_t hor;
            coord_t inner;
        }padding;

        uint8_t empty :1;   /*Transparent background (border still drawn)*/
    }body;


    struct {
        color_t color;
        const font_t * font;
        coord_t letter_space;
        coord_t line_space;
        opa_t opa;
    }text;

    struct {
        color_t color;
        opa_t intense;
        opa_t opa;
    }image;

    struct {
        color_t color;
        coord_t width;
        opa_t opa;
    }line;
}style_t;

#if GUI_USE_ANIMATION == 1
typedef struct {
    const style_t * style_start; /*Pointer to the starting style*/
    const style_t * style_end;   /*Pointer to the destination style*/
    style_t * style_anim;        /*Pointer to a style to animate*/
    anim_cb_t end_cb;            /*Call it when the animation is ready (NULL if unused)*/
    int16_t time;                   /*Animation time in ms*/
    int16_t act_time;               /*Current time in animation. Set to negative to make delay.*/
    uint16_t playback_pause;        /*Wait before play back*/
    uint16_t repeat_pause;          /*Wait before repeat*/
    uint8_t playback :1;            /*When the animation is ready play it back*/
    uint8_t repeat :1;              /*Repeat the animation infinitely*/
}style_anim_t;

/* Example initialization
style_anim_t a;
a.style_anim = &style_to_anim;
a.style_start = &style_1;
a.style_end = &style_2;
a.act_time = 0;
a.time = 1000;
a.playback = 0;
a.playback_pause = 0;
a.repeat = 0;
a.repeat_pause = 0;
a.end_cb = NULL;
style_anim_create(&a);
 */
#endif

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 *  Init the basic styles
 */
void style_init (void);

/**
 * Copy a style to an other
 * @param dest pointer to the destination style
 * @param src pointer to the source style
 */
void style_copy(style_t * dest, const style_t * src);

#if GUI_USE_ANIMATION == 1
/**
 * Create an animation from a pre-configured 'style_anim_t' variable
 * @param anim pointer to a pre-configured 'style_anim_t' variable (will be copied)
 */
void style_anim_create(style_anim_t * anim);
#endif

/*************************
 *    GLOBAL VARIABLES
 *************************/
extern style_t style_scr;
extern style_t style_transp;
extern style_t style_transp_fit;
extern style_t style_transp_tight;
extern style_t style_plain;
extern style_t style_plain_color;
extern style_t style_pretty;
extern style_t style_pretty_color;
extern style_t style_btn_rel;
extern style_t style_btn_pr;
extern style_t style_btn_tgl_rel;
extern style_t style_btn_tgl_pr;;
extern style_t style_btn_ina;




#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
