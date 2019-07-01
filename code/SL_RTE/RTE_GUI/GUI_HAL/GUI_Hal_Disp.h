#ifndef __GUI_HAL_DISP_H
#define __GUI_HAL_DISP_H
#ifdef __cplusplus
extern "C" {
#endif
#include "RTE_Include.h"
/**********************
 *      TYPEDEFS
 **********************/
/**
 * Display Driver structure to be registered by HAL
 */
typedef struct _disp_drv_t {
    /*Write the internal buffer (VDB) to the display. 'flush_ready()' has to be called when finished*/
    void (*disp_flush)(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const color_t * color_p,bool reverse);

    /*Fill an area with a color on the display*/
    void (*disp_fill)(int32_t x1, int32_t y1, int32_t x2, int32_t y2, color_t color);

    /*Write pixel map (e.g. image) to the display*/
    void (*disp_map)(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const color_t * color_p);

#if GUI_USE_GPU == 1
    /*Blend two memories using opacity (GPU only)*/
    void (*mem_blend)(color_t * dest, const color_t * src, uint32_t length, opa_t opa);

    /*Fill a memory with a color (GPU only)*/
    void (*mem_fill)(color_t * dest, uint32_t length, color_t color);
#endif
} disp_drv_t;

typedef struct _disp_t {
    disp_drv_t driver;
    struct _disp_t *next;
} disp_t;
/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize a display driver with default values.
 * It is used to surly have known values in the fields ant not memory junk.
 * After it you can set the fields.
 * @param driver pointer to driver variable to initialize
 */
void disp_drv_init(disp_drv_t *driver);

/**
 * Register an initialized display driver.
 * Automatically set the first display as active.
 * @param driver pointer to an initialized 'disp_drv_t' variable (can be local variable)
 * @return pointer to the new display or NULL on error
 */
disp_t * disp_drv_register(disp_drv_t *driver);

/**
 * Set the active display
 * @param disp pointer to a display (return value of 'disp_register')
 */
void disp_set_active(disp_t * disp);

/**
 * Get a pointer to the active display
 * @return pointer to the active display
 */
disp_t * disp_get_active(void);

/**
 * Get the next display.
 * @param disp pointer to the current display. NULL to initialize.
 * @return the next display or NULL if no more. Give the first display when the parameter is NULL
 */
disp_t * disp_next(disp_t * disp);

/**
 * Fill a rectangular area with a color on the active display
 * @param x1 left coordinate of the rectangle
 * @param x2 right coordinate of the rectangle
 * @param y1 top coordinate of the rectangle
 * @param y2 bottom coordinate of the rectangle
 * @param color_p pointer to an array of colors
 */
void disp_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, color_t *color_p,bool reverse);

/**
 * Fill a rectangular area with a color on the active display
 * @param x1 left coordinate of the rectangle
 * @param x2 right coordinate of the rectangle
 * @param y1 top coordinate of the rectangle
 * @param y2 bottom coordinate of the rectangle
 * @param color fill color
 */
void disp_fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, color_t color);

/**
 * Put a color map to a rectangular area on the active display
 * @param x1 left coordinate of the rectangle
 * @param x2 right coordinate of the rectangle
 * @param y1 top coordinate of the rectangle
 * @param y2 bottom coordinate of the rectangle
 * @param color_map pointer to an array of colors
 */
void disp_map(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const color_t * color_map);

#if GUI_USE_GPU
/**
 * Blend pixels to a destination memory from a source memory
 * In 'disp_drv_t' 'mem_blend' is optional. (NULL if not available)
 * @param dest a memory address. Blend 'src' here.
 * @param src pointer to pixel map. Blend it to 'dest'.
 * @param length number of pixels in 'src'
 * @param opa opacity (0, GUI_OPA_TRANSP: transparent ... 255, GUI_OPA_COVER, fully cover)
 */
void disp_mem_blend(color_t * dest, const color_t * src, uint32_t length, opa_t opa);

/**
 * Fill a memory with a color (GPUs may support it)
 * In 'disp_drv_t' 'mem_fill' is optional. (NULL if not available)
 * @param dest a memory address. Copy 'src' here.
 * @param src pointer to pixel map. Copy it to 'dest'.
 * @param length number of pixels in 'src'
 * @param opa opacity (0, GUI_OPA_TRANSP: transparent ... 255, GUI_OPA_COVER, fully cover)
 */
void disp_mem_fill(color_t * dest, uint32_t length, color_t color);
/**
 * Shows if memory blending (by GPU) is supported or not
 * @return false: 'mem_blend' is not supported in the driver; true: 'mem_blend' is supported in the driver
 */
bool disp_is_mem_blend_supported(void);

/**
 * Shows if memory fill (by GPU) is supported or not
 * @return false: 'mem_fill' is not supported in the drover; true: 'mem_fill' is supported in the driver
 */
bool disp_is_mem_fill_supported(void);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
