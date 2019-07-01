#ifndef __GUI_FONT_H
#define __GUI_FONT_H
#ifdef __cplusplus
extern "C" {
#endif
#include "RTE_Include.h"
/**********************
 *      TYPEDEFS
 **********************/

typedef struct
{
    uint32_t w_px         :8;
    uint32_t glyph_index  :24;
}font_glyph_dsc_t;

typedef struct
{
    uint32_t unicode         :21;
    uint32_t glyph_dsc_index :11;
}font_unicode_map_t;

typedef struct _font_struct
{
    uint32_t unicode_first;
    uint32_t unicode_last;
    uint8_t h_px;
    const uint8_t * glyph_bitmap;
    const font_glyph_dsc_t * glyph_dsc;
    const uint32_t * unicode_list;
    const uint8_t * (*get_bitmap)(const struct _font_struct * ,uint32_t);    /*Get a glyph's  bitmap from a font*/
    int16_t (*get_width)(const struct _font_struct * ,uint32_t);       /*Get a glyph's with with a given font*/
    struct _font_struct * next_page;    /*Pointer to a font extension*/
    uint32_t bpp   :4;                     /*Bit per pixel: 1, 2 or 4*/
}font_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize the built-in fonts
 */
void font_init(void);

/**
 * Create a pair from font name and font dsc. get function. After it 'font_get' can be used for this font
 * @param child pointer to a font to join to the 'parent'
 * @param parent pointer to a font. 'child' will be joined here
 */
void font_add(font_t *child, font_t *parent);

/**
 * Return with the bitmap of a font.
 * @param font_p pointer to a font
 * @param letter a letter
 * @return  pointer to the bitmap of the letter
 */
const uint8_t * font_get_bitmap(const font_t * font_p, uint32_t letter);

/**
 * Get the height of a font
 * @param font_p pointer to a font
 * @return the height of a font
 */
static inline uint8_t font_get_height(const font_t * font_p)
{
    return font_p->h_px;
}

/**
 * Get the width of a letter in a font
 * @param font_p pointer to a font
 * @param letter a letter
 * @return the width of a letter
 */
uint8_t font_get_width(const font_t * font_p, uint32_t letter);

/**
 * Get the bit-per-pixel of font
 * @param font pointer to font
 * @param letter a letter from font (font extensions can have different bpp)
 * @return bpp of the font (or font extension)
 */
uint8_t font_get_bpp(const font_t * font, uint32_t letter);

/**
 * Generic bitmap get function used in 'font->get_bitmap' when the font contains all characters in the range
 * @param font pointer to font
 * @param unicode_letter an unicode letter which bitmap should be get
 * @return pointer to the bitmap or NULL if not found
 */
const uint8_t * font_get_bitmap_continuous(const font_t * font, uint32_t unicode_letter);

/**
 * Generic bitmap get function used in 'font->get_bitmap' when the font NOT contains all characters in the range (sparse)
 * @param font pointer to font
 * @param unicode_letter an unicode letter which bitmap should be get
 * @return pointer to the bitmap or NULL if not found
 */
const uint8_t * font_get_bitmap_sparse(const font_t * font, uint32_t unicode_letter);
/**
 * Generic glyph width get function used in 'font->get_width' when the font contains all characters in the range
 * @param font pointer to font
 * @param unicode_letter an unicode letter which width should be get
 * @return width of the gylph or -1 if not found
 */
int16_t font_get_width_continuous(const font_t * font, uint32_t unicode_letter);

/**
 * Generic glyph width get function used in 'font->get_bitmap' when the font NOT contains all characters in the range (sparse)
 * @param font pointer to font
 * @param unicode_letter an unicode letter which width should be get
 * @return width of the glyph or -1 if not found
 */
int16_t font_get_width_sparse(const font_t * font, uint32_t unicode_letter);

/**********************
 *      MACROS
 **********************/
#define FONT_DECLARE(font_name) extern font_t font_name;

/******************************
 *  FONT DECLARATION INCLUDES
 *****************************/

/*10 px */
#if USE_FONT_DEJAVU_10
FONT_DECLARE(font_dejavu_10);
#endif

#if USE_FONT_DEJAVU_10_LATIN_SUP
FONT_DECLARE(font_dejavu_10_latin_sup);
#endif

#if USE_FONT_DEJAVU_10_CYRILLIC
FONT_DECLARE(font_dejavu_10_cyrillic);
#endif

#if USE_FONT_SYMBOL_10
FONT_DECLARE(font_symbol_10);
#endif

/*20 px */
#if USE_FONT_DEJAVU_20
FONT_DECLARE(font_dejavu_20);
#endif

#if USE_FONT_DEJAVU_20_LATIN_SUP
FONT_DECLARE(font_dejavu_20_latin_sup);
#endif

#if USE_FONT_DEJAVU_20_CYRILLIC
FONT_DECLARE(font_dejavu_20_cyrillic);
#endif

#if USE_FONT_SYMBOL_20
FONT_DECLARE(font_symbol_20);
#endif

/*30 px */
#if USE_FONT_DEJAVU_30
FONT_DECLARE(font_dejavu_30);
#endif

#if USE_FONT_DEJAVU_30_LATIN_SUP
FONT_DECLARE(font_dejavu_30_latin_sup);
#endif

#if USE_FONT_DEJAVU_30_CYRILLIC
FONT_DECLARE(font_dejavu_30_cyrillic);
#endif

#if USE_FONT_SYMBOL_30
FONT_DECLARE(font_symbol_30);
#endif

/*40 px */
#if USE_FONT_DEJAVU_40
FONT_DECLARE(font_dejavu_40);
#endif

#if USE_FONT_DEJAVU_40_LATIN_SUP
FONT_DECLARE(font_dejavu_40_latin_sup);
#endif

#if USE_FONT_DEJAVU_40_CYRILLIC
FONT_DECLARE(font_dejavu_40_cyrillic);
#endif

#if USE_FONT_SYMBOL_40
FONT_DECLARE(font_symbol_40);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
