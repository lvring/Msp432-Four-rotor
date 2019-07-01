#ifndef __GUI_INDEV_H
#define __GUI_INDEV_H
#ifdef __cplusplus
extern "C" {
#endif
#include "RTE_Include.h"
/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize the display input device subsystem
 */
void indev_init(void);

/**
 * Get the currently processed input device. Can be used in action functions too.
 * @return pointer to the currently processed input device or NULL if no input device processing right now
 */
indev_t * indev_get_act(void);

/**
 * Reset one or all input devices
 * @param indev pointer to an input device to reset or NULL to reset all of them
 */
void indev_reset(indev_t * indev);

/**
 * Reset the long press state of an input device
 * @param indev_proc pointer to an input device
 */
void indev_reset_lpr(indev_t * indev);

/**
 * Enable input devices device by type
 * @param type Input device type
 * @param enable true: enable this type; false: disable this type
 */
void indev_enable(hal_indev_type_t type, bool enable);

/**
 * Set a cursor for a pointer input device (for GUI_INPUT_TYPE_POINTER and GUI_INPUT_TYPE_BUTTON)
 * @param indev pointer to an input device
 * @param cur_obj pointer to an object to be used as cursor
 */
void indev_set_cursor(indev_t *indev, obj_t *cur_obj);

#if GUI_USE_GROUP
/**
 * Set a destination group for a keypad input device (for GUI_INDEV_TYPE_KEYPAD)
 * @param indev pointer to an input device
 * @param group point to a group
 */
void indev_set_group(indev_t *indev, group_t *group);
#endif

/**
 * Set the an array of points for GUI_INDEV_TYPE_BUTTON.
 * These points will be assigned to the buttons to press a specific point on the screen
 * @param indev pointer to an input device
 * @param group point to a group
 */
void indev_set_button_points(indev_t *indev, point_t *points);

/**
 * Get the last point of an input device (for GUI_INDEV_TYPE_POINTER and GUI_INDEV_TYPE_BUTTON)
 * @param indev pointer to an input device
 * @param point pointer to a point to store the result
 */
void indev_get_point(indev_t * indev, point_t * point);

/**
 * Check if there is dragging with an input device or not (for GUI_INDEV_TYPE_POINTER and GUI_INDEV_TYPE_BUTTON)
 * @param indev pointer to an input device
 * @return true: drag is in progress
 */
bool indev_is_dragging(indev_t * indev);

/**
 * Get the vector of dragging of an input device (for GUI_INDEV_TYPE_POINTER and GUI_INDEV_TYPE_BUTTON)
 * @param indev pointer to an input device
 * @param point pointer to a point to store the vector
 */
void indev_get_vect(indev_t * indev, point_t * point);
/**
 * Get elapsed time since last press
 * @param indev pointer to an input device (NULL to get the overall smallest inactivity)
 * @return Elapsed ticks (milliseconds) since last press
 */
uint32_t indev_get_inactive_time(indev_t * indev);

/**
 * Do nothing until the next release
 * @param indev pointer to an input device
 */
void indev_wait_release(indev_t * indev);

/**********************
 *      MACROS
 **********************/
#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
