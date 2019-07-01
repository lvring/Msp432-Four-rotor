/**
 * @file led.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_LED != 0

/*********************
 *      DEFINES
 *********************/
#define GUI_LED_WIDTH_DEF    (GUI_DPI / 3)
#define GUI_LED_HEIGHT_DEF   (GUI_DPI / 3)
#define GUI_LED_BRIGHT_OFF	100
#define GUI_LED_BRIGHT_ON	255

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static bool led_design(obj_t * led, const area_t * mask, design_mode_t mode);
static res_t led_signal(obj_t * led, signal_t sign, void * param);

/**********************
 *  STATIC VARIABLES
 **********************/
static design_func_t ancestor_design_f;
static signal_func_t ancestor_signal;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Create a led objects
 * @param par pointer to an object, it will be the parent of the new led
 * @param copy pointer to a led object, if not NULL then the new object will be copied from it
 * @return pointer to the created led
 */
obj_t * led_create(obj_t * par, obj_t * copy)
{
    /*Create the ancestor basic object*/
	obj_t * new_led = obj_create(par, copy);
    RTE_AssertParam(new_led);
    if(ancestor_signal == NULL) ancestor_signal = obj_get_signal_func(new_led);
    if(ancestor_design_f == NULL) ancestor_design_f = obj_get_design_func(new_led);
    
    /*Allocate the object type specific extended data*/
    led_ext_t * ext = obj_allocate_ext_attr(new_led, sizeof(led_ext_t));
    RTE_AssertParam(ext);
    ext->bright = GUI_LED_BRIGHT_ON;

    obj_set_signal_func(new_led, led_signal);
    obj_set_design_func(new_led, led_design);

    /*Init the new led object*/
    if(copy == NULL) {
    	obj_set_size(new_led, GUI_LED_WIDTH_DEF, GUI_LED_HEIGHT_DEF);

        /*Set the default styles*/
        theme_t *th = theme_get_current();
        if(th) {
            led_set_style(new_led, th->led);
        } else {
            led_set_style(new_led, &style_pretty_color);
        }
    }
    /*Copy an existing object*/
    else {
    	led_ext_t * copy_ext = obj_get_ext_attr(copy);
    	ext->bright = copy_ext->bright;

        /*Refresh the style with new signal function*/
        obj_refresh_style(new_led);
    }
    
    return new_led;
}

/*=====================
 * Setter functions
 *====================*/

/**
 * Set the brightness of a LED object
 * @param led pointer to a LED object
 * @param bright 0 (max. dark) ... 255 (max. light)
 */
void led_set_bright(obj_t * led, uint8_t bright)
{
	/*Set the brightness*/
	led_ext_t * ext = obj_get_ext_attr(led);
	if(ext->bright == bright) return;

	ext->bright = bright;

	/*Invalidate the object there fore it will be redrawn*/
	obj_invalidate(led);
}

/**
 * Light on a LED
 * @param led pointer to a LED object
 */
void led_on(obj_t * led)
{
	led_set_bright(led, GUI_LED_BRIGHT_ON);
}

/**
 * Light off a LED
 * @param led pointer to a LED object
 */
void led_off(obj_t * led)
{
	led_set_bright(led, GUI_LED_BRIGHT_OFF);
}


/**
 * Toggle the state of a LED
 * @param led pointer to a LED object
 */
void led_toggle(obj_t * led)
{
	uint8_t bright = led_get_bright(led);
	if(bright > (GUI_LED_BRIGHT_OFF + GUI_LED_BRIGHT_ON) >> 1) led_off(led);
	else led_on(led);
}

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the brightness of a LEd object
 * @param led pointer to LED object
 * @return bright 0 (max. dark) ... 255 (max. light)
 */
uint8_t led_get_bright(obj_t * led)
{
	led_ext_t * ext = obj_get_ext_attr(led);
	return ext->bright;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Handle the drawing related tasks of the leds
 * @param led pointer to an object
 * @param mask the object will be drawn only in this area
 * @param mode GUI_DESIGN_COVER_CHK: only check if the object fully covers the 'mask_p' area
 *                                  (return 'true' if yes)
 *             GUI_DESIGN_DRAW: draw the object (always return 'true')
 *             GUI_DESIGN_DRAW_POST: drawing after every children are drawn
 * @param return true/false, depends on 'mode'
 */
static bool led_design(obj_t * led, const area_t * mask, design_mode_t mode)
{
    if(mode == GUI_DESIGN_COVER_CHK) {
    	/*Return false if the object is not covers the mask area*/
    	return ancestor_design_f(led, mask, mode);
    } else if(mode == GUI_DESIGN_DRAW_MAIN) {
		/*Make darker colors in a temporary style according to the brightness*/
		led_ext_t * ext = obj_get_ext_attr(led);
		style_t * style = obj_get_style(led);

        /* Store the real pointer because of 'group'
         * If the object is in focus 'obj_get_style()' will give a pointer to tmp style
         * and to the real object style. It is important because of style change tricks below*/
        style_t *style_ori_p = led->style_p;

		/*Create a temporal style*/
        style_t leds_tmp;
		memcpy(&leds_tmp, style, sizeof(leds_tmp));

		/*Mix. the color with black proportionally with brightness*/
		leds_tmp.body.main_color = color_mix(leds_tmp.body.main_color, GUI_COLOR_BLACK, ext->bright);
		leds_tmp.body.grad_color = color_mix(leds_tmp.body.grad_color, GUI_COLOR_BLACK, ext->bright);
        leds_tmp.body.border.color = color_mix(leds_tmp.body.border.color, GUI_COLOR_BLACK, ext->bright);

		/*Set the current swidth according to brightness proportionally between GUI_LED_BRIGHT_OFF and GUI_LED_BRIGHT_ON*/
		uint16_t bright_tmp = ext->bright;
        leds_tmp.body.shadow.width = ((bright_tmp - GUI_LED_BRIGHT_OFF) * style->body.shadow.width) / (GUI_LED_BRIGHT_ON - GUI_LED_BRIGHT_OFF);

		led->style_p = &leds_tmp;
		ancestor_design_f(led, mask, mode);
        led->style_p = style_ori_p;                 /*Restore the ORIGINAL style pointer*/
    }
    return true;
}

/**
 * Signal function of the led
 * @param led pointer to a led object
 * @param sign a signal type from signal_t enum
 * @param param pointer to a signal specific variable
 * @return GUI_RES_OK: the object is not deleted in the function; GUI_RES_INV: the object is deleted
 */
static res_t led_signal(obj_t * led, signal_t sign, void * param)
{
    res_t res;

    /* Include the ancient signal function */
    res = ancestor_signal(led, sign, param);
    if(res != GUI_RES_OK) return res;


    if(sign == GUI_SIGNAL_GET_TYPE) {
        obj_type_t * buf = param;
        uint8_t i;
        for(i = 0; i < GUI_MAX_ANCESTOR_NUM - 1; i++) {  /*Find the last set data*/
            if(buf->type[i] == NULL) break;
        }
        buf->type[i] = "led";
    }

    return res;
}
#endif
