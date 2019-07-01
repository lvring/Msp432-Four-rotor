/**
 * @file circ.h
 * 
 */

#ifndef GUI_CIRC_H
#define GUI_CIRC_H

#ifdef __cplusplus
extern "C" {
#endif
/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
/*********************
 *      DEFINES
 *********************/
#define GUI_CIRC_OCT1_X(p) (p.x)
#define GUI_CIRC_OCT1_Y(p) (p.y)
#define GUI_CIRC_OCT2_X(p) (p.y)
#define GUI_CIRC_OCT2_Y(p) (p.x)
#define GUI_CIRC_OCT3_X(p) (-p.y)
#define GUI_CIRC_OCT3_Y(p) (p.x)
#define GUI_CIRC_OCT4_X(p) (-p.x)
#define GUI_CIRC_OCT4_Y(p) (p.y)
#define GUI_CIRC_OCT5_X(p) (-p.x)
#define GUI_CIRC_OCT5_Y(p) (-p.y)
#define GUI_CIRC_OCT6_X(p) (-p.y)
#define GUI_CIRC_OCT6_Y(p) (-p.x)
#define GUI_CIRC_OCT7_X(p) (p.y)
#define GUI_CIRC_OCT7_Y(p) (-p.x)
#define GUI_CIRC_OCT8_X(p) (p.x)
#define GUI_CIRC_OCT8_Y(p) (-p.y)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize the circle drawing
 * @param c pointer to a point. The coordinates will be calculated here
 * @param tmp point to a variable. It will store temporary data
 * @param radius radius of the circle
 */
void circ_init(point_t * c, coord_t * tmp, coord_t radius);

/**
 * Test the circle drawing is ready or not
 * @param c same as in circ_init
 * @return true if the circle is not ready yet
 */
bool circ_cont(point_t * c);

/**
 * Get the next point from the circle
 * @param c same as in circ_init. The next point stored here.
 * @param tmp same as in circ_init.
 */
void circ_next(point_t * c, coord_t * tmp);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
