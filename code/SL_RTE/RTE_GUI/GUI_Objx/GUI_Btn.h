/**
 * @file btn.h
 * 
 */

#ifndef GUI_BTN_H
#define GUI_BTN_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_BTN != 0

/*Testing of dependencies*/
#if GUI_USE_CONT == 0
#error "btn: cont is required. Enable it in conf.h (GUI_USE_CONT  1) "
#endif


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/*Button states*/
typedef enum
{
    GUI_BTN_STATE_REL,
    GUI_BTN_STATE_PR,
    GUI_BTN_STATE_TGL_REL,
    GUI_BTN_STATE_TGL_PR,
    GUI_BTN_STATE_INA,
    GUI_BTN_STATE_NUM,
}btn_state_t;

typedef enum
{
    GUI_BTN_ACTION_CLICK,
    GUI_BTN_ACTION_PR,
    GUI_BTN_ACTION_LONG_PR,
    GUI_BTN_ACTION_LONG_PR_REPEAT,
    GUI_BTN_ACTION_NUM,
}btn_action_t;

/*Data of button*/
typedef struct
{
	cont_ext_t cont; /*Ext. of ancestor*/
	/*New data for this type */
	action_t actions[GUI_BTN_ACTION_NUM];
	style_t * styles[GUI_BTN_STATE_NUM];        /*Styles in each state*/

    btn_state_t state;                         /*Current state of the button from 'btn_state_t' enum*/
    uint8_t toggle :1;                            /*1: Toggle enabled*/
    uint8_t long_pr_action_executed :1;           /*1: Long press action executed (Handled by the library)*/
}btn_ext_t;

/*Styles*/
typedef enum {
    GUI_BTN_STYLE_REL,
    GUI_BTN_STYLE_PR,
    GUI_BTN_STYLE_TGL_REL,
    GUI_BTN_STYLE_TGL_PR,
    GUI_BTN_STYLE_INA,
}btn_style_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a button objects
 * @param par pointer to an object, it will be the parent of the new button
 * @param copy pointer to a button object, if not NULL then the new object will be copied from it
 * @return pointer to the created button
 */
obj_t * btn_create(obj_t * par, obj_t * copy);

/*=====================
 * Setter functions
 *====================*/

/**
 * Enable the toggled states. On release the button will change from/to toggled state.
 * @param btn pointer to a button object
 * @param tgl true: enable toggled states, false: disable
 */
void btn_set_toggle(obj_t * btn, bool tgl);

/**
 * Set the state of the button
 * @param btn pointer to a button object
 * @param state the new state of the button (from btn_state_t enum)
 */
void btn_set_state(obj_t * btn, btn_state_t state);

/**
 * Toggle the state of the button (ON->OFF, OFF->ON)
 * @param btn pointer to a button object
 */
void btn_toggle(obj_t * btn);

/**
 * Set a function to call when the button event happens
 * @param btn pointer to a button object
 * @param action type of event form 'action_t' (press, release, long press, long press repeat)
 */
void btn_set_action(obj_t * btn, btn_action_t type, action_t action);

/**
 * Set the layout on a button
 * @param btn pointer to a button object
 * @param layout a layout from 'cont_layout_t'
 */
static inline void btn_set_layout(obj_t * btn, layout_t layout)
{
    cont_set_layout(btn, layout);
}

/**
 * Enable the horizontal or vertical fit.
 * The button size will be set to involve the children horizontally or vertically.
 * @param btn pointer to a button object
 * @param hor_en true: enable the horizontal fit
 * @param ver_en true: enable the vertical fit
 */
static inline void btn_set_fit(obj_t * btn, bool hor_en, bool ver_en)
{
    cont_set_fit(btn, hor_en, ver_en);
}

/**
 * Set a style of a button.
 * @param btn pointer to button object
 * @param type which style should be set
 * @param style pointer to a style
 *  */
void btn_set_style(obj_t * btn, btn_style_t type, style_t *style);

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the current state of the button
 * @param btn pointer to a button object
 * @return the state of the button (from btn_state_t enum)
 */
btn_state_t btn_get_state(obj_t * btn);

/**
 * Get the toggle enable attribute of the button
 * @param btn pointer to a button object
 * @return ture: toggle enabled, false: disabled
 */
bool btn_get_toggle(obj_t * btn);

/**
 * Get the release action of a button
 * @param btn pointer to a button object
 * @return pointer to the release action function
 */
action_t btn_get_action(obj_t * btn, btn_action_t type);

/**
 * Get the layout of a button
 * @param btn pointer to button object
 * @return the layout from 'cont_layout_t'
 */
static inline layout_t btn_get_layout(obj_t * btn)
{
    return cont_get_layout(btn);
}

/**
 * Get horizontal fit enable attribute of a button
 * @param btn pointer to a button object
 * @return true: horizontal fit is enabled; false: disabled
 */
static inline bool btn_get_hor_fit(obj_t * btn)
{
    return cont_get_hor_fit(btn);
}

/**
 * Get vertical fit enable attribute of a container
 * @param btn pointer to a button object
 * @return true: vertical fit is enabled; false: disabled
 */
static inline bool btn_get_ver_fit(obj_t * btn)
{
    return cont_get_ver_fit(btn);
}

/**
 * Get style of a button.
 * @param btn pointer to button object
 * @param type which style should be get
 * @return style pointer to the style
 *  */
style_t * btn_get_style(obj_t * btn, btn_style_t type);

/**********************
 *      MACROS
 **********************/

#endif  /*GUI_USE_BUTTON*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /*GUI_BTN_H*/
