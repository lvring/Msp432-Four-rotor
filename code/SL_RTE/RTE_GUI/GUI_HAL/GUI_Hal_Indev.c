#include "RTE_Include.h"
#if RTE_USE_GUI == 1
/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static indev_t *indev_list = NULL;

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize an input device driver with default values.
 * It is used to surly have known values in the fields ant not memory junk.
 * After it you can set the fields.
 * @param driver pointer to driver variable to initialize
 */
void indev_drv_init(indev_drv_t *driver)
{
    driver->read = NULL;
    driver->type = GUI_INDEV_TYPE_NONE;
    driver->user_data = NULL;
}

/**
 * Register an initialized input device driver.
 * @param driver pointer to an initialized 'indev_drv_t' variable (can be local variable)
 * @return pointer to the new input device or NULL on error
 */
indev_t * indev_drv_register(indev_drv_t *driver)
{
    indev_t *node;

    node = RTE_BGet(MEM_RTE,sizeof(indev_t));
    if (!node) return NULL;

    memset(node, 0, sizeof(indev_t));
    memcpy(&node->driver, driver, sizeof(indev_drv_t));

    node->next = NULL;
    node->proc.reset_query = 1;
    node->cursor = NULL;
    node->group = NULL;
    node->btn_points = NULL;

    if (indev_list == NULL) {
        indev_list = node;
    } else {
        indev_t *last = indev_list;
        while (last->next)
            last = last->next;

        last->next = node;
    }

    return node;
}

/**
 * Get the next input device.
 * @param indev pointer to the current input device. NULL to initialize.
 * @return the next input devise or NULL if no more. Give the first input device when the parameter is NULL
 */
indev_t * indev_next(indev_t * indev)
{

    if(indev == NULL) {
        return indev_list;
    } else {
        if(indev->next == NULL) return NULL;
        else return indev->next;
    }
}

/**
 * Read data from an input device.
 * @param indev pointer to an input device
 * @param data input device will write its data here
 * @return false: no more data; true: there more data to read (buffered)
 */
bool indev_read(indev_t * indev, indev_data_t *data)
{
    bool cont = false;

    if(indev->driver.read) {
        data->user_data = indev->driver.user_data;
        cont = indev->driver.read(data);
    } else {
        memset(data, 0, sizeof(indev_data_t));
    }

    return cont;
}
#endif
