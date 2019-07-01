#ifndef __GUI_HAL_INDEX_H
#define __GUI_HAL_INDEX_H
#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/*Possible input device types*/
typedef enum {
    GUI_INDEV_TYPE_NONE,        /*Show uninitialized state*/
    GUI_INDEV_TYPE_POINTER,     /*Touch pad, mouse, external button*/
    GUI_INDEV_TYPE_KEYPAD,      /*Keypad or keyboard*/
    GUI_INDEV_TYPE_BUTTON,      /*External (hardware button) which is assinged to a specific point of the screen*/
} hal_indev_type_t;

/*States for input devices*/
typedef enum {
    GUI_INDEV_STATE_REL = 0,
    GUI_INDEV_STATE_PR
}indev_state_t;

/*Data type when an input device is read */
typedef struct {
    union {
        point_t point;      /*For GUI_INDEV_TYPE_POINTER the currently pressed point*/
        uint32_t key;          /*For GUI_INDEV_TYPE_KEYPAD the currently pressed key*/
        uint32_t btn;          /*For GUI_INDEV_TYPE_BUTTON the currently pressed button*/
    };
    indev_state_t state;    /*GUI_INDEV_EVENT_REL or GUI_INDEV_EVENT_PR*/
    void *user_data;           /*'indev_drv_t.priv' for this driver*/
}indev_data_t;

/*Initialized by the user and registered by 'indev_add()'*/
typedef struct {
    hal_indev_type_t type;                   /*Input device type*/
    bool (*read)(indev_data_t *data);        /*Function pointer to read data. Return 'true' if there is still data to be read (buffered)*/
    void *user_data;                            /*Pointer to user defined data, passed in 'indev_data_t' on read*/
}indev_drv_t;

struct _obj_t;

/*Run time data of input devices*/
typedef struct _indev_proc_t {
    indev_state_t state;
    union {
        struct {    /*Pointer and button data*/
            point_t act_point;
            point_t last_point;
            point_t vect;
            point_t drag_sum;				/*Count the dragged pixels to check GUI_INDEV_DRAG_LIMIT*/
            struct _obj_t * act_obj;
            struct _obj_t * last_obj;

            /*Flags*/
            uint8_t drag_range_out      :1;
            uint8_t drag_in_prog        :1;
            uint8_t wait_unil_release   :1;
        };
        struct {    /*Keypad data*/
            indev_state_t last_state;
            uint32_t last_key;
        };
    };

    uint32_t pr_timestamp;          /*Pressed time stamp*/
    uint32_t longpr_rep_timestamp;  /*Long press repeat time stamp*/

    /*Flags*/
    uint8_t long_pr_sent        :1;
    uint8_t reset_query         :1;
    uint8_t disabled            :1;
}indev_proc_t;


struct _obj_t;
struct _group_t;

/*The main input device descriptor with driver, runtime data ('proc') and some additional information*/
typedef struct _indev_t {
    indev_drv_t driver;
    indev_proc_t proc;
    uint32_t last_activity_time;
    union {
        struct _obj_t *cursor;       /*Cursor for GUI_INPUT_TYPE_POINTER*/
        struct _group_t *group;      /*Keypad destination group*/
        point_t * btn_points;      /*Array points assigned to the button ()screen will be pressed here by the buttons*/

    };
    struct _indev_t *next;
} indev_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize an input device driver with default values.
 * It is used to surly have known values in the fields ant not memory junk.
 * After it you can set the fields.
 * @param driver pointer to driver variable to initialize
 */
void indev_drv_init(indev_drv_t *driver);

/**
 * Register an initialized input device driver.
 * @param driver pointer to an initialized 'indev_drv_t' variable (can be local variable)
 * @return pointer to the new input device or NULL on error
 */
indev_t * indev_drv_register(indev_drv_t *driver);

/**
 * Get the next input device.
 * @param indev pointer to the current input device. NULL to initialize.
 * @return the next input devise or NULL if no more. Gives the first input device when the parameter is NULL
 */
indev_t * indev_next(indev_t * indev);

/**
 * Read data from an input device.
 * @param indev pointer to an input device
 * @param data input device will write its data here
 * @return false: no more data; true: there more data to read (buffered)
 */
bool indev_read(indev_t * indev, indev_data_t *data);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
