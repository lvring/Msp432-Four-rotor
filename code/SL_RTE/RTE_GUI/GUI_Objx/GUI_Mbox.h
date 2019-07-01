/**
 * @file mbox.h
 * 
 */

#ifndef GUI_MBOX_H
#define GUI_MBOX_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_MBOX != 0

/*Testing of dependencies*/
#if GUI_USE_CONT == 0
#error "mbox: cont is required. Enable it in conf.h (GUI_USE_CONT  1) "
#endif

#if GUI_USE_BTNM == 0
#error "mbox: btnm is required. Enable it in conf.h (GUI_USE_BTNM  1) "
#endif

#if GUI_USE_LABEL == 0
#error "mbox: label is required. Enable it in conf.h (GUI_USE_LABEL  1) "
#endif



/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/*Data of message box*/
typedef struct
{
    cont_ext_t bg; /*Ext. of ancestor*/
    /*New data for this type */
    obj_t *text;             /*Text of the message box*/
    obj_t *btnm;            /*Button matrix for the buttons*/
    uint16_t anim_time;         /*Duration of close animation [ms] (0: no animation)*/
}mbox_ext_t;

typedef enum {
    GUI_MBOX_STYLE_BG,
    GUI_MBOX_STYLE_BTN_BG,
    GUI_MBOX_STYLE_BTN_REL,
    GUI_MBOX_STYLE_BTN_PR,
    GUI_MBOX_STYLE_BTN_TGL_REL,
    GUI_MBOX_STYLE_BTN_TGL_PR,
    GUI_MBOX_STYLE_BTN_INA,
}mbox_style_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a message box objects
 * @param par pointer to an object, it will be the parent of the new message box
 * @param copy pointer to a message box object, if not NULL then the new object will be copied from it
 * @return pointer to the created message box
 */
obj_t * mbox_create(obj_t * par, obj_t * copy);

/*======================
 * Add/remove functions
 *=====================*/

/**
 * Add button to the message box
 * @param mbox pointer to message box object
 * @param btn_map button descriptor (button matrix map).
 *                E.g.  a const char *txt[] = {"ok", "close", ""} (Can not be local variable)
 * @param action a function which will be called when a button is released
 */
void mbox_add_btns(obj_t * mbox, const char **btn_map, btnm_action_t action);

/*=====================
 * Setter functions
 *====================*/

/**
 * Set the text of the message box
 * @param mbox pointer to a message box
 * @param txt a '\0' terminated character string which will be the message box text
 */
void mbox_set_text(obj_t * mbox, const char * txt);

/**
 * Stop the action to call when button is released
 * @param mbox pointer to a message box object
 * @param pointer to an 'btnm_action_t' action
 */
void mbox_set_action(obj_t * mbox, btnm_action_t action);

/**
 * Set animation duration
 * @param mbox pointer to a message box object
 * @param anim_time animation length in  milliseconds (0: no animation)
 */
void mbox_set_anim_time(obj_t * mbox, uint16_t anim_time);

/**
 * Automatically delete the message box after a given time
 * @param mbox pointer to a message box object
 * @param delay a time (in milliseconds) to wait before delete the message box
 */
void mbox_start_auto_close(obj_t * mbox, uint16_t delay);

/**
 * Stop the auto. closing of message box
 * @param mbox pointer to a message box object
 */
void mbox_stop_auto_close(obj_t * mbox);

/**
 * Set a style of a message box
 * @param mbox pointer to a message box object
 * @param type which style should be set
 * @param style pointer to a style
 */
void mbox_set_style(obj_t *mbox, mbox_style_t type, style_t *style);

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the text of the message box
 * @param mbox pointer to a message box object
 * @return pointer to the text of the message box
 */
const char * mbox_get_text(obj_t * mbox);

/**
 * Get the message box object from one of its button.
 * It is useful in the button release actions where only the button is known
 * @param btn pointer to a button of a message box
 * @return pointer to the button's message box
 */
obj_t * mbox_get_from_btn(obj_t * btn);

/**
 * Get the animation duration (close animation time)
 * @param mbox pointer to a message box object
 * @return animation length in  milliseconds (0: no animation)
 */
uint16_t mbox_get_anim_time(obj_t * mbox);


/**
 * Get a style of a message box
 * @param mbox pointer to a message box object
 * @param type which style should be get
 * @return style pointer to a style
 */
style_t * mbox_get_style(obj_t *mbox, mbox_style_t type);

/**********************
 *      MACROS
 **********************/


#endif  /*GUI_USE_MBOX*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /*GUI_MBOX_H*/
