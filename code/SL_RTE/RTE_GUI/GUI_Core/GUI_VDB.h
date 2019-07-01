#ifndef __GUI_VDB_H
#define __GUI_VDB_H
#ifdef __cplusplus
extern "C" {
#endif
#include "RTE_Include.h"
/*********************
 *      DEFINES
 *********************/
#if GUI_VDB_SIZE != 0
/**********************
 *      TYPEDEFS
 **********************/

typedef struct
{
	color_t *buf;
	area_t area;
}vdb_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void vdb_init(void);
/**
 * Get the 'vdb' variable or allocate one in GUI_VDB_DOUBLE mode
 * @return pointer to a 'vdb' variable
 */
vdb_t * vdb_get(void);

/**
 * Flush the content of the vdb
 */
void vdb_flush(bool reverse);


/**
 * In 'GUI_VDB_DOUBLE' mode  has to be called when 'disp_map()'
 * is ready with copying the map to a frame buffer.
 */
void flush_ready(void);

/**********************
 *      MACROS
 **********************/

#else /*GUI_VDB_SIZE != 0*/

/*Just for compatibility*/
void flush_ready(void);
#endif
#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
