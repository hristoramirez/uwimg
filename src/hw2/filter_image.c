#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "image.h"
#define TWOPI 6.2831853

void l1_normalize(image im)
{
    for (int row = 0; row < im.w; row++) {
        for (int col = 0; col < im.h; col++) {
            for (int i = 0; i < im.c; i++) {
                float val = 1.0 / (im.w * im.h);
                set_pixel(im, row, col, i, val);
            }
        }
    }
}

image make_box_filter(int w)
{
    image box_filter = make_image(w, w, 1);
    l1_normalize(box_filter);
    return box_filter;
}

image convolve_image(image im, image filter, int preserve)
{
    // Check that filter has 1 or the same number
    // of channels as im
    assert(im.c == filter.c || filter.c == 1);
    float q;
    image result;
    image temp;

    // Make image depending on preserve
    if (preserve) {
        result = make_image(im.w, im.h, im.c);
    } else {
        result = make_image(im.w, im.h, 1);
    }

    if (filter.c == im.c) {
        for (int i = 0; i < im.c; i++) {
            for (int row = 0; row < im.h; row++) {
                for (int col = 0; col < im.w; col++) {
                    q = 0.0;
                    for (int fy = 0; fy < filter.h; fy++) {
                        for (int fx = 0; fx < filter.w; fx++) {
                            int x = col - ( (filter.w / 2) - fx );
                            int y = row - ( (filter.h / 2) - fy );
                            float f_val = get_pixel(filter, x, y, i);
                            float i_val = get_pixel(im, col, row, i);
                            set_pixel(result, col, row, i, f_val * i_val);
                        }
                    }
                }
            }
        }
    }

    return result;
}

image make_highpass_filter()
{
    // TODO
    return make_image(1,1,1);
}

image make_sharpen_filter()
{
    // TODO
    return make_image(1,1,1);
}

image make_emboss_filter()
{
    // TODO
    return make_image(1,1,1);
}

// Question 2.2.1: Which of these filters should we use preserve when we run our convolution and which ones should we not? Why?
// Answer: TODO

// Question 2.2.2: Do we have to do any post-processing for the above filters? Which ones and why?
// Answer: TODO

image make_gaussian_filter(float sigma)
{
    // TODO
    return make_image(1,1,1);
}

image add_image(image a, image b)
{
    // TODO
    return make_image(1,1,1);
}

image sub_image(image a, image b)
{
    // TODO
    return make_image(1,1,1);
}

image make_gx_filter()
{
    // TODO
    return make_image(1,1,1);
}

image make_gy_filter()
{
    // TODO
    return make_image(1,1,1);
}

void feature_normalize(image im)
{
    // TODO
}

image *sobel_image(image im)
{
    // TODO
    return calloc(2, sizeof(image));
}

image colorize_sobel(image im)
{
    // TODO
    return make_image(1,1,1);
}
