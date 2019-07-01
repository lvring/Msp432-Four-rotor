/**
 * @file roller.h
 * 
 */

#ifndef GUI_ROLLER_H
#define GUI_ROLLER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_ROLLER != 0

/*Testing of dependencies*/
#if GUI_USE_DDLIST == 0
#error "roller: ddlist is required. Enable it in conf.h (GUI_USE_DDLIST  1) "
#endif


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
/*Data of roller*/
typedef struct {
    ddlist_ext_t ddlist; /*Ext. of ancestor*/
    /*New data for this type */
}roller_ext_t;

typedef enum {
    GUI_ROLLER_STYLE_BG,
    GUI_ROLLER_STYLE_SEL,
}roller_style_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a roller object
 * @param par pointer to an object, it will be the parent of the new roller
 * @param copy pointer to a roller object, if not NULL then the new object will be copied from it
 * @return pointer to the created roller
 */
obj_t * roller_create(obj_t * par, obj_t * copy);

/*=====================
 * Setter functions
 *====================*/

/**
 * Set the options on a roller
 * @param roller pointer to roller object
 * @param options a string with '\n' separated options. E.g. "One\nTwo\nThree"
 */
static inline void roller_set_options(obj_t * roller, const char * options)
{
    ddlist_set_options(roller, options);
}

/**
 * Set the selected option
 * @param roller pointer to a roller object
 * @param sel_opt id of the selected option (0 ... number of option - 1);
 * @param anim_en true: set with animation; false set immediately
 */
void roller_set_selected(obj_t *roller, uint16_t sel_opt, bool anim_en);


/**
 * Set a function to call when a new option is chosen
 * @param roller pointer to a roller
 * @param action pointer to a callback function
 */
static inline void roller_set_action(obj_t * roller, action_t action)
{
    ddlist_set_action(roller, action);
}

/**
 * Set the height to show the given number of rows (options)
 * @param roller pointer to a roller object
 * @param row_cnt number of desired visible rows
 */
void roller_set_visible_row_count(obj_t *roller, uint8_t row_cnt);

/**
 * Enable or disable the horizontal fit to the content
 * @param roller pointer to a roller
 * @param fit en true: enable auto fit; false: disable auto fit
 */
static inline void roller_set_hor_fit(obj_t * roller, bool fit_en)
{
    ddlist_set_hor_fit(roller, fit_en);
}

/**
 * Set the open/close animation time.
 * @param roller pointer to a roller object
 * @param anim_time: open/close animation time [ms]
 */
static inline void roller_set_anim_time(obj_t *roller, uint16_t anim_time)
{
    ddlist_set_anim_time(roller, anim_time);
}


/**
 * Set a style of a roller
 * @param roller pointer to a roller object
 * @param type which style should be set
 * @param style pointer to a style
 */
void roller_set_style(obj_t *roller, roller_style_t type, style_t *style);

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the options of a roller
 * @param roller pointer to roller object
 * @return the options separated by '\n'-s (E.g. "Option1\nOption2\nOption3")
 */
static inline const char * roller_get_options(obj_t *roller)
{
    return ddlist_get_options(roller);
}

/**
 * Get the id of the selected option
 * @param roller pointer to a roller object
 * @return id of the selected option (0 ... number of option - 1);
 */
static inline uint16_t roller_get_selected(obj_t *roller)
{
    return ddlist_get_selected(roller);
}

/**
 * Get the current selected option as a string
 * @param roller pointer to roller object
 * @param buf pointer to an array to store the string
 */
static inline void roller_get_selected_str(obj_t * roller, char * buf)
{
    ddlist_get_selected_str(roller, buf);
}

/**
 * Get the "option selected" callback function
 * @param roller pointer to a roller
 * @return  pointer to the call back function
 */
static inline action_t roller_get_action(obj_t * roller)
{
    return ddlist_get_action(roller);
}

/**
 * Get the open/close animation time.
 * @param roller pointer to a roller
 * @return open/close animation time [ms]
 */
static inline uint16_t roller_get_anim_time(obj_t * roller)
{
    return ddlist_get_anim_time(roller);
}

/**
 * Get the auto width set attribute
 * @param roller pointer to a roller object
 * @return true: auto size enabled; false: manual width settings enabled
 */
bool roller_get_hor_fit(obj_t *roller);

/**
 * Get a style of a roller
 * @param roller pointer to a roller object
 * @param type which style should be get
 * @return style pointer to a style
 *  */
style_t * roller_get_style(obj_t *roller, roller_style_t type);

/**********************
 *      MACROS
 **********************/

#endif  /*GUI_USE_ROLLER*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /*GUI_ROLLER_H*/
