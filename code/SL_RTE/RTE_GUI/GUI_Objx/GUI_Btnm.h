/**
 * @file btnm.h
 * 
 */


#ifndef GUI_BTNM_H
#define GUI_BTNM_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_BTNM != 0


/*********************
 *      DEFINES
 *********************/

/*Control byte*/
#define GUI_BTNM_CTRL_CODE       0x80    /*The control byte has to begin (if present) with 0b10xxxxxx*/
#define GUI_BTNM_CTRL_MASK       0xC0
#define GUI_BTNM_WIDTH_MASK      0x07
#define GUI_BTNM_HIDE_MASK       0x08
#define GUI_BTNM_REPEAT_DISABLE_MASK     0x10
#define GUI_BTNM_INACTIVE_MASK   0x20

/**********************
 *      TYPEDEFS
 **********************/

/* Type of callback function which is called when a button is released or long pressed on the button matrix
 * Parameters: button matrix, text of the released button
 * return GUI_ACTION_RES_INV if  the button matrix is deleted else GUI_ACTION_RES_OK*/
typedef res_t (*btnm_action_t) (obj_t *, const char *txt);

/*Data of button matrix*/
typedef struct
{
    /*No inherited ext.*/ /*Ext. of ancestor*/
    /*New data for this type */
    const char ** map_p;                        /*Pointer to the current map*/
    area_t *button_areas;                    /*Array of areas of buttons*/
    btnm_action_t action;                    /*A function to call when a button is releases*/
    style_t *styles_btn[GUI_BTN_STATE_NUM];   /*Styles of buttons in each state*/
    uint16_t btn_cnt;                           /*Number of button in 'map_p'(Handled by the library)*/
    uint16_t btn_id_pr;                         /*Index of the currently pressed button (in `button_areas`) or GUI_BTNM_PR_NONE*/
    uint16_t btn_id_tgl;                        /*Index of the currently toggled button (in `button_areas`) or GUI_BTNM_PR_NONE */
    uint8_t toggle     :1;                      /*Enable toggling*/
}btnm_ext_t;

typedef enum {
    GUI_BTNM_STYLE_BG,
    GUI_BTNM_STYLE_BTN_REL,
    GUI_BTNM_STYLE_BTN_PR,
    GUI_BTNM_STYLE_BTN_TGL_REL,
    GUI_BTNM_STYLE_BTN_TGL_PR,
    GUI_BTNM_STYLE_BTN_INA,
}btnm_style_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a button matrix objects
 * @param par pointer to an object, it will be the parent of the new button matrix
 * @param copy pointer to a button matrix object, if not NULL then the new object will be copied from it
 * @return pointer to the created button matrix
 */
obj_t * btnm_create(obj_t * par, obj_t * copy);

/*=====================
 * Setter functions
 *====================*/

/**
 * Set a new map. Buttons will be created/deleted according to the map.
 * @param btnm pointer to a button matrix object
 * @param map pointer a string array. The last string has to be: "".
 *            Use "\n" to begin a new line.
 *            The first byte can be a control data:
 *             - bit 7: always 1
 *             - bit 6: always 0
 *             - bit 5: inactive (disabled)
 *             - bit 4: no repeat (on long press)
 *             - bit 3: hidden
 *             - bit 2..0: button relative width
 *             Example (practically use octal numbers): "\224abc": "abc" text with 4 width and no long press
 */
void btnm_set_map(obj_t * btnm, const char ** map);

/**
 * Set a new callback function for the buttons (It will be called when a button is released)
 * @param btnm: pointer to button matrix object
 * @param action pointer to a callback function
 */
void btnm_set_action(obj_t * btnm, btnm_action_t action);

/**
 * Enable or disable button toggling
 * @param btnm pointer to button matrix object
 * @param en true: enable toggling; false: disable toggling
 * @param id index of the currently toggled button (ignored if 'en' == false)
 */
void btnm_set_toggle(obj_t * btnm, bool en, uint16_t id);

/**
 * Set a style of a button matrix
 * @param btnm pointer to a button matrix object
 * @param type which style should be set
 * @param style pointer to a style
 */
void btnm_set_style(obj_t *btnm, btnm_style_t type, style_t *style);

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the current map of a button matrix
 * @param btnm pointer to a button matrix object
 * @return the current map
 */
const char ** btnm_get_map(obj_t * btnm);

/**
 * Get a the callback function of the buttons on a button matrix
 * @param btnm: pointer to button matrix object
 * @return pointer to the callback function
 */
btnm_action_t btnm_get_action(obj_t * btnm);


/**
 * Get the toggled button
 * @param btnm pointer to button matrix object
 * @return  index of the currently toggled button (0: if unset)
 */
uint16_t btnm_get_toggled(obj_t * btnm);

/**
 * Get a style of a button matrix
 * @param btnm pointer to a button matrix object
 * @param type which style should be get
 * @return style pointer to a style
 */
style_t * btnm_get_style(obj_t *btnm, btnm_style_t type);

/**********************
 *      MACROS
 **********************/

#endif /*GUI_USE_BTNM*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*GUI_BTNM_H*/
