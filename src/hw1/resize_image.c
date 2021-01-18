#include <math.h>
#include "image.h"

typedef float interpolateFn(image, float, float, int);

image resize(image im, int w, int h, interpolateFn fn) {
    // Solve system of equations
    float ax, ay, bx, by, xi, yi, val;

    image resized = make_image(w, h, im.c);

    // Solve for a
    ax = 1.0 * im.w / w;
    ay = 1.0 * im.h / h;
;    
    // Solve for b
    // a(-0.5) + b = -0.5
    // b = -0.5 + 0.5(a)
    bx = 0.5 * (ax - 1.0);
    by = 0.5 * (ay - 1.0);

    // Iterate over new points
    for (int row = 0; row < w; row++) {
        for (int col = 0; col < h; col++) {
            xi = ax * row + bx;
            yi = ay * col + by;
            for (int i = 0; i < im.c; i++) {
                val = fn(im, xi, yi, i);
                set_pixel(resized, row, col, i, val);
            }
        }
    }

    return resized;
}

float nn_interpolate(image im, float x, float y, int c)
{
    return get_pixel(im, roundf(x), roundf(y), c);
}

image nn_resize(image im, int w, int h)
{
    return resize(im, w, h, nn_interpolate);
}

float bilinear_interpolate(image im, float x, float y, int c)
{
    // float top, bottom, left, right;
    float top, bottom, left, right;
    float d1, d2, d3, d4;
    float q1, q2;

    //      d1      |   d2
    // tl.......................tr
    // .                       .
    // .                       . d3
    // .                       . -
    // .                       .
    // .    (x,y)              . d4
    // .                       .
    // .                       .
    // bl......................br

    top = floorf(y);
    bottom = ceilf(y);
    left = floorf(x);
    right = ceilf(x);

    d1 = x - left;
    d2 = right - x;
    d3 = y - top;
    d4 = bottom - y;

    // Calculate q1 and q2 component for channel c
    q1 = d4 * get_pixel(im, left, top, c) + d3 * get_pixel(im, left, bottom, c);
    q2 = d4 * get_pixel(im, right, top, c) + d3 * get_pixel(im, right, bottom, c);

    return d2 * q1 + d1 * q2;
}

image bilinear_resize(image im, int w, int h)
{
    return resize(im, w, h, bilinear_interpolate);
}