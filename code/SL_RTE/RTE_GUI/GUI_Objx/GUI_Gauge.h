/**
 * @file gauge.h
 * 
 */

#ifndef GUI_GAUGE_H
#define GUI_GAUGE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_GAUGE != 0

/*Testing of dependencies*/
#if GUI_USE_LMETER == 0
#error "gauge: lmeter is required. Enable it in conf.h (GUI_USE_LMETER  1) "
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/*Data of gauge*/
typedef struct
{
	lmeter_ext_t lmeter;     /*Ext. of ancestor*/
	/*New data for this type */
	int16_t * values;               /*Array of the set values (for needles) */
	const color_t * needle_colors;        /*Color of the needles (color_t my_colors[needle_num])*/
	const uint16_t * needle_lengths;
	uint16_t rotate_angle;
	uint8_t needle_count;             /*Number of needles*/
	uint8_t label_count;              /*Number of labels on the scale*/
}gauge_ext_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a gauge objects
 * @param par pointer to an object, it will be the parent of the new gauge
 * @param copy pointer to a gauge object, if not NULL then the new object will be copied from it
 * @return pointer to the created gauge
 */
obj_t * gauge_create(obj_t * par, obj_t * copy);

/*=====================
 * Setter functions
 *====================*/

/**
 * Set the number of needles
 * @param gauge pointer to gauge object
 * @param needle_cnt new count of needles
 * @param colors an array of colors for needles (with 'num' elements)
 */
void gauge_set_needle_count(obj_t * gauge, uint8_t needle_cnt, const color_t * colors, const uint16_t * lengths);

/**
 * Set the value of a needle
 * @param gauge pointer to a gauge
 * @param needle_id the id of the needle
 * @param value the new value
 */
void gauge_set_value(obj_t * gauge, uint8_t needle_id, int16_t value);


/**
 * Set the value of a rotate angle
 * @param gauge pointer to a gauge
 * @param value the new angle
 */
void gauge_set_rotateangle(obj_t * gauge, uint16_t angle);
/**
 * Set minimum and the maximum values of a gauge
 * @param gauge pointer to he gauge object
 * @param min minimum value
 * @param max maximum value
 */
static inline void gauge_set_range(obj_t *gauge, int16_t min, int16_t max)
{
    lmeter_set_range(gauge, min, max);
}

/**
 * Set a critical value on the scale. After this value 'line.color' scale lines will be drawn
 * @param gauge pointer to a gauge object
 * @param value the critical value
 */
static inline void gauge_set_critical_value(obj_t * gauge, int16_t value)
{
    lmeter_set_value(gauge, value);
}

/**
 * Set the scale settings of a gauge
 * @param gauge pointer to a gauge object
 * @param angle angle of the scale (0..360)
 * @param line_cnt count of scale lines
 * @param label_cnt count of scale labels
 */
void gauge_set_scale(obj_t * gauge, uint16_t angle, uint8_t line_cnt, uint8_t label_cnt);

/**
 * Set the styles of a gauge
 * @param gauge pointer to a gauge object
 * @param bg set the style of the gauge
 *  */
static inline void gauge_set_style(obj_t *gauge, style_t *bg)
{
    obj_set_style(gauge, bg);
}

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the value of a needle
 * @param gauge pointer to gauge object
 * @param needle the id of the needle
 * @return the value of the needle [min,max]
 */
int16_t gauge_get_value(obj_t * gauge,  uint8_t needle);

/**
 * Get the count of needles on a gauge
 * @param gauge pointer to gauge
 * @return count of needles
 */
uint8_t gauge_get_needle_count(obj_t * gauge);

/**
 * Get the minimum value of a gauge
 * @param gauge pointer to a gauge object
 * @return the minimum value of the gauge
 */
static inline int16_t gauge_get_min_value(obj_t * lmeter)
{
    return lmeter_get_min_value(lmeter);
}

/**
 * Get the maximum value of a gauge
 * @param gauge pointer to a gauge object
 * @return the maximum value of the gauge
 */
static inline int16_t gauge_get_max_value(obj_t * lmeter)
{
    return lmeter_get_max_value(lmeter);
}

/**
 * Get a critical value on the scale.
 * @param gauge pointer to a gauge object
 * @return the critical value
 */
static inline int16_t gauge_get_critical_value(obj_t * gauge)
{
    return lmeter_get_value(gauge);
}

/**
 * Set the number of labels (and the thicker lines too)
 * @param gauge pointer to a gauge object
 * @return count of labels
 */
uint8_t gauge_get_label_count(obj_t * gauge);

/**
 * Get the scale number of a gauge
 * @param gauge pointer to a gauge object
 * @return number of the scale units
 */
static inline uint8_t gauge_get_line_count(obj_t * gauge)
{
    return lmeter_get_line_count(gauge);
}

/**
 * Get the scale angle of a gauge
 * @param gauge pointer to a gauge object
 * @return angle of the scale
 */
static inline uint16_t gauge_get_scale_angle(obj_t * gauge)
{
    return lmeter_get_scale_angle(gauge);
}

/**
 * Get the style of a gauge
 * @param gauge pointer to a gauge object
 * @return pointer to the gauge's style
 */
static inline style_t * gauge_get_style(obj_t *gauge)
{
    return obj_get_style(gauge);
}

/**********************
 *      MACROS
 **********************/

#endif  /*GUI_USE_GAUGE*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /*GUI_GAUGE_H*/
