#ifndef __GUI_DRAW_H
#define __GUI_DRAW_H
#ifdef __cplusplus
extern "C" {
#endif
#include "RTE_Include.h"
/*********************
 *      DEFINES
 *********************/
/*If image pixels contains alpha we need to know how much byte is a pixel*/
#if GUI_COLOR_DEPTH == 1 || GUI_COLOR_DEPTH == 8
# define GUI_IMG_PX_SIZE_ALPHA_BYTE   2
#elif GUI_COLOR_DEPTH == 16
# define GUI_IMG_PX_SIZE_ALPHA_BYTE   3
#elif GUI_COLOR_DEPTH == 24
# define GUI_IMG_PX_SIZE_ALPHA_BYTE   4
#endif

/**********************
 *      TYPEDEFS
 **********************/ 

/* Image header it is compatible with
 * the result image converter utility*/
typedef struct
{
    union{
        struct {
            uint32_t chroma_keyed:1;    /*1: The image contains transparent pixels with GUI_COLOR_TRANSP color*/
            uint32_t alpha_byte  :1;    /*Every pixel is extended with a 8 bit alpha channel*/
            uint32_t format      :6;    /*See: img_px_format*/
            uint32_t w:12;              /*Width of the image map*/
            uint32_t h:12;              /*Height of the image map*/
        }header;
        uint8_t src_type;
    };

    union {
        const uint8_t * pixel_map;  /*For internal images (c arrays) pointer to the pixels array*/
        uint8_t first_pixel;        /*For external images (binary) the first byte of the pixels (just for convenient)*/
    };
}img_t;

typedef enum {
    GUI_IMG_FORMAT_UNKOWN = 0,
    GUI_IMG_FORMAT_INTERNAL_RAW,       /*'img_t' variable compiled with the code*/
    GUI_IMG_FORMAT_FILE_RAW_RGB332,    /*8 bit*/
    GUI_IMG_FORMAT_FILE_RAW_RGB565,    /*16 bit*/
    GUI_IMG_FORMAT_FILE_RAW_RGB888,    /*24 bit (stored on 32 bit)*/
}img_format_t;


typedef enum {
    GUI_IMG_SRC_VARIABLE,
    GUI_IMG_SRC_FILE,
    GUI_IMG_SRC_SYMBOL,
    GUI_IMG_SRC_UNKNOWN,
}img_src_t;


/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Draw a rectangle
 * @param cords_p the coordinates of the rectangle
 * @param mask_p the rectangle will be drawn only in this mask
 * @param style_p pointer to a style
 */
void draw_rect(const area_t * cords_p, const area_t * mask_p, const style_t * style_p);

#if GUI_USE_TRIANGLE != 0
/**
 *
 * @param points pointer to an array with 3 points
 * @param mask_p the triangle will be drawn only in this mask
 * @param color color of the triangle
 */
void draw_triangle(const point_t * points, const area_t * mask_p, color_t color);
#endif

/**
 * Write a text
 * @param cords_p coordinates of the label
 * @param mask_p the label will be drawn only in this area
 * @param style_p pointer to a style
 * @param txt 0 terminated text to write
 * @param flags settings for the text from 'txt_flag_t' enum
 * @param offset text offset in x and y direction (NULL if unused)
 */
void draw_label(const area_t * cords_p,const area_t * mask_p, const style_t * style_p,
                    const char * txt, txt_flag_t flag, point_t * offset);

#if GUI_USE_IMG
/**
 * Draw an image
 * @param cords_p the coordinates of the image
 * @param mask_p the image will be drawn only in this area
 * @param map_p pointer to a color_t array which contains the pixels of the image
 */
void draw_img(const area_t * coords, const area_t * mask,
             const style_t * style, const void * src);
#endif

/**
 * Draw a line
 * @param p1 first point of the line
 * @param p2 second point of the line
 * @param mask_pthe line will be drawn only on this area
 * @param style_p pointer to a style
 */
void draw_line(const point_t * p1, const point_t * p2, const area_t * mask_p,
                  const style_t * style_p);

/**********************
 *      MACROS
 **********************/
#ifdef __cplusplus
}
#endif
#endif
