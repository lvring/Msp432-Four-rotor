/**
 * @file sw.h
 * 
 */

#ifndef GUI_SW_H
#define GUI_SW_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_SW != 0

/*Testing of dependencies*/
#if GUI_USE_SLIDER == 0
#error "sw: slider is required. Enable it in conf.h (GUI_USE_SLIDER  1)"
#endif


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
/*Data of switch*/
typedef struct
{
    slider_ext_t slider;         /*Ext. of ancestor*/
    /*New data for this type */
    style_t *style_knob_off;       /*Style of the knob when the switch is OFF*/
    style_t *style_knob_on;       /*Style of the knob when the switch is ON (NULL to use the same as OFF)*/
    uint8_t changed   :1;           /*Indicates the switch explicitly changed by drag*/
}sw_ext_t;

typedef enum {
    GUI_SW_STYLE_BG,
    GUI_SW_STYLE_INDIC,
    GUI_SW_STYLE_KNOB_OFF,
    GUI_SW_STYLE_KNOB_ON,
}sw_style_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a switch objects
 * @param par pointer to an object, it will be the parent of the new switch
 * @param copy pointer to a switch object, if not NULL then the new object will be copied from it
 * @return pointer to the created switch
 */
obj_t * sw_create(obj_t * par, obj_t * copy);

/*=====================
 * Setter functions
 *====================*/

/**
 * Turn ON the switch
 * @param sw pointer to a switch object
 */
void sw_on(obj_t *sw);

/**
 * Turn OFF the switch
 * @param sw pointer to a switch object
 */
void sw_off(obj_t *sw);

/**
 * Set a function which will be called when the switch is toggled by the user
 * @param sw pointer to switch object
 * @param action a callback function
 */
static inline void sw_set_action(obj_t * sw, action_t action)
{
    slider_set_action(sw, action);
}

/**
 * Set a style of a switch
 * @param sw pointer to a switch object
 * @param type which style should be set
 * @param style pointer to a style
 */
void sw_set_style(obj_t *sw, sw_style_t type, style_t *style);

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the state of a switch
 * @param sw pointer to a switch object
 * @return false: OFF; true: ON
 */
static inline bool sw_get_state(obj_t *sw)
{
    return bar_get_value(sw) == 0 ? false : true;
}

/**
 * Get the switch action function
 * @param slider pointer to a switch object
 * @return the callback function
 */
static inline action_t sw_get_action(obj_t * slider)
{
    return slider_get_action(slider);
}

/**
 * Get a style of a switch
 * @param sw pointer to a  switch object
 * @param type which style should be get
 * @return style pointer to a style
 */
style_t * sw_get_style(obj_t *sw, sw_style_t type);

/**********************
 *      MACROS
 **********************/

#endif  /*GUI_USE_SW*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /*GUI_SW_H*/
