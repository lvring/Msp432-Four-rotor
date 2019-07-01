#ifndef __GUI_GROUP_H
#define __GUI_GROUP_H
#ifdef __cplusplus
extern "C" {
#endif
#include "RTE_Include.h"
/*********************
 *      DEFINES
 *********************/
/*Predefined keys to control the focused object via group_send(group, c)*/
/*For compatibility in signal function define the keys regardless to GUI_GROUP*/
#define GUI_GROUP_KEY_UP             17      /*0x11*/
#define GUI_GROUP_KEY_DOWN           18      /*0x12*/
#define GUI_GROUP_KEY_RIGHT          19      /*0x13*/
#define GUI_GROUP_KEY_LEFT           20      /*0x14*/
#define GUI_GROUP_KEY_ESC            27      /*0x1B*/
#define GUI_GROUP_KEY_ENTER          10      /*0x0A, '\n'*/
#define GUI_GROUP_KEY_NEXT           9       /*0x09, '\t'*/
#define GUI_GROUP_KEY_PREV           11      /*0x0B, '*/
#define GUI_GROUP_KEY_ENTER_LONG     14      /*0x0E, Sent by the library if ENTER is long pressed*/
#if GUI_USE_GROUP  == 1
/**********************
 *      TYPEDEFS
 **********************/
struct _group_t;

typedef void (*group_style_mod_func_t)(style_t *);
typedef void (*group_focus_cb_t)(struct _group_t *);

typedef struct _group_t
{
    ll_t obj_ll;                         /*Linked list to store the objects in the group */
    obj_t ** obj_focus;                  /*The object in focus*/
    group_style_mod_func_t style_mod;   /*A function which modifies the style of the focused object*/
    group_focus_cb_t focus_cb;           /*A function to call when a new object is focused (optional)*/
    style_t style_tmp;                   /*Stores the modified style of the focused object */
    uint8_t frozen:1;                       /*1: can't focus to new object*/
}group_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a new object group
 * @return pointer to the new object group
 */
group_t * group_create(void);

/**
 * Delete a group object
 * @param group pointer to a group
 */
void group_del(group_t * group);

/**
 * Add an object to a group
 * @param group pointer to a group
 * @param obj pointer to an object to add
 */
void group_add_obj(group_t * group, obj_t * obj);

/**
 * Remove an object from its group
 * @param obj pointer to an object to remove
 */
void group_remove_obj(obj_t * obj);

/**
 * Focus on an object (defocus the current)
 * @param obj pointer to an object to focus on
 */
void group_focus_obj(obj_t * obj);

/**
 * Focus the next object in a group (defocus the current)
 * @param group pointer to a group
 */
void group_focus_next(group_t * group);

/**
 * Focus the previous object in a group (defocus the current)
 * @param group pointer to a group
 */
void group_focus_prev(group_t * group);

/**
 * Do not let to change the focus from the current object
 * @param group pointer to a group
 * @param en true: freeze, false: release freezing (normal mode)
 */
void group_focus_freeze(group_t * group, bool en);

/**
 * Send a control character to the focuses object of a group
 * @param group pointer to a group
 * @param c a character (use GUI_GROUP_KEY_.. to navigate)
 */
void group_send_data(group_t * group, uint32_t c);

/**
 * Set a function for a group which will modify the object's style if it is in focus
 * @param group pointer to a group
 * @param style_mod_func the style modifier function pointer
 */
void group_set_style_mod_cb(group_t * group,group_style_mod_func_t style_mod_func);

/**
 * Set a function for a group which will be called when a new object is focused
 * @param group pointer to a group
 * @param focus_cb the call back function or NULL if unused
 */
void group_set_focus_cb(group_t * group, group_focus_cb_t focus_cb);

/**
 * Modify a style with the set 'style_mod' function. The input style remains unchanged.
 * @param group pointer to group
 * @param style pointer to a style to modify
 * @return a copy of the input style but modified with the 'style_mod' function
 */
style_t * group_mod_style(group_t * group, const style_t * style);

/**
 * Get the focused object or NULL if there isn't one
 * @param group pointer to a group
 * @return pointer to the focused object
 */
obj_t * group_get_focused(group_t * group);

/**
 * Get a the style modifier function of a group
 * @param group pointer to a group
 * @return pointer to the style modifier function
 */
group_style_mod_func_t group_get_style_mod_cb(group_t * group);

/**
 * Get the focus callback function of a group
 * @param group pointer to a group
 * @return the call back function or NULL if not set
 */
group_focus_cb_t group_get_focus_cb(group_t * group);

/**********************
 *      MACROS
 **********************/

#endif /*GUI_USE_GROUP != 0*/

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
