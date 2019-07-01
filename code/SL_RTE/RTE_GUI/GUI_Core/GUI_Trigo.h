/**
 * @file trig.h
 * Basic trigonometric integer functions
 */

#ifndef GUI_TRIGO_H
#define GUI_TRIGO_H

#ifdef __cplusplus
extern "C" {
#endif


/*********************
 *      INCLUDES
 *********************/
#include <stdint.h>

/*********************
 *      DEFINES
 *********************/
#define GUI_TRIGO_SIN_MAX    32767
#define GUI_TRIGO_SHIFT      15      /* >> GUI_TRIGO_SHIFT to normalize*/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Return with sinus of an angle
 * @param angle
 * @return sinus of 'angle'. sin(-90) = -32767, sin(90) = 32767
 */
int16_t trigo_sin(int16_t angle);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
