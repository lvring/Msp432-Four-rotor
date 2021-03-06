#include "RTE_Include.h"
/*****************************************************************************
*** Author: Shannon
*** Version: 1.0 2018.10.09
*** History: 1.0 �������޸���lvgl��https://littlevgl.com/
*****************************************************************************/
#if RTE_USE_GUI == 1
/**
 * Convert a HSV color to RGB
 * @param h hue [0..359]
 * @param s saturation [0..100]
 * @param v value [0..100]
 * @return the given RGB color in RGB (with GUI_COLOR_DEPTH depth)
 */
color_t color_hsv_to_rgb(uint16_t h, uint8_t s, uint8_t v)
{
    h = (uint32_t)((uint32_t)h * 255) / 360;
    s = (uint16_t)((uint16_t)s * 255) / 100;
    v = (uint16_t)((uint16_t)v * 255) / 100;

    uint8_t r, g, b;

    uint8_t region, remainder, p, q, t;

    if (s == 0)
    {
        r = v;
        g = v;
        b = v;
        return GUI_COLOR_MAKE(v, v, v);
    }

    region = h / 43;
    remainder = (h - (region * 43)) * 6;

    p = (v * (255 - s)) >> 8;
    q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

    switch (region)
    {
        case 0:
            r = v; g = t; b = p;
            break;
        case 1:
            r = q; g = v; b = p;
            break;
        case 2:
            r = p; g = v; b = t;
            break;
        case 3:
            r = p; g = q; b = v;
            break;
        case 4:
            r = t; g = p; b = v;
            break;
        default:
            r = v; g = p; b = q;
            break;
    }

    color_t result = GUI_COLOR_MAKE(r, g, b);
    return result;
}

/**
 * Convert an RGB color to HSV
 * @param r red
 * @param g green
 * @param b blue
 * @return the given RGB color n HSV
 */
color_hsv_t color_rgb_to_hsv(uint8_t r, uint8_t g, uint8_t b)
{
    color_hsv_t hsv;
    uint8_t rgbMin, rgbMax;

    rgbMin = r < g ? (r < b ? r : b) : (g < b ? g : b);
    rgbMax = r > g ? (r > b ? r : b) : (g > b ? g : b);

    hsv.v = rgbMax;
    if (hsv.v == 0)
    {
        hsv.h = 0;
        hsv.s = 0;
        return hsv;
    }

    hsv.s = 255 * (long)(rgbMax - rgbMin) / hsv.v;
    if (hsv.s == 0)
    {
        hsv.h = 0;
        return hsv;
    }
    if (rgbMax == r)
     hsv.h = 0 + 43 * (g - b) / (rgbMax - rgbMin);
    else if (rgbMax == g)
     hsv.h = 85 + 43 * (b - r) / (rgbMax - rgbMin);
    else
     hsv.h = 171 + 43 * (r - g) / (rgbMax - rgbMin);
    return hsv;
}

#endif

