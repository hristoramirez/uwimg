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
static float GX_KERNEL[] = {
    -1, 0, 1,
    -2, 0, 2,
    -1, 0, 1
};
static float GY_KERNEL[] = {
    -1, -2, -1,
    0, 0, 0,
    1, 2, 1 
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
}

image make_box_filter(int w)
{
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
    return make_filter(HIGHPASS_KERNEL, K_DIM);
}

image make_sharpen_filter()
{
    return make_filter(SHARPEN_KERNEL, K_DIM);
}

image make_emboss_filter()
{
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
    image filter;
    // Determine the size of filter
    int size = roundf(6.0 * sigma);
    size = (size % 2 == 0) ? size + 1 : size;
    
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

    filter = make_filter(gaussian, size);
    l1_normalize(filter);

    return filter;
}

image add_image(image a, image b)
{
    // Check that images can actually be added
    assert(a.c == b.c && a.w == b.w && a.h == b.h);    
    image result = make_image(a.w, a.h, a.c);
    float a_val;
    float b_val;

    for (int c = 0; c < a.c; c++) {
        for (int row = 0; row < a.h; row++) {
            for (int col = 0; col < a.w; col++) {
                a_val = get_pixel(a, col, row, c);
                b_val = get_pixel(b, col, row, c);
                set_pixel(result, col, row, c, a_val + b_val);
            }
        }
    }

    return result;
}

image sub_image(image a, image b)
{
    // Check that images can actually be added
    assert(a.c == b.c && a.w == b.w && a.h == b.h);    
    image result = make_image(a.w, a.h, a.c);
    float a_val;
    float b_val;

    for (int c = 0; c < a.c; c++) {
        for (int row = 0; row < a.h; row++) {
            for (int col = 0; col < a.w; col++) {
                a_val = get_pixel(a, col, row, c);
                b_val = get_pixel(b, col, row, c);
                set_pixel(result, col, row, c, a_val - b_val);
            }
        }
    }

    return result;
}

image make_gx_filter()
{
    return make_filter(GX_KERNEL, K_DIM);
}

image make_gy_filter()
{
    return make_filter(GY_KERNEL, K_DIM);
}

void feature_normalize(image im)
{
    float min = INFINITY;
    float max = 0.0;
    float val;

    for (int c = 0; c < im.c; c++) {
        // Find the min and max of the image
        for (int row = 0; row < im.h; row++) {
            for (int col = 0; col < im.w; col++) {
                val = get_pixel(im, col, row, c);
                min = MIN(val, min);
                max = MAX(val, max);
            }
        }

        for (int row = 0; row < im.h; row++) {
            for (int col = 0; col < im.w; col++) {
                if (max - min > 0.0) {
                    val = get_pixel(im, col, row, c);
                    val = (val - min) / (max - min);
                } else {
                    val = 0.0;
                }
                set_pixel(im, col, row, c, val);
            }
        }
    }
}

image *sobel_image(image im)
{
    // Prep
    image* result = calloc(2, sizeof(image));
    result[0] = make_image(im.w, im.h, 1); // magnitude
    result[1] = make_image(im.w, im.h, 1); // direction

    image fx = make_gx_filter();
    image fy = make_gy_filter();

    image gx = convolve_image(im, fx, 0);
    image gy = convolve_image(im, fy, 0);

    float x, y;
    for (int row = 0; row < im.h; row++) {
        for (int col = 0; col < im.w; col++) {
            x = get_pixel(gx, col, row, 0);
            y = get_pixel(gy, col, row, 0);
            set_pixel(result[0], col, row, 0, sqrtf(x * x + y * y));
            set_pixel(result[1], col, row, 0, atan2f(y, x));
        }
    }

    free_image(fx);
    free_image(fy);
    free_image(gx);
    free_image(gy);
    return result;
}

image colorize_sobel(image im)
{
    image result = make_image(im.w, im.h, im.c);
    image filter = make_gaussian_filter(3.0);
    im = convolve_image(im, filter, 1);
    image* sobel = sobel_image(im);
    float h, s;
    
    // Process as hsv
    feature_normalize(sobel[0]);
    feature_normalize(sobel[1]);
    for (int row = 0; row < im.h; row++) {
        for (int col = 0; col < im.w; col++) {
            s = get_pixel(sobel[0], col, row, 0);
            h = get_pixel(sobel[1], col, row, 0);
            set_pixel(result, col, row, 0, h);
            set_pixel(result, col, row, 1, s);
            set_pixel(result, col, row, 2, s);
        }
    }
    // Convert to rgb
    hsv_to_rgb(result);


    free_image(filter);
    free_image(sobel[0]);
    free_image(sobel[1]);
    return result;
}
