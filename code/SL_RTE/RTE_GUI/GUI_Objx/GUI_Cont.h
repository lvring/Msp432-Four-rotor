/**
 * @file cont.h
 * 
 */

#ifndef GUI_CONT_H
#define GUI_CONT_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_CONT != 0


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/*Layout options*/
typedef enum
{
	GUI_LAYOUT_OFF = 0,
	GUI_LAYOUT_CENTER,
	GUI_LAYOUT_COL_L,	/*Column left align*/
	GUI_LAYOUT_COL_M,	/*Column middle align*/
	GUI_LAYOUT_COL_R,	/*Column right align*/
	GUI_LAYOUT_ROW_T,	/*Row top align*/
	GUI_LAYOUT_ROW_M,	/*Row middle align*/
	GUI_LAYOUT_ROW_B,	/*Row bottom align*/
	GUI_LAYOUT_PRETTY,	/*Put as many object as possible in row and begin a new row*/
	GUI_LAYOUT_GRID,	    /*Align same-sized object into a grid*/
}layout_t;

typedef struct
{
    /*Inherited from 'base_obj' so no inherited ext. */ /*Ext. of ancestor*/
    /*New data for this type */
    uint8_t layout  :4;     /*A layout from 'cont_layout_t' enum*/
    uint8_t hor_fit :1;     /*1: Enable horizontal fit to involve all children*/
    uint8_t ver_fit :1;     /*1: Enable horizontal fir to involve all children*/
}cont_ext_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a container objects
 * @param par pointer to an object, it will be the parent of the new container
 * @param copy pointer to a container object, if not NULL then the new object will be copied from it
 * @return pointer to the created container
 */
obj_t * cont_create(obj_t * par, obj_t * copy);

/*=====================
 * Setter functions
 *====================*/

/**
 * Set a layout on a container
 * @param cont pointer to a container object
 * @param layout a layout from 'cont_layout_t'
 */
void cont_set_layout(obj_t * cont, layout_t layout);


/**
 * Enable the horizontal or vertical fit.
 * The container size will be set to involve the children horizontally or vertically.
 * @param cont pointer to a container object
 * @param hor_en true: enable the horizontal fit
 * @param ver_en true: enable the vertical fit
 */
void cont_set_fit(obj_t * cont, bool hor_en, bool ver_en);

/**
 * Set the style of a container
 * @param cont pointer to a container object
 * @param style pointer to the new style
 */
static inline void cont_set_style(obj_t *cont, style_t * style)
{
    obj_set_style(cont, style);
}

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the layout of a container
 * @param cont pointer to container object
 * @return the layout from 'cont_layout_t'
 */
layout_t cont_get_layout(obj_t * cont);

/**
 * Get horizontal fit enable attribute of a container
 * @param cont pointer to a container object
 * @return true: horizontal fit is enabled; false: disabled
 */
bool cont_get_hor_fit(obj_t * cont);

/**
 * Get vertical fit enable attribute of a container
 * @param cont pointer to a container object
 * @return true: vertical fit is enabled; false: disabled
 */
bool cont_get_ver_fit(obj_t * cont);

/**
 * Get the style of a container
 * @param cont pointer to a container object
 * @return pointer to the container's style
 */
static inline style_t * cont_get_style(obj_t *cont)
{
    return obj_get_style(cont);
}

/**********************
 *      MACROS
 **********************/

#endif  /*GUI_USE_CONT*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /*GUI_CONT_H*/
