/**
 * @file ddlist.h
 * 
 */

#ifndef GUI_DDLIST_H
#define GUI_DDLIST_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_DDLIST != 0

/*Testing of dependencies*/
#if GUI_USE_PAGE == 0
#error "ddlist: page is required. Enable it in conf.h (GUI_USE_PAGE  1) "
#endif

#if GUI_USE_LABEL == 0
#error "ddlist: label is required. Enable it in conf.h (GUI_USE_LABEL  1) "
#endif


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
/*Data of drop down list*/
typedef struct
{
    page_ext_t page; /*Ext. of ancestor*/
    /*New data for this type */
    obj_t *label;                     /*Label for the options*/
    style_t * sel_style;              /*Style of the selected option*/
    action_t action;                  /*Pointer to function to call when an option is selected*/
    uint16_t option_cnt;                 /*Number of options*/
    uint16_t sel_opt_id;                 /*Index of the current option*/
    uint16_t sel_opt_id_ori;             /*Store the original index on focus*/
    uint16_t anim_time;                  /*Open/Close animation time [ms]*/
    uint8_t opened :1;                   /*1: The list is opened*/
    coord_t fix_height;                   /*Height if the ddlist is opened. (0: auto-size)*/
}ddlist_ext_t;

typedef enum {
    GUI_DDLIST_STYLE_BG,
    GUI_DDLIST_STYLE_SEL,
    GUI_DDLIST_STYLE_SB,
}ddlist_style_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
/**
 * Create a drop down list objects
 * @param par pointer to an object, it will be the parent of the new drop down list
 * @param copy pointer to a drop down list object, if not NULL then the new object will be copied from it
 * @return pointer to the created drop down list
 */
obj_t * ddlist_create(obj_t * par, obj_t * copy);

/*=====================
 * Setter functions
 *====================*/

/**
 * Set the options in a drop down list from a string
 * @param ddlist pointer to drop down list object
 * @param options a string with '\n' separated options. E.g. "One\nTwo\nThree"
 */
void ddlist_set_options(obj_t * ddlist, const char * options);

/**
 * Set the selected option
 * @param ddlist pointer to drop down list object
 * @param sel_opt id of the selected option (0 ... number of option - 1);
 */
void ddlist_set_selected(obj_t * ddlist, uint16_t sel_opt);

/**
 * Set a function to call when a new option is chosen
 * @param ddlist pointer to a drop down list
 * @param action pointer to a call back function
 */
void ddlist_set_action(obj_t * ddlist, action_t action);

/**
 * Set the fix height for the drop down list
 * If 0 then the opened ddlist will be auto. sized else the set height will be applied.
 * @param ddlist pointer to a drop down list
 * @param h the height when the list is opened (0: auto size)
 */
void ddlist_set_fix_height(obj_t * ddlist, coord_t h);

/**
 * Enable or disable the horizontal fit to the content
 * @param ddlist pointer to a drop down list
 * @param fit en true: enable auto fit; false: disable auto fit
 */
void ddlist_set_hor_fit(obj_t * ddlist, bool fit_en);

/**
 * Set the scroll bar mode of a drop down list
 * @param ddlist pointer to a drop down list object
 * @param sb_mode the new mode from 'page_sb_mode_t' enum
 */
static inline void ddlist_set_sb_mode(obj_t * ddlist, sb_mode_t mode)
{
    page_set_sb_mode(ddlist, mode);
}

/**
 * Set the open/close animation time.
 * @param ddlist pointer to a drop down list
 * @param anim_time: open/close animation time [ms]
 */
void ddlist_set_anim_time(obj_t * ddlist, uint16_t anim_time);


/**
 * Set a style of a drop down list
 * @param ddlist pointer to a drop down list object
 * @param type which style should be set
 * @param style pointer to a style
 *  */
void ddlist_set_style(obj_t *ddlist, ddlist_style_t type, style_t *style);

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the options of a drop down list
 * @param ddlist pointer to drop down list object
 * @return the options separated by '\n'-s (E.g. "Option1\nOption2\nOption3")
 */
const char * ddlist_get_options(obj_t * ddlist);

/**
 * Get the selected option
 * @param ddlist pointer to drop down list object
 * @return id of the selected option (0 ... number of option - 1);
 */
uint16_t ddlist_get_selected(obj_t * ddlist);

/**
 * Get the current selected option as a string
 * @param ddlist pointer to ddlist object
 * @param buf pointer to an array to store the string
 */
void ddlist_get_selected_str(obj_t * ddlist, char * buf);

/**
 * Get the "option selected" callback function
 * @param ddlist pointer to a drop down list
 * @return  pointer to the call back function
 */
action_t ddlist_get_action(obj_t * ddlist);

/**
 * Get the fix height value.
 * @param ddlist pointer to a drop down list object
 * @return the height if the ddlist is opened (0: auto size)
 */
coord_t ddlist_get_fix_height(obj_t * ddlist);

/**
 * Get the scroll bar mode of a drop down list
 * @param ddlist pointer to a  drop down list object
 * @return scrollbar mode from 'page_sb_mode_t' enum
 */
static inline sb_mode_t ddlist_get_sb_mode(obj_t * ddlist)
{
    return page_get_sb_mode(ddlist);
}

/**
 * Get the open/close animation time.
 * @param ddlist pointer to a drop down list
 * @return open/close animation time [ms]
 */
uint16_t ddlist_get_anim_time(obj_t * ddlist);

/**
 * Get a style of a drop down list
 * @param ddlist pointer to a drop down list object
 * @param type which style should be get
 * @return style pointer to a style
 */
style_t * ddlist_get_style(obj_t *ddlist, ddlist_style_t type);

/*=====================
 * Other functions
 *====================*/

/**
 * Open the drop down list with or without animation
 * @param ddlist pointer to drop down list object
 * @param anim_en true: use animation; false: not use animations
 */
void ddlist_open(obj_t * ddlist, bool anim);

/**
 * Close (Collapse) the drop down list
 * @param ddlist pointer to drop down list object
 * @param anim true: use animation; false: not use animations
 */
void ddlist_close_en(obj_t * ddlist, bool anim);

/**********************
 *      MACROS
 **********************/

#endif  /*GUI_USE_DDLIST*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /*GUI_DDLIST_H*/
