/**
 *@file themes.h
 *
 */

#ifndef GUI_THEMES_H
#define GUI_THEMES_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *    INCLUDES
 *********************/
#include "RTE_Include.h"

/*********************
 *    DEFINES
 *********************/

/**********************
 *    TYPEDEFS
 **********************/

typedef struct {
    style_t *bg;
    style_t *panel;

#if GUI_USE_CONT != 0
    style_t *cont;
#endif
#if GUI_USE_BTN != 0
    struct {
        style_t *rel;
        style_t *pr;
        style_t *tgl_rel;
        style_t *tgl_pr;
        style_t *ina;
    }btn;
#endif

#if GUI_USE_LABEL != 0
    struct {
        style_t *prim;
        style_t *sec;
        style_t *hint;
    }label;
#endif

#if GUI_USE_IMG != 0
    struct {
        style_t *light;
        style_t *dark;
    }img;
#endif

#if GUI_USE_LINE != 0
    struct {
        style_t *decor;
    }line;
#endif

#if GUI_USE_LED != 0
    style_t *led;
#endif

#if GUI_USE_BAR != 0
    struct {
        style_t *bg;
        style_t *indic;
    }bar;
#endif

#if GUI_USE_SLIDER != 0
    struct {
        style_t *bg;
        style_t *indic;
        style_t *knob;
    }slider;
#endif

#if GUI_USE_LMETER != 0
    style_t *lmeter;
#endif

#if GUI_USE_GAUGE != 0
    style_t *gauge;
#endif

#if GUI_USE_SW != 0
    struct {
        style_t *bg;
        style_t *indic;
        style_t *knob_off;
        style_t *knob_on;
    }sw;
#endif

#if GUI_USE_CHART != 0
    style_t *chart;
#endif

#if GUI_USE_CB != 0
    struct {
        style_t *bg;
        struct {
            style_t *rel;
            style_t *pr;
            style_t *tgl_rel;
            style_t *tgl_pr;
            style_t *ina;
        }box;
    }cb;
#endif

#if GUI_USE_BTNM != 0
    struct {
        style_t *bg;
        struct {
            style_t *rel;
            style_t *pr;
            style_t *tgl_rel;
            style_t *tgl_pr;
            style_t *ina;
        }btn;
    }btnm;
#endif

#if GUI_USE_KB != 0
    struct {
        style_t *bg;
        struct {
            style_t *rel;
            style_t *pr;
            style_t *tgl_rel;
            style_t *tgl_pr;
            style_t *ina;
        } btn;
    }kb;
#endif

#if GUI_USE_MBOX != 0
    struct {
        style_t *bg;
        struct {
            style_t *bg;
            style_t *rel;
            style_t *pr;
        }btn;
    }mbox;
#endif

#if GUI_USE_PAGE != 0
    struct {
        style_t *bg;
        style_t *scrl;
        style_t *sb;
    }page;
#endif

#if GUI_USE_TA != 0
    struct {
        style_t *area;
        style_t *oneline;
        style_t *cursor;
        style_t *sb;
    }ta;
#endif

#if GUI_USE_LIST
    struct {
        style_t *bg;
        style_t *scrl;
        style_t *sb;
        struct {
            style_t *rel;
            style_t *pr;
            style_t *tgl_rel;
            style_t *tgl_pr;
            style_t *ina;
        }btn;
    }list;
#endif

#if GUI_USE_DDLIST != 0
    struct {
        style_t *bg;
        style_t *sel;
        style_t *sb;
    }ddlist;
#endif

#if GUI_USE_ROLLER != 0
    struct {
        style_t *bg;
        style_t *sel;
    }roller;
#endif

#if GUI_USE_TABVIEW != 0
    struct {
        style_t *bg;
        style_t *indic;
        struct {
            style_t *bg;
            style_t *rel;
            style_t *pr;
            style_t *tgl_rel;
            style_t *tgl_pr;
        }btn;
    }tabview;
#endif

#if GUI_USE_WIN != 0
    struct {
        style_t *bg;
        style_t *sb;
        style_t *header;
        struct {
            style_t *bg;
            style_t *scrl;
        }content;
        struct {
            style_t *rel;
            style_t *pr;
        }btn;
    }win;
#endif
}theme_t;

/**********************
 *  GLOBAL PROTOTYPES
 **********************/

/**
 * Set a theme for the system.
 * From now, all the created objects will use styles from this theme by default
 * @param th pointer to theme (return value of: 'theme_init_xxx()')
 */
void theme_set_current(theme_t *th);

/**
 * Get the current system theme.
 * @return pointer to the current system theme. NULL if not set.
 */
theme_t * theme_get_current(void);

/**********************
 *    MACROS
 **********************/

/**********************
 *     POST INCLUDE
 *********************/
#include "GUI_Themes/GUI_Theme_Default.h"
#include "GUI_Themes/GUI_Theme_Alien.h"
#include "GUI_Themes/GUI_Theme_Night.h"
#include "GUI_Themes/GUI_Theme_Zen.h"
#include "GUI_Themes/GUI_Theme_Mono.h"
#include "GUI_Themes/GUI_Theme_Material.h"
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*GUI_THEMES_H*/
