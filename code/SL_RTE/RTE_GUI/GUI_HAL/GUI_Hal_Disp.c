#include "RTE_Include.h"
#if RTE_USE_GUI == 1
/**********************
 *  STATIC VARIABLES
 **********************/
static disp_t *disp_list = NULL;
static disp_t *active;
/**
 * Initialize a display driver with default values.
 * It is used to surly have known values in the fields ant not memory junk.
 * After it you can set the fields.
 * @param driver pointer to driver variable to initialize
 */
void disp_drv_init(disp_drv_t *driver)
{
    driver->disp_fill = NULL;
    driver->disp_map = NULL;
    driver->disp_flush = NULL;

#if GUI_USE_GPU
    driver->mem_blend = NULL;
    driver->mem_fill = NULL;
#endif
}

/**
 * Register an initialized display driver.
 * Automatically set the first display as active.
 * @param driver pointer to an initialized 'disp_drv_t' variable (can be local variable)
 * @return pointer to the new display or NULL on error
 */
disp_t * disp_drv_register(disp_drv_t *driver)
{
    disp_t *node;

    node = RTE_BGetz(MEM_RTE,sizeof(disp_t));
    if (!node) return NULL;

    memcpy(&node->driver,driver, sizeof(disp_drv_t));
    node->next = NULL;

    /* Set first display as active by default */
    if (disp_list == NULL) {
        disp_list = node;
        active = node;
        obj_invalidate(scr_act());
    } else {
        disp_list->next = node;
    }

    return node;
}


/**
 * Set the active display
 * @param disp pointer to a display (return value of 'disp_register')
 */
void disp_set_active(disp_t * disp)
{
    active = disp;
    obj_invalidate(scr_act());
}

/**
 * Get a pointer to the active display
 * @return pointer to the active display
 */
disp_t * disp_get_active(void)
{
    return active;
}

/**
 * Get the next display.
 * @param disp pointer to the current display. NULL to initialize.
 * @return the next display or NULL if no more. Give the first display when the parameter is NULL
 */
disp_t * disp_next(disp_t * disp)
{
    if(disp == NULL) {
        return disp_list;
    } else {
        if(disp_list->next == NULL) return NULL;
        else return disp_list->next;
    }
}

/**
 * Write the content of the internal buffer (VDB) to the display
 * @param x1 left coordinate of the rectangle
 * @param x2 right coordinate of the rectangle
 * @param y1 top coordinate of the rectangle
 * @param y2 bottom coordinate of the rectangle
 * @param color_p fill color
 */
void disp_fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, color_t color)
{
    if(active == NULL) return;
    if(active->driver.disp_fill != NULL) active->driver.disp_fill(x1, y1, x2, y2, color);
}

/**
 * Fill a rectangular area with a color on the active display
 * @param x1 left coordinate of the rectangle
 * @param x2 right coordinate of the rectangle
 * @param y1 top coordinate of the rectangle
 * @param y2 bottom coordinate of the rectangle
 * @param color_p pointer to an array of colors
 */
void disp_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, color_t *color_p,bool reverse)
{
    if(active == NULL) return;
    if(active->driver.disp_flush != NULL) active->driver.disp_flush(x1, y1, x2, y2, color_p,reverse);
}

/**
 * Put a color map to a rectangular area on the active display
 * @param x1 left coordinate of the rectangle
 * @param x2 right coordinate of the rectangle
 * @param y1 top coordinate of the rectangle
 * @param y2 bottom coordinate of the rectangle
 * @param color_map pointer to an array of colors
 */
void disp_map(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const color_t * color_map)
{
    if(active == NULL) return;
    if(active->driver.disp_map != NULL)  active->driver.disp_map(x1, y1, x2, y2, color_map);
}

#if GUI_USE_GPU

/**
 * Blend pixels to a destination memory from a source memory
 * In 'disp_drv_t' 'mem_blend' is optional. (NULL if not available)
 * @param dest a memory address. Blend 'src' here.
 * @param src pointer to pixel map. Blend it to 'dest'.
 * @param length number of pixels in 'src'
 * @param opa opacity (0, GUI_OPA_TRANSP: transparent ... 255, GUI_OPA_COVER, fully cover)
 */
void disp_mem_blend(color_t * dest, const color_t * src, uint32_t length, opa_t opa)
{
    if(active == NULL) return;
    if(active->driver.mem_blend != NULL) active->driver.mem_blend(dest, src, length, opa);
}

/**
 * Fill a memory with a color (GPUs may support it)
 * In 'disp_drv_t' 'mem_fill' is optional. (NULL if not available)
 * @param dest a memory address. Copy 'src' here.
 * @param src pointer to pixel map. Copy it to 'dest'.
 * @param length number of pixels in 'src'
 * @param opa opacity (0, GUI_OPA_TRANSP: transparent ... 255, GUI_OPA_COVER, fully cover)
 */
void disp_mem_fill(color_t * dest, uint32_t length, color_t color)
{
    if(active == NULL) return;
    if(active->driver.mem_fill != NULL) active->driver.mem_fill(dest, length, color);
}

/**
 * Shows if memory blending (by GPU) is supported or not
 * @return false: 'mem_blend' is not supported in the driver; true: 'mem_blend' is supported in the driver
 */
bool disp_is_mem_blend_supported(void)
{
    if(active == NULL) return false;
    if(active->driver.mem_blend) return true;
    else return false;
}

/**
 * Shows if memory fill (by GPU) is supported or not
 * @return false: 'mem_fill' is not supported in the drover; true: 'mem_fill' is supported in the driver
 */
bool disp_is_mem_fill_supported(void)
{
    if(active == NULL) return false;
    if(active->driver.mem_fill) return true;
    else return false;
}
#endif

#endif
