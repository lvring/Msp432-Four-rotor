/**
 * @file page.h
 * 
 */

#ifndef GUI_PAGE_H
#define GUI_PAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_PAGE != 0

/*Testing of dependencies*/
#if GUI_USE_CONT == 0
#error "page: cont is required. Enable it in conf.h (GUI_USE_CONT  1) "
#endif


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/*Scrollbar modes: shows when should the scrollbars be visible*/
typedef enum
{
    GUI_SB_MODE_OFF,    /*Never show scrollbars*/
    GUI_SB_MODE_ON,     /*Always show scrollbars*/
    GUI_SB_MODE_DRAG,   /*Show scrollbars when page is being dragged*/
    GUI_SB_MODE_AUTO,   /*Show scrollbars when the scrollable container is large enough to be scrolled*/
}sb_mode_t;

/*Data of page*/
typedef struct
{
    cont_ext_t bg; /*Ext. of ancestor*/
    /*New data for this type */
    obj_t * scrl;            /*The scrollable object on the background*/
    action_t rel_action;     /*Function to call when the page is released*/
    action_t pr_action;      /*Function to call when the page is pressed*/
    struct {
        style_t *style;          /*Style of scrollbars*/
        area_t hor_area;            /*Horizontal scrollbar area relative to the page. (Handled by the library) */
        area_t ver_area;            /*Vertical scrollbar area relative to the page (Handled by the library)*/
        uint8_t hor_draw :1;        /*1: horizontal scrollbar is visible now (Handled by the library)*/
        uint8_t ver_draw :1;        /*1: vertical scrollbar is visible now (Handled by the library)*/
        uint8_t mode     :3;        /*Scrollbar visibility from 'page_sb_mode_t'*/
    }sb;
}page_ext_t;

typedef enum {
    GUI_PAGE_STYLE_BG,
    GUI_PAGE_STYLE_SCRL,
    GUI_PAGE_STYLE_SB,
}page_style_t;


/**********************
 * GLOBAL PROTOTYPES
 **********************/


/**
 * Create a page objects
 * @param par pointer to an object, it will be the parent of the new page
 * @param copy pointer to a page object, if not NULL then the new object will be copied from it
 * @return pointer to the created page
 */
obj_t * page_create(obj_t * par, obj_t * copy);

/**
 * Get the scrollable object of a page
 * @param page pointer to a page object
 * @return pointer to a container which is the scrollable part of the page
 */
obj_t * page_get_scrl(obj_t * page);

/*=====================
 * Setter functions
 *====================*/

/**
 * Set a release action for the page
 * @param page pointer to a page object
 * @param rel_action a function to call when the page is released
 */
void page_set_rel_action(obj_t * page, action_t rel_action);

/**
 * Set a press action for the page
 * @param page pointer to a page object
 * @param pr_action a function to call when the page is pressed
 */
void page_set_pr_action(obj_t * page, action_t pr_action);

/**
 * Set the scroll bar mode on a page
 * @param page pointer to a page object
 * @param sb.mode the new mode from 'page_sb.mode_t' enum
 */
void page_set_sb_mode(obj_t * page, sb_mode_t sb_mode);

/**
 * Set the fit attribute of the scrollable part of a page.
 * It means it can set its size automatically to involve all children.
 * (Can be set separately horizontally and vertically)
 * @param page pointer to a page object
 * @param hor_en true: enable horizontal fit
 * @param ver_en true: enable vertical fit
 */
static inline void page_set_scrl_fit(obj_t *page, bool hor_en, bool ver_en)
{
    cont_set_fit(page_get_scrl(page), hor_en, ver_en);
}

/**
 * Set width of the scrollable part of a page
 * @param page pointer to a page object
 * @param w the new width of the scrollable (it ha no effect is horizontal fit is enabled)
 */
static inline void page_set_scrl_width(obj_t *page, coord_t w)
{
    obj_set_width(page_get_scrl(page), w);
}

/**
 * Set height of the scrollable part of a page
 * @param page pointer to a page object
 * @param h the new height of the scrollable (it ha no effect is vertical fit is enabled)
 */
static inline void page_set_scrl_height(obj_t *page, coord_t h)
{
    obj_set_height(page_get_scrl(page), h);

}

/**
* Set the layout of the scrollable part of the page
* @param page pointer to a page object
* @param layout a layout from 'cont_layout_t'
*/
static inline void page_set_scrl_layout(obj_t * page, layout_t layout)
{
    cont_set_layout(page_get_scrl(page), layout);
}

/**
 * Set a style of a page
 * @param page pointer to a page object
 * @param type which style should be set
 * @param style pointer to a style
 */
void page_set_style(obj_t *page, page_style_t type, style_t *style);

/*=====================
 * Getter functions
 *====================*/

/**
 * Set the scroll bar mode on a page
 * @param page pointer to a page object
 * @return the mode from 'page_sb.mode_t' enum
 */
sb_mode_t page_get_sb_mode(obj_t * page);

/**
 * Get width of the scrollable part of a page
 * @param page pointer to a page object
 * @return the width of the scrollable
 */
static inline coord_t page_get_scrl_width(obj_t *page)
{
    return obj_get_width(page_get_scrl(page));
}

/**
 * Get height of the scrollable part of a page
 * @param page pointer to a page object
 * @return the height of the scrollable
 */
static inline coord_t page_get_scrl_height(obj_t *page)
{
    return obj_get_height(page_get_scrl(page));
}

/**
* Get the layout of the scrollable part of a page
* @param page pointer to page object
* @return the layout from 'cont_layout_t'
*/
static inline layout_t page_get_scrl_layout(obj_t * page)
{
    return cont_get_layout(page_get_scrl(page));
}

/**
* Get horizontal fit attribute of the scrollable part of a page
* @param page pointer to a page object
* @return true: horizontal fit is enabled; false: disabled
*/
static inline bool page_get_scrl_hor_fit(obj_t * page)
{
    return cont_get_hor_fit(page_get_scrl(page));
}

/**
* Get vertical fit attribute of the scrollable part of a page
* @param page pointer to a page object
* @return true: vertical fit is enabled; false: disabled
*/
static inline bool page_get_scrl_fit_ver(obj_t * page)
{
    return cont_get_ver_fit(page_get_scrl(page));
}

/**
 * Get a style of a page
 * @param page pointer to page object
 * @param type which style should be get
 * @return style pointer to a style
 */
style_t * page_get_style(obj_t *page, page_style_t type);

/*=====================
 * Other functions
 *====================*/

/**
 * Glue the object to the page. After it the page can be moved (dragged) with this object too.
 * @param obj pointer to an object on a page
 * @param glue true: enable glue, false: disable glue
 */
void page_glue_obj(obj_t * obj, bool glue);

/**
 * Focus on an object. It ensures that the object will be visible on the page.
 * @param page pointer to a page object
 * @param obj pointer to an object to focus (must be on the page)
 * @param anim_time scroll animation time in milliseconds (0: no animation)
 */
void page_focus(obj_t * page, obj_t * obj, uint16_t anim_time);

/**********************
 *      MACROS
 **********************/

#endif  /*GUI_USE_PAGE*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /*GUI_PAGE_H*/
