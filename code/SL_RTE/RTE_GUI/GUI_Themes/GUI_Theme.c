/*********************
 *      INCLUDES
 *********************/
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

/**********************
 *  STATIC VARIABLES
 **********************/
static theme_t *current_theme;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Set a theme for the system.
 * From now, all the created objects will use styles from this theme by default
 * @param th pointer to theme (return value of: 'theme_init_xxx()')
 */
void theme_set_current(theme_t *th)
{
    current_theme = th;
}

/**
 * Get the current system theme.
 * @return pointer to the current system theme. NULL if not set.
 */
theme_t * theme_get_current(void)
{
    return current_theme;
}
#endif
/**********************
 *   STATIC FUNCTIONS
 **********************/
