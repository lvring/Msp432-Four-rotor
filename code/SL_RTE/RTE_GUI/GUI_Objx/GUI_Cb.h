/**
 * @file cb.h
 * 
 */

#ifndef GUI_CB_H
#define GUI_CB_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_CB != 0

/*Testing of dependencies*/
#if GUI_USE_BTN == 0
#error "cb: btn is required. Enable it in conf.h (GUI_USE_BTN  1) "
#endif

#if GUI_USE_LABEL == 0
#error "cb: label is required. Enable it in conf.h (GUI_USE_LABEL  1) "
#endif


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/*Data of check box*/
typedef struct
{
    btn_ext_t bg_btn; /*Ext. of ancestor*/
    /*New data for this type */
    obj_t * bullet;  /*Pointer to button*/
    obj_t * label;   /*Pointer to label*/
}cb_ext_t;

typedef enum {
    GUI_CB_STYLE_BG,
    GUI_CB_STYLE_BOX_REL,
    GUI_CB_STYLE_BOX_PR,
    GUI_CB_STYLE_BOX_TGL_REL,
    GUI_CB_STYLE_BOX_TGL_PR,
    GUI_CB_STYLE_BOX_INA,
}cb_style_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a check box objects
 * @param par pointer to an object, it will be the parent of the new check box
 * @param copy pointer to a check box object, if not NULL then the new object will be copied from it
 * @return pointer to the created check box
 */
obj_t * cb_create(obj_t * par, obj_t * copy);

/*=====================
 * Setter functions
 *====================*/

/**
 * Set the text of a check box
 * @param cb pointer to a check box
 * @param txt the text of the check box
 */
void cb_set_text(obj_t * cb, const char * txt);

/**
 * Set the state of the check box
 * @param cb pointer to a check box object
 * @param checked true: make the check box checked; false: make it unchecked
 */
static inline void cb_set_checked(obj_t * cb, bool checked)
{
    btn_set_state(cb, checked ? GUI_BTN_STATE_TGL_REL : GUI_BTN_STATE_REL);
}

/**
 * Make the check box inactive (disabled)
 * @param cb pointer to a check box object
 */
static inline void cb_set_inactive(obj_t * cb)
{
    btn_set_state(cb, GUI_BTN_STATE_INA);
}

/**
 * Set a function to call when the check box is clicked
 * @param cb pointer to a check box object
 */
static inline void cb_set_action(obj_t * cb, action_t action)
{
    btn_set_action(cb, GUI_BTN_ACTION_CLICK, action);
}


/**
 * Set a style of a check box
 * @param cb pointer to check box object
 * @param type which style should be set
 * @param style pointer to a style
 *  */
void cb_set_style(obj_t * cb, cb_style_t type, style_t *style);

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the text of a check box
 * @param cb pointer to check box object
 * @return pointer to the text of the check box
 */
const char * cb_get_text(obj_t * cb);

/**
 * Get the current state of the check box
 * @param cb pointer to a check box object
 * @return true: checked; false: not checked
 */
static inline bool cb_is_checked(obj_t * cb)
{
    return btn_get_state(cb) == GUI_BTN_STATE_REL ? false : true;
}

/**
 * Get the action of a check box
 * @param cb pointer to a button object
 * @return pointer to the action function
 */
static inline action_t cb_get_action(obj_t * cb)
{
    return btn_get_action(cb, GUI_BTN_ACTION_CLICK);
}


/**
 * Get a style of a button
 * @param cb pointer to check box object
 * @param type which style should be get
 * @return style pointer to the style
 *  */
style_t * cb_get_style(obj_t * cb, cb_style_t type);

/**********************
 *      MACROS
 **********************/

#endif  /*GUI_USE_CB*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /*GUI_CB_H*/
