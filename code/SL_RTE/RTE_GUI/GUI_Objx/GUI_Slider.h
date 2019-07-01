/**
 * @file slider.h
 * 
 */

#ifndef GUI_SLIDER_H
#define GUI_SLIDER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_SLIDER != 0

/*Testing of dependencies*/
#if GUI_USE_BAR == 0
#error "slider: bar is required. Enable it in conf.h (GUI_USE_BAR  1) "
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
/*Data of slider*/
typedef struct
{
    bar_ext_t bar;       /*Ext. of ancestor*/
    /*New data for this type */
    action_t action;             /*Function to call when a new value is set*/
    style_t *style_knob;    /*Style of the knob*/
    int16_t drag_value;          /*Store a temporal value during press until release (Handled by the library)*/
    uint8_t knob_in     :1;     /*1: Draw the knob inside the bar*/
}slider_ext_t;

/*Built-in styles of slider*/
typedef enum
{
	GUI_SLIDER_STYLE_BG,
    GUI_SLIDER_STYLE_INDIC,
    GUI_SLIDER_STYLE_KNOB,
}slider_style_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a slider objects
 * @param par pointer to an object, it will be the parent of the new slider
 * @param copy pointer to a slider object, if not NULL then the new object will be copied from it
 * @return pointer to the created slider
 */
obj_t * slider_create(obj_t * par, obj_t * copy);

/*=====================
 * Setter functions
 *====================*/

/**
 * Set a new value on the slider
 * @param slider pointer to a slider object
 * @param value new value
 */
static inline void slider_set_value(obj_t * slider, int16_t value)
{
    bar_set_value(slider, value);
}

/**
 * Set a new value with animation on a slider
 * @param slider pointer to a slider object
 * @param value new value
 * @param anim_time animation time in milliseconds
 */
static inline void slider_set_value_anim(obj_t * slider, int16_t value, uint16_t anim_time)
{
    bar_set_value_anim(slider, value, anim_time);
}

/**
 * Set minimum and the maximum values of a bar
 * @param slider pointer to the slider object
 * @param min minimum value
 * @param max maximum value
 */
static inline void slider_set_range(obj_t *slider, int16_t min, int16_t max)
{
    bar_set_range(slider, min, max);
}

/**
 * Set a function which will be called when a new value is set on the slider
 * @param slider pointer to slider object
 * @param action a callback function
 */
void slider_set_action(obj_t * slider, action_t action);

/**
 * Set the 'knob in' attribute of a slider
 * @param slider pointer to slider object
 * @param in true: the knob is drawn always in the slider;
 *           false: the knob can be out on the edges
 */
void slider_set_knob_in(obj_t * slider, bool in);

/**
 * Set a style of a slider
 * @param slider pointer to a slider object
 * @param type which style should be set
 * @param style pointer to a style
 */
void slider_set_style(obj_t *slider, slider_style_t type, style_t *style);

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the value of a slider
 * @param slider pointer to a slider object
 * @return the value of the slider
 */
int16_t slider_get_value(obj_t * slider);

/**
 * Get the minimum value of a slider
 * @param slider pointer to a slider object
 * @return the minimum value of the slider
 */
static inline int16_t slider_get_min_value(obj_t * slider)
{
    return bar_get_min_value(slider);
}

/**
 * Get the maximum value of a slider
 * @param slider pointer to a slider object
 * @return the maximum value of the slider
 */
static inline int16_t slider_get_max_value(obj_t * slider)
{
    return bar_get_max_value(slider);
}

/**
 * Get the slider action function
 * @param slider pointer to slider object
 * @return the callback function
 */
action_t slider_get_action(obj_t * slider);

/**
 * Give the slider is being dragged or not
 * @param slider pointer to a slider object
 * @return true: drag in progress false: not dragged
 */
bool slider_is_dragged(obj_t * slider);

/**
 * Get the 'knob in' attribute of a slider
 * @param slider pointer to slider object
 * @return true: the knob is drawn always in the slider;
 *         false: the knob can be out on the edges
 */
bool slider_get_knob_in(obj_t * slider);


/**
 * Get a style of a slider
 * @param slider pointer to a slider object
 * @param type which style should be get
 * @return style pointer to a style
 */
style_t * slider_get_style(obj_t *slider, slider_style_t type);

/**********************
 *      MACROS
 **********************/

#endif  /*GUI_USE_SLIDER*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /*GUI_SLIDER_H*/
