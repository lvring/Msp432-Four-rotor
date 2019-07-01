/**
 * @file tabview.h
 * 
 */

#ifndef GUI_TABVIEW_H
#define GUI_TABVIEW_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_TABVIEW != 0


/*Testing of dependencies*/
#if GUI_USE_BTNM == 0
#error "tabview: btnm is required. Enable it in conf.h (GUI_USE_BTNM  1) "
#endif

#if GUI_USE_PAGE == 0
#error "tabview: page is required. Enable it in conf.h (GUI_USE_PAGE  1) "
#endif


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/* parametes: pointer to a tabview object, tab_id*/
typedef void (*tabview_action_t)(obj_t *, uint16_t);

/*Data of tab*/
typedef struct
{
   /*Ext. of ancestor*/
    /*New data for this type */
    obj_t * btns;
    obj_t * indic;
    obj_t * content;   /*A rectangle to show the current tab*/
    const char ** tab_name_ptr;
    point_t point_last;
    uint16_t tab_cur;
    uint16_t tab_cnt;
    uint16_t anim_time;
    uint8_t slide_enable :1;    /*1: enable horizontal sliding by touch pad*/
    uint8_t draging :1;
    uint8_t drag_hor :1;
    tabview_action_t tab_load_action;
}tabview_ext_t;

typedef enum {
    GUI_TABVIEW_STYLE_BG,
    GUI_TABVIEW_STYLE_INDIC,
    GUI_TABVIEW_STYLE_BTN_BG,
    GUI_TABVIEW_STYLE_BTN_REL,
    GUI_TABVIEW_STYLE_BTN_PR,
    GUI_TABVIEW_STYLE_BTN_TGL_REL,
    GUI_TABVIEW_STYLE_BTN_TGL_PR,
}tabview_style_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/


/**
 * Create a Tab view object
 * @param par pointer to an object, it will be the parent of the new tab
 * @param copy pointer to a tab object, if not NULL then the new object will be copied from it
 * @return pointer to the created tab
 */
obj_t * tabview_create(obj_t * par, obj_t * copy);

/*======================
 * Add/remove functions
 *=====================*/

/**
 * Add a new tab with the given name
 * @param tabview pointer to Tab view object where to ass the new tab
 * @param name the text on the tab button
 * @return pointer to the created page object (page). You can create your content here
 */
obj_t * tabview_add_tab(obj_t * tabview, const char * name);

/*=====================
 * Setter functions
 *====================*/

/**
 * Set a new tab
 * @param tabview pointer to Tab view object
 * @param id index of a tab to load
 * @param anim_en true: set with sliding animation; false: set immediately
 */
void tabview_set_tab_act(obj_t * tabview, uint16_t id, bool anim_en);

/**
 * Set an action to call when a tab is loaded (Good to create content only if required)
 * tabview_get_act() still gives the current (old) tab (to remove content from here)
 * @param tabview pointer to a tabview object
 * @param action pointer to a function to call when a tab is loaded
 */
void tabview_set_tab_load_action(obj_t *tabview, tabview_action_t action);

/**
 * Enable horizontal sliding with touch pad
 * @param tabview pointer to Tab view object
 * @param en true: enable sliding; false: disable sliding
 */
void tabview_set_sliding(obj_t * tabview, bool en);

/**
 * Set the animation time of tab view when a new tab is loaded
 * @param tabview pointer to Tab view object
 * @param anim_time time of animation in milliseconds
 */
void tabview_set_anim_time(obj_t * tabview, uint16_t anim_time);

/**
 * Set the style of a tab view
 * @param tabview pointer to a tan view object
 * @param type which style should be set
 * @param style pointer to the new style
 */
void tabview_set_style(obj_t *tabview, tabview_style_t type, style_t *style);

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the index of the currently active tab
 * @param tabview pointer to Tab view object
 * @return the active tab index
 */
uint16_t tabview_get_tab_act(obj_t * tabview);

/**
 * Get the number of tabs
 * @param tabview pointer to Tab view object
 * @return tab count
 */
uint16_t tabview_get_tab_count(obj_t * tabview);
/**
 * Get the page (content area) of a tab
 * @param tabview pointer to Tab view object
 * @param id index of the tab (>= 0)
 * @return pointer to page (page) object
 */
obj_t * tabview_get_tab(obj_t * tabview, uint16_t id);

/**
 * Get the tab load action
 * @param tabview pointer to a tabview object
 * @param return the current tab load action
 */
tabview_action_t tabview_get_tab_load_action(obj_t *tabview);

/**
 * Get horizontal sliding is enabled or not
 * @param tabview pointer to Tab view object
 * @return true: enable sliding; false: disable sliding
 */
bool tabview_get_sliding(obj_t * tabview);

/**
 * Get the animation time of tab view when a new tab is loaded
 * @param tabview pointer to Tab view object
 * @return time of animation in milliseconds
 */
uint16_t tabview_get_anim_time(obj_t * tabview);

/**
 * Get a style of a tab view
 * @param tabview pointer to a ab view object
 * @param type which style should be get
 * @return style pointer to a style
 */
style_t * tabview_get_style(obj_t *tabview, tabview_style_t type);


/**********************
 *      MACROS
 **********************/

#endif  /*GUI_USE_TABVIEW*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /*GUI_TABVIEW_H*/
