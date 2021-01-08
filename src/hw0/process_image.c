#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "image.h"

int get_offset(image im, int x, int y, int c) {
    return x + (y * im.w) + (c * im.w * im.h);
}

float get_pixel(image im, int x, int y, int c)
{
    // Clamp the image if needed
    if (x < 0) {
        x = 0;
    } else if (x + 1 > im.w) {
        x = im.w - 1;
    }

    if (y < 0) {
        y = 0;
    } else if (y + 1 > im.h) {
        y = im.h - 1;
    }

    int offset = get_offset(im, x, y, c);

    return *(im.data + offset);
}

void set_pixel(image im, int x, int y, int c, float v)
{
    if ((x >= 0) && (x < im.w) && (y >= 0) && (y < im.h)) { // not sure if need to check for c
        int offset = get_offset(im, x, y, c);
        *(im.data + offset) = v;
    }
}

image copy_image(image im)
{
    image copy = make_image(im.w, im.h, im.c);
    memcpy(copy.data, im.data, im.w * im.h * im.c * sizeof(float));

    return copy;
}

image rgb_to_grayscale(image im)
{
    assert(im.c == 3);
    image gray = make_image(im.w, im.h, 1);

    for (int row = 0; row < im.w; row++) {
        for (int col = 0; col < im.h; col++) {
            float gR = 0.299 * get_pixel(im, row, col, 0);
            float gG = 0.587 * get_pixel(im, row, col, 1);
            float gB = 0.114 * get_pixel(im, row, col, 2);
            set_pixel(gray, row, col, 0, gR + gG + gB);
        }
    }

    return gray;
}

void shift_image(image im, int c, float v)
{
    for (int row = 0; row < im.w; row++) {
        for (int col = 0; col < im.h; col++) {
            float shifted = v + get_pixel(im, row, col, c);
            set_pixel(im, row, col, c, shifted);
        }
    }
}

void clamp_image(image im)
{
    for (int row = 0; row < im.w; row++) {
        for (int col = 0; col < im.h; col++) {
            for (int c = 0; c < im.c; c++) {
                float v = get_pixel(im, row, col, c);
                
                if (v > 1) {
                    set_pixel(im, row, col, c, 1);
                } else if (v < 0) {
                    set_pixel(im, row, col, c, 0);
                }
            }
        }
    }
}

// These might be handy
float three_way_max(float a, float b, float c)
{
    return (a > b) ? ( (a > c) ? a : c) : ( (b > c) ? b : c) ;
}

float three_way_min(float a, float b, float c)
{
    return (a < b) ? ( (a < c) ? a : c) : ( (b < c) ? b : c) ;
}

void rgb_to_hsv(image im)
{
    float hue, saturation, value;
    float r, g, b;

    for (int row = 0; row < im.w; row++) {
        for (int col = 0; col < im.h; col++) {
            r = get_pixel(im, row, col, 0);
            g = get_pixel(im, row, col, 1);
            b = get_pixel(im, row, col, 2);
            
            // Value
            value = three_way_max(r, g, b);

            // Saturation
            float m = three_way_min(r, g, b);
            float c = value - m;
            saturation = (value == 0) ? 0 : (c / value);

            // Hue
            float h;
            if (value == r) {
                h = (g - b) / c;
            } else if (value == g) {
                h = 2 + (b - r) / c;
            } else if (value == b) {
                h = 4 + (r - g) / c;
            }

            if (c == 0) {
                hue = 0;
            } else {
                hue = (h < 0) ? (1 + h / 6) : (h / 6);
            }

            // Set to HSV
            set_pixel(im, row, col, 0, hue);
            set_pixel(im, row, col, 1, saturation);
            set_pixel(im, row, col, 2, value);
        }
    }
}

void hsv_to_rgb(image im)
{
    float hue, saturation, value;
    float r, g, b;

    for (int row = 0; row < im.w; row++) {
        for (int col = 0; col < im.h; col++) {
            hue = get_pixel(im, row, col, 0);
            saturation = get_pixel(im, row, col, 1);
            value = get_pixel(im, row, col, 2);

            float h, c, m;
            c = saturation * value;


            // shade of gray, set to the value
            if (c == 0) {
                set_pixel(im, row, col, 0, value);
                set_pixel(im, row, col, 1, value);
                set_pixel(im, row, col, 2, value);

                continue;
            }

            // Other color
            m = value - c;
            h = 6 * hue;

            


            

        }
    }
}
