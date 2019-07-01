/**
 * @file led.h
 * 
 */

#ifndef GUI_LED_H
#define GUI_LED_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_LED != 0


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/*Data of led*/
typedef struct
{
    /*No inherited ext.*/
    /*New data for this type */
    uint8_t bright;         /*Current brightness of the LED (0..255)*/
}led_ext_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a led objects
 * @param par pointer to an object, it will be the parent of the new led
 * @param copy pointer to a led object, if not NULL then the new object will be copied from it
 * @return pointer to the created led
 */
obj_t * led_create(obj_t * par, obj_t * copy);

/**
 * Set the brightness of a LED object
 * @param led pointer to a LED object
 * @param bright 0 (max. dark) ... 255 (max. light)
 */
void led_set_bright(obj_t * led, uint8_t bright);

/**
 * Light on a LED
 * @param led pointer to a LED object
 */
void led_on(obj_t * led);

/**
 * Light off a LED
 * @param led pointer to a LED object
 */
void led_off(obj_t * led);

/**
 * Toggle the state of a LED
 * @param led pointer to a LED object
 */
void led_toggle(obj_t * led);

/**
 * Set the style of a led
 * @param led pointer to a led object
 * @param style pointer to a style
 */
static inline void led_set_style(obj_t *led, style_t *style)
{
    obj_set_style(led, style);
}

/**
 * Get the brightness of a LEd object
 * @param led pointer to LED object
 * @return bright 0 (max. dark) ... 255 (max. light)
 */
uint8_t led_get_bright(obj_t * led);

/**
 * Get the style of an led object
 * @param led pointer to an led object
 * @return pointer to the led's style
 */
static inline style_t* led_get_style(obj_t *led)
{
    return obj_get_style(led);
}

/**********************
 *      MACROS
 **********************/

#endif  /*GUI_USE_LED*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /*GUI_LED_H*/
