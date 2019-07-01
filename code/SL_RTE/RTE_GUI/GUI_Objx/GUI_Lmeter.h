/**
 * @file lmeter.h
 * 
 */

#ifndef GUI_LMETER_H
#define GUI_LMETER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_LMETER != 0


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
/*Data of line meter*/
typedef struct
{
    /*No inherited ext.*/            /*Ext. of ancestor*/
    /*New data for this type */
    uint16_t scale_angle;        /*Angle of the scale in deg. (0..360)*/
    uint8_t line_cnt;            /*Count of lines */
    int16_t cur_value;
    int16_t min_value;
    int16_t max_value;
}lmeter_ext_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a line meter objects
 * @param par pointer to an object, it will be the parent of the new line meter
 * @param copy pointer to a line meter object, if not NULL then the new object will be copied from it
 * @return pointer to the created line meter
 */
obj_t * lmeter_create(obj_t * par, obj_t * copy);

/*=====================
 * Setter functions
 *====================*/

/**
 * Set a new value on the line meter
 * @param lmeter pointer to a line meter object
 * @param value new value
 */
void lmeter_set_value(obj_t *lmeter, int16_t value);

/**
 * Set minimum and the maximum values of a line meter
 * @param lmeter pointer to he line meter object
 * @param min minimum value
 * @param max maximum value
 */
void lmeter_set_range(obj_t *lmeter, int16_t min, int16_t max);

/**
 * Set the scale settings of a line meter
 * @param lmeter pointer to a line meter object
 * @param angle angle of the scale (0..360)
 * @param line_cnt number of lines
 */
void lmeter_set_scale(obj_t * lmeter, uint16_t angle, uint8_t line_cnt);

/**
 * Set the styles of a line meter
 * @param lmeter pointer to a line meter object
 * @param bg set the style of the line meter
 */
static inline void lmeter_set_style(obj_t *lmeter, style_t *bg)
{
    obj_set_style(lmeter, bg);
}

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the value of a line meter
 * @param lmeter pointer to a line meter object
 * @return the value of the line meter
 */
int16_t lmeter_get_value(obj_t *lmeter);

/**
 * Get the minimum value of a line meter
 * @param lmeter pointer to a line meter object
 * @return the minimum value of the line meter
 */
int16_t lmeter_get_min_value(obj_t * lmeter);

/**
 * Get the maximum value of a line meter
 * @param lmeter pointer to a line meter object
 * @return the maximum value of the line meter
 */
int16_t lmeter_get_max_value(obj_t * lmeter);

/**
 * Get the scale number of a line meter
 * @param lmeter pointer to a line meter object
 * @return number of the scale units
 */
uint8_t lmeter_get_line_count(obj_t * lmeter);

/**
 * Get the scale angle of a line meter
 * @param lmeter pointer to a line meter object
 * @return angle of the scale
 */
uint16_t lmeter_get_scale_angle(obj_t * lmeter);

/**
 * Get the style of a line meter
 * @param lmeter pointer to a line meter object
 * @return pointer to the line meter's style
 */
static inline style_t * lmeter_get_style_bg(obj_t * lmeter)
{
    return obj_get_style(lmeter);
}

/**********************
 *      MACROS
 **********************/

#endif  /*GUI_USE_LMETER*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /*GUI_LMETER_H*/
