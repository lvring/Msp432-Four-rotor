/**
 * @file bar.h
 * 
 */

#ifndef GUI_BAR_H
#define GUI_BAR_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_BAR != 0

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/*Data of bar*/
typedef struct
{
    /*No inherited ext*/            /*Ext. of ancestor*/
    /*New data for this type */
    int16_t cur_value;              /*Current value of the bar*/
    int16_t min_value;              /*Minimum value of the bar*/
    int16_t max_value;              /*Maximum value of the bar*/
    style_t *style_indic;        /*Style of the indicator*/
}bar_ext_t;

typedef enum {
    GUI_BAR_STYLE_BG,
    GUI_BAR_STYLE_INDIC,
}bar_style_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a bar objects
 * @param par pointer to an object, it will be the parent of the new bar
 * @param copy pointer to a bar object, if not NULL then the new object will be copied from it
 * @return pointer to the created bar
 */
obj_t * bar_create(obj_t * par, obj_t * copy);

/*=====================
 * Setter functions
 *====================*/

/**
 * Set a new value on the bar
 * @param bar pointer to a bar object
 * @param value new value
 */
void bar_set_value(obj_t * bar, int16_t value);

/**
 * Set a new value with animation on the bar
 * @param bar pointer to a bar object
 * @param value new value
 * @param anim_time animation time in milliseconds
 */
void bar_set_value_anim(obj_t * bar, int16_t value, uint16_t anim_time);


/**
 * Set minimum and the maximum values of a bar
 * @param bar pointer to the bar object
 * @param min minimum value
 * @param max maximum value
 */
void bar_set_range(obj_t * bar, int16_t min, int16_t max);

/**
 * Set a style of a bar
 * @param bar pointer to a bar object
 * @param type which style should be set
 * @param style pointer to a style
 */
void bar_set_style(obj_t *bar, bar_style_t type, style_t *style);

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the value of a bar
 * @param bar pointer to a bar object
 * @return the value of the bar
 */
int16_t bar_get_value(obj_t * bar);

/**
 * Get the minimum value of a bar
 * @param bar pointer to a bar object
 * @return the minimum value of the bar
 */
int16_t bar_get_min_value(obj_t * bar);

/**
 * Get the maximum value of a bar
 * @param bar pointer to a bar object
 * @return the maximum value of the bar
 */
int16_t bar_get_max_value(obj_t * bar);


/**
 * Get a style of a bar
 * @param bar pointer to a bar object
 * @param type which style should be get
 * @return style pointer to a style
 */
style_t * bar_get_style(obj_t *bar, bar_style_t type);

/**********************
 *      MACROS
 **********************/

#endif  /*GUI_USE_BAR*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*GUI_BAR_H*/
