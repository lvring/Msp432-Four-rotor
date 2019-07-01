/**
 * @file draw_vbasic.h
 * 
 */

#ifndef GUI_DRAW_VBASIC_H
#define GUI_DRAW_VBASIC_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"

#if GUI_VDB_SIZE != 0

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void vpx(coord_t x, coord_t y, const area_t * mask_p, color_t color, opa_t opa);
/**
 * Fill an area in the Virtual Display Buffer
 * @param cords_p coordinates of the area to fill
 * @param mask_p fill only o this mask
 * @param color fill color
 * @param opa opacity of the area (0..255)
 */
void vfill(const area_t * cords_p, const area_t * mask_p,
                color_t color, opa_t opa);

/**
 * Draw a letter in the Virtual Display Buffer
 * @param pos_p left-top coordinate of the latter
 * @param mask_p the letter will be drawn only on this area
 * @param font_p pointer to font
 * @param letter a letter to draw
 * @param color color of letter
 * @param opa opacity of letter (0..255)
 */
void vletter(const point_t * pos_p, const area_t * mask_p,
                const font_t * font_p, uint32_t letter,
                color_t color, opa_t opa);

/**
 * Draw a color map to the display (image)
 * @param cords_p coordinates the color map
 * @param mask_p the map will drawn only on this area  (truncated to VDB area)
 * @param map_p pointer to a color_t array
 * @param opa opacity of the map
 * @param chroma_keyed true: enable transparency of GUI_IMG_GUI_COLOR_TRANSP color pixels
 * @param alpha_byte true: extra alpha byte is inserted for every pixel
 * @param recolor mix the pixels with this color
 * @param recolor_opa the intense of recoloring
 */
void vmap(const area_t * cords_p, const area_t * mask_p,
        const uint8_t * map_p, opa_t opa, bool chroma_key, bool alpha_byte,
        color_t recolor, opa_t recolor_opa);


/**
 * Reallocate 'color_map_tmp' to the new hor. res. size. It is used in 'sw_fill'
 */
void vdraw_refresh_temp_arrays(void);

/**********************
 *      MACROS
 **********************/

#endif  /*GUI_VDB_SIZE != 0*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /*GUI_DRAW_RBASIC_H*/
