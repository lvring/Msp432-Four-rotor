/**
 * @file win.h
 * 
 */

#ifndef GUI_WIN_H
#define GUI_WIN_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_WIN != 0

/*Testing of dependencies*/
#if GUI_USE_BTN == 0
#error "win: btn is required. Enable it in conf.h (GUI_USE_BTN  1) "
#endif

#if GUI_USE_LABEL == 0
#error "win: label is required. Enable it in conf.h (GUI_USE_LABEL  1) "
#endif

#if GUI_USE_IMG == 0
#error "win: img is required. Enable it in conf.h (GUI_USE_IMG  1) "
#endif


#if GUI_USE_PAGE == 0
#error "win: page is required. Enable it in conf.h (GUI_USE_PAGE  1) "
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/*Data of window*/
typedef struct
{
    /*Ext. of ancestor*/
    /*New data for this type */
    obj_t * page;                /*Pointer to a page which holds the content*/
    obj_t * header;              /*Pointer to the header container of the window*/
    obj_t * title;               /*Pointer to the title label of the window*/
    style_t * style_header;      /*Style of the header container*/
    style_t * style_btn_rel;    /*Control button releases style*/
    style_t * style_btn_pr;     /*Control button pressed style*/
    coord_t btn_size;               /*Size of the control buttons (square)*/
}win_ext_t;

typedef enum {
    GUI_WIN_STYLE_BG,
    GUI_WIN_STYLE_CONTENT_BG,
    GUI_WIN_STYLE_CONTENT_SCRL,
    GUI_WIN_STYLE_SB,
    GUI_WIN_STYLE_HEADER,
    GUI_WIN_STYLE_BTN_REL,
    GUI_WIN_STYLE_BTN_PR,
}win_style_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a window objects
 * @param par pointer to an object, it will be the parent of the new window
 * @param copy pointer to a window object, if not NULL then the new object will be copied from it
 * @return pointer to the created window
 */
obj_t * win_create(obj_t * par, obj_t * copy);


/*======================
 * Add/remove functions
 *=====================*/

/**
 * Add control button to the header of the window
 * @param win pointer to a window object
 * @param img_src an image source ('img_t' variable, path to file or a symbol)
 * @param rel_action a function pointer to call when the button is released
 * @return pointer to the created button object
 */
obj_t * win_add_btn(obj_t * win, const void * img_src, action_t rel_action);

/*=====================
 * Setter functions
 *====================*/

/**
 * A release action which can be assigned to a window control button to close it
 * @param btn pointer to the released button
 * @return always GUI_ACTION_RES_INV because the button is deleted with the window
 */
res_t win_close_action(obj_t * btn);

/**
 * Set the title of a window
 * @param win pointer to a window object
 * @param title string of the new title
 */
void win_set_title(obj_t * win, const char * title);

/**
 * Set the control button size of a window
 * @param win pointer to a window object
 * @return control button size
 */
void win_set_btn_size(obj_t * win, coord_t size);

/**
 * Set the scroll bar mode of a window
 * @param win pointer to a window object
 * @param sb_mode the new scroll bar mode from  'sb_mode_t'
 */
void win_set_sb_mode(obj_t *win, sb_mode_t sb_mode);

/**
 * Set the layout of the window
 * @param win pointer to a window object
 * @param layout the layout from 'layout_t'
 */
void win_set_layout(obj_t *win, layout_t layout);

/**
 * Set a style of a window
 * @param win pointer to a window object
 * @param type which style should be set
 * @param style pointer to a style
 */
void win_set_style(obj_t *win, win_style_t type, style_t *style);

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the title of a window
 * @param win pointer to a window object
 * @return title string of the window
 */
const char * win_get_title(obj_t * win);

/**
 * Get the control button size of a window
 * @param win pointer to a window object
 * @return control button size
 */
coord_t win_get_btn_size(obj_t * win);

/**
 * Get the layout of a window
 * @param win pointer to a window object
 * @return the layout of the window (from 'layout_t')
 */
layout_t win_get_layout(obj_t *win);

/**
 * Get the scroll bar mode of a window
 * @param win pointer to a window object
 * @return the scroll bar mode of the window (from 'sb_mode_t')
 */
sb_mode_t win_get_sb_mode(obj_t *win);

/**
 * Get width of the content area (page scrollable) of the window
 * @param win pointer to a window object
 * @return the width of the content area
 */
coord_t win_get_width(obj_t * win);

/**
 * Get the pointer of a widow from one of  its control button.
 * It is useful in the action of the control buttons where only button is known.
 * @param ctrl_btn pointer to a control button of a window
 * @return pointer to the window of 'ctrl_btn'
 */
obj_t * win_get_from_btn(obj_t * ctrl_btn);

/**
 * Get a style of a window
 * @param win pointer to a button object
 * @param type which style window be get
 * @return style pointer to a style
 */
style_t * win_get_style(obj_t *win, win_style_t type);

/*=====================
 * Other functions
 *====================*/

/**
 * Focus on an object. It ensures that the object will be visible in the window.
 * @param win pointer to a window object
 * @param obj pointer to an object to focus (must be in the window)
 * @param anim_time scroll animation time in milliseconds (0: no animation)
 */
void win_focus(obj_t * win, obj_t * obj, uint16_t anim_time);

/**********************
 *      MACROS
 **********************/

#endif /*GUI_USE_WIN*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*GUI_WIN_H*/
