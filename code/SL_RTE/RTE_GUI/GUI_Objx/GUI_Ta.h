/**
 * @file ta.h
 * 
 */

#ifndef GUI_TA_H
#define GUI_TA_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "RTE_Include.h"
#if GUI_USE_TA != 0

/*Testing of dependencies*/
#if GUI_USE_PAGE == 0
#error "ta: page is required. Enable it in conf.h (GUI_USE_PAGE  1) "
#endif

#if GUI_USE_LABEL == 0
#error "ta: label is required. Enable it in conf.h (GUI_USE_LABEL  1) "
#endif


/*********************
 *      DEFINES
 *********************/
#define GUI_TA_CURSOR_LAST (0x7FFF) /*Put the cursor after the last character*/

/**********************
 *      TYPEDEFS
 **********************/

typedef enum {
    GUI_CURSOR_NONE,
	GUI_CURSOR_LINE,
	GUI_CURSOR_BLOCK,
	GUI_CURSOR_OUTLINE,
	GUI_CURSOR_UNDERLINE,
    GUI_CURSOR_HIDDEN = 0x10,    /*Or it to any value to hide the cursor temporally*/
}cursor_type_t;

/*Data of text area*/
typedef struct
{
    page_ext_t page; /*Ext. of ancestor*/
    /*New data for this type */
    obj_t * label;           /*Label of the text area*/
    char * pwd_tmp;             /*Used to store the original text in password mode*/
    uint8_t pwd_mode :1;        /*Replace characters with '*' */
    uint8_t one_line :1;        /*One line mode (ignore line breaks)*/
    struct {
        style_t *style;      /*Style of the cursor (NULL to use label's style)*/
        coord_t valid_x;         /*Used when stepping up/down in text area when stepping to a shorter line. (Handled by the library)*/
        uint16_t pos;           /*The current cursor position (0: before 1. letter; 1: before 2. letter etc.)*/
        cursor_type_t type;  /*Shape of the cursor*/
        uint8_t state :1;       /*Indicates that the cursor is visible now or not (Handled by the library)*/
    }cursor;
}ta_ext_t;

typedef enum {
    GUI_TA_STYLE_BG,
    GUI_TA_STYLE_SB,
    GUI_TA_STYLE_CURSOR,
}ta_style_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/


/**
 * Create a text area objects
 * @param par pointer to an object, it will be the parent of the new text area
 * @param copy pointer to a text area object, if not NULL then the new object will be copied from it
 * @return pointer to the created text area
 */
obj_t * ta_create(obj_t * par, obj_t * copy);


/*======================
 * Add/remove functions
 *=====================*/

/**
 * Insert a character to the current cursor position
 * @param ta pointer to a text area object
 * @param c a character
 */
void ta_add_char(obj_t * ta, char c);

/**
 * Insert a text to the current cursor position
 * @param ta pointer to a text area object
 * @param txt a '\0' terminated string to insert
 */
void ta_add_text(obj_t * ta, const char * txt);

/**
 * Delete a the left character from the current cursor position
 * @param ta pointer to a text area object
 */
void ta_del_char(obj_t * ta);

/*=====================
 * Setter functions
 *====================*/

/**
 * Set the text of a text area
 * @param ta pointer to a text area
 * @param txt pointer to the text
 */
void ta_set_text(obj_t * ta, const char * txt);

/**
 * Set the cursor position
 * @param obj pointer to a text area object
 * @param pos the new cursor position in character index
 *             < 0 : index from the end of the text
 *             GUI_TA_CURSOR_LAST: go after the last character
 */
void ta_set_cursor_pos(obj_t * ta, int16_t pos);

/**
 * Set the cursor type.
 * @param ta pointer to a text area object
 * @param cur_type: element of 'cursor_type_t'
 */
void ta_set_cursor_type(obj_t * ta, cursor_type_t cur_type);
/**
 * Enable/Disable password mode
 * @param ta pointer to a text area object
 * @param pwd_en true: enable, false: disable
 */
void ta_set_pwd_mode(obj_t * ta, bool pwd_en);

/**
 * Configure the text area to one line or back to normal
 * @param ta pointer to a Text area object
 * @param en true: one line, false: normal
 */
void ta_set_one_line(obj_t * ta, bool en);

/**
 * Set the scroll bar mode of a text area
 * @param ta pointer to a text area object
 * @param sb_mode the new mode from 'page_sb_mode_t' enum
 */
static inline void ta_set_sb_mode(obj_t * ta, sb_mode_t mode)
{
    page_set_sb_mode(ta, mode);
}

/**
 * Set a style of a text area
 * @param ta pointer to a text area object
 * @param type which style should be set
 * @param style pointer to a style
 */
void ta_set_style(obj_t *ta, ta_style_t type, style_t *style);

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the text of a text area
 * @param ta pointer to a text area object
 * @return pointer to the text
 */
const char * ta_get_text(obj_t * ta);

/**
 * Get the label of a text area
 * @param ta pointer to a text area object
 * @return pointer to the label object
 */
obj_t * ta_get_label(obj_t * ta);

/**
 * Get the current cursor position in character index
 * @param ta pointer to a text area object
 * @return the cursor position
 */
uint16_t ta_get_cursor_pos(obj_t * ta);

/**
 * Get the current cursor visibility.
 * @param ta pointer to a text area object
 * @return true: the cursor is drawn, false: the cursor is hidden
 */
bool ta_get_cursor_show(obj_t * ta);

/**
 * Get the current cursor type.
 * @param ta pointer to a text area object
 * @return element of 'cursor_type_t'
 */
cursor_type_t ta_get_cursor_type(obj_t * ta);

/**
 * Get the password mode attribute
 * @param ta pointer to a text area object
 * @return true: password mode is enabled, false: disabled
 */
bool ta_get_pwd_mode(obj_t * ta);

/**
 * Get the one line configuration attribute
 * @param ta pointer to a text area object
 * @return true: one line configuration is enabled, false: disabled
 */
bool ta_get_one_line(obj_t * ta);

/**
 * Get the scroll bar mode of a text area
 * @param ta pointer to a text area object
 * @return scrollbar mode from 'page_sb_mode_t' enum
 */
static inline sb_mode_t ta_get_sb_mode(obj_t * ta)
{
    return page_get_sb_mode(ta);
}

/**
 * Get a style of a text area
 * @param ta pointer to a text area object
 * @param type which style should be get
 * @return style pointer to a style
 */
style_t * ta_get_style(obj_t *ta, ta_style_t type);

/*=====================
 * Other functions
 *====================*/

/**
 * Move the cursor one character right
 * @param ta pointer to a text area object
 */
void ta_cursor_right(obj_t * ta);

/**
 * Move the cursor one character left
 * @param ta pointer to a text area object
 */
void ta_cursor_left(obj_t * ta);

/**
 * Move the cursor one line down
 * @param ta pointer to a text area object
 */
void ta_cursor_down(obj_t * ta);

/**
 * Move the cursor one line up
 * @param ta pointer to a text area object
 */
void ta_cursor_up(obj_t * ta);

/**********************
 *      MACROS
 **********************/

#endif  /*GUI_USE_TA_H*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /*GUI_TA_H*/
