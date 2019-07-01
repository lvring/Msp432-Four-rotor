/**
 * @file img.h
 * 
 */

#ifndef GUI_IMG_H
#define GUI_IMG_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_IMG != 0


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
/*Data of image*/
typedef struct
{
    /*No inherited ext. because inherited from the base object*/ /*Ext. of ancestor*/
    /*New data for this type */
    const void * src;             /*Image source: Pointer to an array or a file or a symbol*/

    coord_t w;               /*Width of the image (Handled by the library)*/
    coord_t h;               /*Height of the image (Handled by the library)*/
    uint8_t src_type  :2;       /*See: img_src_t*/
    uint8_t auto_size :1;       /*1: automatically set the object size to the image size*/
    uint8_t chroma_keyed :1;    /*1: Chroma keyed image, GUI_COLOR_TRANSP (conf.h) pixels will be transparent (Handled by the library)*/
    uint8_t alpha_byte   :1;    /*1: Extra byte for every pixel to define opacity*/
}img_ext_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create an image objects
 * @param par pointer to an object, it will be the parent of the new button
 * @param copy pointer to a image object, if not NULL then the new object will be copied from it
 * @return pointer to the created image
 */
obj_t * img_create(obj_t * par, obj_t * copy);

/*=====================
 * Setter functions
 *====================*/

/**
 * Set the pixel map to display by the image
 * @param img pointer to an image object
 * @param data the image data
 */
void img_set_src(obj_t * img, const void * src_img);

/**
 * Obsolete since v5.1. Just for compatibility with v5.0. Will be removed in v6.0.
 * Use 'img_set_src()' instead.
 * @param img -
 * @param fn -
 */
static inline void img_set_file(obj_t * img, const char * fn)
{

}

/**
 * Enable the auto size feature.
 * If enabled the object size will be same as the picture size.
 * @param img pointer to an image
 * @param autosize_en true: auto size enable, false: auto size disable
 */
void img_set_auto_size(obj_t * img, bool autosize_en);

/**
 * Set the style of an image
 * @param img pointer to an image object
 * @param style pointer to a style
 */
static inline void img_set_style(obj_t *img, style_t *style)
{
    obj_set_style(img, style);
}

/**
 * Obsolete since v5.1. Just for compatibility with v5.0. Will be removed in v6.0
 * @param img -
 * @param upscale -
 */
static inline void img_set_upscale(obj_t * img, bool upcale)
{

}

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the type of an image source
 * @param src pointer to an image source:
 *  - pointer to an 'img_t' variable (image stored internally and compiled into the code)
 *  - a path to an file (e.g. "S:/folder/image.bin")
 *  - or a symbol (e.g. SYMBOL_CLOSE)
 * @return type of the image source GUI_IMG_SRC_VARIABLE/FILE/SYMBOL/UNKOWN
 */
img_src_t img_get_src_type(const void * src);


/**
 * Get the name of the file set for an image
 * @param img pointer to an image
 * @return file name
 */
const char * img_get_file_name(obj_t * img);

/**
 * Get the auto size enable attribute
 * @param img pointer to an image
 * @return true: auto size is enabled, false: auto size is disabled
 */
bool img_get_auto_size(obj_t * img);

/**
 * Get the style of an image object
 * @param img pointer to an image object
 * @return pointer to the image's style
 */
static inline style_t* img_get_style(obj_t *img)
{
    return obj_get_style(img);
}

/**
 * Obsolete since v5.1. Just for compatibility with v5.0. Will be removed in v6.0
 * @param img -
 * @return false
 */
static inline bool img_get_upscale(obj_t * img)
{
    return false;
}

/**********************
 *      MACROS
 **********************/

/*Use this macro to declare an image in a c file*/
#define GUI_IMG_DECLARE(var_name) extern const img_t var_name;

#endif  /*GUI_USE_IMG*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /*GUI_IMG_H*/
