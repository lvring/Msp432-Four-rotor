/**
 * @file kb.h
 * 
 */

#ifndef GUI_KB_H
#define GUI_KB_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_KB != 0

/*Testing of dependencies*/
#if GUI_USE_BTNM == 0
#error "kb: btnm is required. Enable it in conf.h (GUI_USE_BTNM  1) "
#endif

#if GUI_USE_TA == 0
#error "kb: ta is required. Enable it in conf.h (GUI_USE_TA  1) "
#endif


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef enum {
    GUI_KB_MODE_TEXT,
    GUI_KB_MODE_NUM,
}kb_mode_t;


/*Data of keyboard*/
typedef struct {
    btnm_ext_t btnm;     /*Ext. of ancestor*/
    /*New data for this type */
    obj_t *ta;               /*Pointer to the assigned text area*/
    kb_mode_t mode;          /*Key map type*/
    uint8_t cursor_mng      :1; /*1: automatically show/hide cursor when a text area is assigned or left*/
    action_t  ok_action;     /*Called when the "Ok" button is clicked*/
    action_t  hide_action;  /*Called when the "Hide" button is clicked*/
}kb_ext_t;

typedef enum {
    GUI_KB_STYLE_BG,
    GUI_KB_STYLE_BTN_REL,
    GUI_KB_STYLE_BTN_PR,
    GUI_KB_STYLE_BTN_TGL_REL,
    GUI_KB_STYLE_BTN_TGL_PR,
    GUI_KB_STYLE_BTN_INA,
}kb_style_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a keyboard objects
 * @param par pointer to an object, it will be the parent of the new keyboard
 * @param copy pointer to a keyboard object, if not NULL then the new object will be copied from it
 * @return pointer to the created keyboard
 */
obj_t * kb_create(obj_t * par, obj_t * copy);

/*=====================
 * Setter functions
 *====================*/

/**
 * Assign a Text Area to the Keyboard. The pressed characters will be put there.
 * @param kb pointer to a Keyboard object
 * @param ta pointer to a Text Area object to write there
 */
void kb_set_ta(obj_t * kb, obj_t * ta);

/**
 * Set a new a mode (text or number map)
 * @param kb pointer to a Keyboard object
 * @param mode the mode from 'kb_mode_t'
 */
void kb_set_mode(obj_t * kb, kb_mode_t mode);

/**
 * Automatically hide or show the cursor of the current Text Area
 * @param kb pointer to a Keyboard object
 * @param en true: show cursor on the current text area, false: hide cursor
 */
void kb_set_cursor_manage(obj_t * kb, bool en);

/**
 * Set call back to call when the "Ok" button is pressed
 * @param kb pointer to Keyboard object
 * @param action a callback with 'action_t' type
 */
void kb_set_ok_action(obj_t * kb, action_t action);

/**
 * Set call back to call when the "Hide" button is pressed
 * @param kb pointer to Keyboard object
 * @param action a callback with 'action_t' type
 */
void kb_set_hide_action(obj_t * kb, action_t action);

/**
 * Set a new map for the keyboard
 * @param kb pointer to a Keyboard object
 * @param map pointer to a string array to describe the map.
 *            See 'btnm_set_map()' for more info.
 */
static inline void kb_set_map(obj_t *kb, const char ** map)
{
    btnm_set_map(kb, map);
}

/**
 * Set a style of a keyboard
 * @param kb pointer to a keyboard object
 * @param type which style should be set
 * @param style pointer to a style
 */
void kb_set_style(obj_t *kb, kb_style_t type, style_t *style);

/*=====================
 * Getter functions
 *====================*/

/**
 * Assign a Text Area to the Keyboard. The pressed characters will be put there.
 * @param kb pointer to a Keyboard object
 * @return pointer to the assigned Text Area object
 */
obj_t * kb_get_ta(obj_t * kb);

/**
 * Set a new a mode (text or number map)
 * @param kb pointer to a Keyboard object
 * @return the current mode from 'kb_mode_t'
 */
kb_mode_t kb_get_mode(obj_t * kb);

/**
 * Get the current cursor manage mode.
 * @param kb pointer to a Keyboard object
 * @return true: show cursor on the current text area, false: hide cursor
 */
bool kb_get_cursor_manage(obj_t * kb);

/**
 * Get the callback to call when the "Ok" button is pressed
 * @param kb pointer to Keyboard object
 * @return the ok callback
 */
action_t kb_get_ok_action(obj_t * kb);

/**
 * Get the callback to call when the "Hide" button is pressed
 * @param kb pointer to Keyboard object
 * @return the close callback
 */
action_t kb_get_hide_action(obj_t * kb);

/**
 * Get a style of a keyboard
 * @param kb pointer to a keyboard object
 * @param type which style should be get
 * @return style pointer to a style
 */
style_t * kb_get_style(obj_t *kb, kb_style_t type);

/**********************
 *      MACROS
 **********************/

#endif  /*GUI_USE_KB*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /*GUI_KB_H*/
