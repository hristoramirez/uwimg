#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "image.h"
#define TWOPI 6.2831853
#define K_DIM 3

// Kernel values
static float HIGHPASS_KERNEL[] = {
        0, -1, 0,
        -1, 4, -1,
        0, -1, 0
    };
static float SHARPEN_KERNEL[] = {
        0, -1, 0,
        -1, 5, -1,
        0, -1, 0
    };
static float EMBOSS_KERNEL[] = {
        -2, -1, 0,
        -1, 1, 1,
        0, 1, 2
    };

// Creates a dim x dim filter from the given values
image make_filter(float* values, int dim) {
    image filter = make_image(dim, dim, 1);

    for (int row = 0; row < dim; row++) {
        for (int col = 0; col < dim; col++) {
            set_pixel(filter, col, row, 0, *(values + row * dim + col));
        }
    }

    return filter;
}

// // Creates a K_DIM x K_DIM kernel from the given values
// image make_filter(float values[K_DIM][K_DIM]) {
//     image filter = make_image(K_DIM, K_DIM, 1);

//     for (int row = 0; row < K_DIM; row++) {
//         for (int col = 0; col < K_DIM; col++) {
//             set_pixel(filter, col, row, 0, values[row][col]);
//         }
//     }

//     return filter;
// }

void l1_normalize(image im)
{
    float sum;
    float val;
    for (int c = 0; c < im.c; c++) {
        sum = 0.0;
        for (int row = 0; row < im.h; row++) {
            for (int col = 0; col < im.w; col++) {
                sum += get_pixel(im, col, row, c);
            }
        }

        for (int row = 0; row < im.h; row++) {
            for (int col = 0; col < im.w; col++) {
                val = (1.0 / sum) * get_pixel(im, col, row, c);
                set_pixel(im, col, row, c, val);
            }
        }
    }
    // float val = 1.0 / (im.w * im.h);
    // for (int i = 0; i < im.c; i++) {
    //     for (int row = 0; row < im.h; row++) {
    //         for (int col = 0; col < im.w; col++) {
    //             set_pixel(im, col, row, i, val);
    //         }
    //     }
    // }
}

image make_box_filter(int w)
{
    // image box_filter = make_image(w, w, 1);
    // l1_normalize(box_filter);
    // return box_filter;

    image filter = make_image(w, w, 1);
    
    // Fill with ones
    for (int row = 0; row < w; row++) {
        for (int col = 0; col < w; col++) {
            set_pixel(filter, col, row, 0, 1.0);
        }
    }

    // Normalize
    l1_normalize(filter);
    return filter;
}

image convolve_image(image im, image filter, int preserve)
{
    // Check that filter has 1 or the same number
    // of channels as im
    assert(im.c == filter.c || filter.c == 1);
    assert(preserve == 0 || preserve == 1);
    float q;
    image result;

    // Make image depending on preserve
    if (preserve) {
        result = make_image(im.w, im.h, im.c);
    } else {
        result = make_image(im.w, im.h, 1);
    }

    if (filter.c == im.c) {
        for (int c = 0; c < im.c; c++) {
            for (int row = 0; row < im.h; row++) {
                for (int col = 0; col < im.w; col++) {
                    q = 0.0;
                    for (int fy = 0; fy < filter.h; fy++) {
                        for (int fx = 0; fx < filter.w; fx++) {
                            int x = col - ( (filter.w / 2) - fx );
                            int y = row - ( (filter.h / 2) - fy );
                            float f_val = get_pixel(filter, fx, fy, c);
                            float i_val = get_pixel(im, x, y, c);
                            q += f_val * i_val;
                        }
                    }
                    set_pixel(result, col, row, c, q);
                }
            }
        }
    } else if (filter.c == 1 && im.c > 1) {
        // Apply the filter to each channel
        for (int c = 0; c < im.c; c++) {
            for (int row = 0; row < im.h; row++) {
                for (int col = 0; col < im.w; col++) {
                    q = 0.0;
                    for (int fy = 0; fy < filter.h; fy++) {
                        for (int fx = 0; fx < filter.w; fx++) {
                            int x = col - ( (filter.w / 2) - fx );
                            int y = row - ( (filter.h / 2) - fy );
                            float f_val = get_pixel(filter, fx, fy, 0);
                            float i_val = get_pixel(im, x, y, c);
                            q += f_val * i_val;
                        }
                    }
                    if (preserve) {
                        set_pixel(result, col, row, c, q);
                    } else {
                        float curr_sum = get_pixel(result, col, row, 0);
                        set_pixel(result, col, row, 0, curr_sum + q);
                    }
                }
            }
        }
    }

    return result;
}

image make_highpass_filter()
{
    // float values[K_DIM * K_DIM] = {
    //     0, -1, 0,
    //     -1, 4, -1,
    //     0, -1, 0
    // };

    return make_filter(HIGHPASS_KERNEL, K_DIM);
}

image make_sharpen_filter()
{
    // float values[K_DIM * K_DIM] = {
    //     0, -1, 0,
    //     -1, 5, -1,
    //     0, -1, 0
    // };

    return make_filter(SHARPEN_KERNEL, K_DIM);
}

image make_emboss_filter()
{
    // float values[K_DIM * K_DIM] = {
    //     -2, -1, 0,
    //     -1, 1, 1,
    //     0, 1, 2
    // };

    return make_filter(EMBOSS_KERNEL, K_DIM);
}

// Question 2.2.1: Which of these filters should we use preserve when we run our convolution and which ones should we not? Why?
// Answer:
//      We should use preserve on box, emboss, and sharpen filters. These filters
//      are mainly stylistic filters that should apply independently to each channel.
//      The highpass filter should not use preserve because the filter should
//      transform the original image into a single channel for use in edge detection.

// Question 2.2.2: Do we have to do any post-processing for the above filters? Which ones and why?
// Answer: highpass, emboss, sharpen
//      Highpass, emboss, and sharpen require additional post-processing because
//      their kernels have potential to overflow the values at channels since they
//      are not normalized. For example the sharpen kernel would set a pixel surrounded
//      by lower value pixels to exceed 1. And also the highpass filter projects the
//      information of mutliple color channels into 1 channel.

image make_gaussian_filter(float sigma)
{
    // Determine the size of filter
    int size = roundf(6.0 * sigma);
    size = (size % 2 == 0) ? size + 1 : size;
    //image filter = make_image(size, size, 1);
    
    // Calculate using 2d gaussian formula
    float gaussian[size * size];
    float val;
    for (int row = 0; row < size; row++) {
        for (int col = 0; col < size; col++) {
            // Adjust coordinates so center is (0,0)
            int x = -1.0 * ( (size / 2) - col );
            int y = -1.0 * ( (size / 2) - row );
            val = 1.0 / (TWOPI * sigma * sigma);
            val = val * expf(-1.0 * (x * x + y * y) / (2.0 * sigma * sigma));
            gaussian[row * size + col] = val;
            //set_pixel(filter, col, row, 0, val);
        }
    }

    image filter = make_filter(gaussian, size);
    l1_normalize(filter);

    return filter;
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
