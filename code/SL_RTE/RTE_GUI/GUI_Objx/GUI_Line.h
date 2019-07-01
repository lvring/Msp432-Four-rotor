/**
 * @file line.h
 *
 */

#ifndef GUI_LINE_H
#define GUI_LINE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_LINE != 0

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/*Data of line*/
typedef struct
{
    /*Inherited from 'base_obj' so no inherited ext.*/  /*Ext. of ancestor*/
    const point_t * point_array;    /*Pointer to an array with the points of the line*/
    uint16_t  point_num;            /*Number of points in 'point_array' */
    uint8_t  auto_size  :1;         /*1: set obj. width to x max and obj. height to y max */
    uint8_t  y_inv      :1;         /*1: y == 0 will be on the bottom*/
}line_ext_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/


/**
 * Create a line objects
 * @param par pointer to an object, it will be the parent of the new line
 * @return pointer to the created line
 */
obj_t * line_create(obj_t * par, obj_t * copy);

/*=====================
 * Setter functions
 *====================*/

/**
 * Set an array of points. The line object will connect these points.
 * @param line pointer to a line object
 * @param point_a an array of points. Only the address is saved,
 * so the array can NOT be a local variable which will be destroyed
 * @param point_num number of points in 'point_a'
 */
void line_set_points(obj_t * line, const point_t * point_a, uint16_t point_num);

/**
 * Enable (or disable) the auto-size option. The size of the object will fit to its points.
 * (set width to x max and height to y max)
 * @param line pointer to a line object
 * @param autosize_en true: auto size is enabled, false: auto size is disabled
 */
void line_set_auto_size(obj_t * line, bool autosize_en);

/**
 * Enable (or disable) the y coordinate inversion.
 * If enabled then y will be subtracted from the height of the object,
 * therefore the y=0 coordinate will be on the bottom.
 * @param line pointer to a line object
 * @param yinv_en true: enable the y inversion, false:disable the y inversion
 */
void line_set_y_invert(obj_t * line, bool yinv_en);

/**
 * Set the style of a line
 * @param line pointer to a line object
 * @param style pointer to a style
 */
static inline void line_set_style(obj_t *line, style_t *style)
{
    obj_set_style(line, style);
}

/**
 * Obsolete since v5.1. Just for compatibility with v5.0. Will be removed in v6.0
 * @param line -
 * @param upscale -
 */
static inline void line_set_upscale(obj_t * line, bool upcale)
{

}
/*=====================
 * Getter functions
 *====================*/

/**
 * Get the auto size attribute
 * @param line pointer to a line object
 * @return true: auto size is enabled, false: disabled
 */
bool line_get_auto_size(obj_t * line);

/**
 * Get the y inversion attribute
 * @param line pointer to a line object
 * @return true: y inversion is enabled, false: disabled
 */
bool line_get_y_inv(obj_t * line);

/**
 * Get the style of an line object
 * @param line pointer to an line object
 * @return pointer to the line's style
 */
static inline style_t* line_get_style(obj_t *line)
{
    return obj_get_style(line);
}

/**
 * Obsolete since v5.1. Just for compatibility with v5.0. Will be removed in v6.0
 * @param line -
 * @return false
 */
static inline bool line_get_upscale(obj_t * line)
{
    return false;
}


/**********************
 *      MACROS
 **********************/

#endif /*GUI_USE_LINE*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /*GUI_LINE_H*/
