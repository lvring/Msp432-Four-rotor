/**
 * @file list.h
 * 
 */

#ifndef GUI_LIST_H
#define GUI_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_LIST != 0

/*Testing of dependencies*/
#if GUI_USE_PAGE == 0
#error "list: page is required. Enable it in conf.h (GUI_USE_PAGE  1) "
#endif

#if GUI_USE_BTN == 0
#error "list: btn is required. Enable it in conf.h (GUI_USE_BTN  1) "
#endif

#if GUI_USE_LABEL == 0
#error "list: label is required. Enable it in conf.h (GUI_USE_LABEL  1) "
#endif


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
/*Data of list*/
typedef struct
{
    page_ext_t page; /*Ext. of ancestor*/
    /*New data for this type */
    uint16_t anim_time;                          /*Scroll animation time*/
    style_t *styles_btn[GUI_BTN_STATE_NUM];    /*Styles of the list element buttons*/
    style_t *style_img;                       /*Style of the list element images on buttons*/
}list_ext_t;

typedef enum {
    GUI_LIST_STYLE_BG,
    GUI_LIST_STYLE_SCRL,
    GUI_LIST_STYLE_SB,
    GUI_LIST_STYLE_BTN_REL,
    GUI_LIST_STYLE_BTN_PR,
    GUI_LIST_STYLE_BTN_TGL_REL,
    GUI_LIST_STYLE_BTN_TGL_PR,
    GUI_LIST_STYLE_BTN_INA,
}list_style_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a list objects
 * @param par pointer to an object, it will be the parent of the new list
 * @param copy pointer to a list object, if not NULL then the new object will be copied from it
 * @return pointer to the created list
 */
obj_t * list_create(obj_t * par, obj_t * copy);

/*======================
 * Add/remove functions
 *=====================*/

/**
 * Add a list element to the list
 * @param list pointer to list object
 * @param img_fn file name of an image before the text (NULL if unused)
 * @param txt text of the list element (NULL if unused)
 * @param rel_action pointer to release action function (like with btn)
 * @return pointer to the new list element which can be customized (a button)
 */
obj_t * list_add(obj_t * list, const void * img_src, const char * txt, action_t rel_action);

/*=====================
 * Setter functions
 *====================*/

/**
 * Set scroll animation duration on 'list_up()' 'list_down()' 'list_focus()'
 * @param list pointer to a list object
 * @param anim_time duration of animation [ms]
 */
void list_set_anim_time(obj_t *list, uint16_t anim_time);

/**
 * Set the scroll bar mode of a list
 * @param list pointer to a list object
 * @param sb_mode the new mode from 'page_sb_mode_t' enum
 */
static inline void list_set_sb_mode(obj_t * list, sb_mode_t mode)
{
    page_set_sb_mode(list, mode);
}

/**
 * Set a style of a list
 * @param list pointer to a list object
 * @param type which style should be set
 * @param style pointer to a style
 */
void list_set_style(obj_t *list, list_style_t type, style_t *style);

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the text of a list element
 * @param btn pointer to list element
 * @return pointer to the text
 */
const char * list_get_btn_text(obj_t * btn);
/**
 * Get the label object from a list element
 * @param btn pointer to a list element (button)
 * @return pointer to the label from the list element or NULL if not found
 */
obj_t * list_get_btn_label(obj_t * btn);

/**
 * Get the image object from a list element
 * @param btn pointer to a list element (button)
 * @return pointer to the image from the list element or NULL if not found
 */
obj_t * list_get_btn_img(obj_t * btn);

/**
 * Get scroll animation duration
 * @param list pointer to a list object
 * @return duration of animation [ms]
 */
uint16_t list_get_anim_time(obj_t *list);

/**
 * Get the scroll bar mode of a list
 * @param list pointer to a list object
 * @return scrollbar mode from 'page_sb_mode_t' enum
 */
static inline sb_mode_t list_get_sb_mode(obj_t * list)
{
    return page_get_sb_mode(list);
}

/**
 * Get a style of a list
 * @param list pointer to a list object
 * @param type which style should be get
 * @return style pointer to a style
 *  */
style_t * list_get_style(obj_t *list, list_style_t type);

/*=====================
 * Other functions
 *====================*/

/**
 * Move the list elements up by one
 * @param list pointer a to list object
 */
void list_up(obj_t * list);
/**
 * Move the list elements down by one
 * @param list pointer to a list object
 */
void list_down(obj_t * list);

/**
 * Focus on a list button. It ensures that the button will be visible on the list.
 * @param btn pointer to a list button to focus
 * @param anim_en true: scroll with animation, false: without animation
 */
void list_focus(obj_t *btn, bool anim_en);

/**********************
 *      MACROS
 **********************/

#endif  /*GUI_USE_LIST*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /*GUI_LIST_H*/
