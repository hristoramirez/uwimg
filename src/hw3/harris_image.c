#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "image.h"
#include "matrix.h"
#include <time.h>

#define ALPHA 0.06

// Frees an array of descriptors.
// descriptor *d: the array.
// int n: number of elements in array.
void free_descriptors(descriptor *d, int n)
{
    int i;
    for(i = 0; i < n; ++i){
        free(d[i].data);
    }
    free(d);
}

// Create a feature descriptor for an index in an image.
// image im: source image.
// int i: index in image for the pixel we want to describe.
// returns: descriptor for that index.
descriptor describe_index(image im, int i)
{
    int w = 5;
    descriptor d;
    d.p.x = i%im.w;
    d.p.y = i/im.w;
    d.data = calloc(w*w*im.c, sizeof(float));
    d.n = w*w*im.c;
    int c, dx, dy;
    int count = 0;
    // If you want you can experiment with other descriptors
    // This subtracts the central value from neighbors
    // to compensate some for exposure/lighting changes.
    for(c = 0; c < im.c; ++c){
        float cval = im.data[c*im.w*im.h + i];
        for(dx = -w/2; dx < (w+1)/2; ++dx){
            for(dy = -w/2; dy < (w+1)/2; ++dy){
                float val = get_pixel(im, i%im.w+dx, i/im.w+dy, c);
                d.data[count++] = cval - val;
            }
        }
    }
    return d;
}

// Marks the spot of a point in an image.
// image im: image to mark.
// ponit p: spot to mark in the image.
void mark_spot(image im, point p)
{
    int x = p.x;
    int y = p.y;
    int i;
    for(i = -9; i < 10; ++i){
        set_pixel(im, x+i, y, 0, 1);
        set_pixel(im, x, y+i, 0, 1);
        set_pixel(im, x+i, y, 1, 0);
        set_pixel(im, x, y+i, 1, 0);
        set_pixel(im, x+i, y, 2, 1);
        set_pixel(im, x, y+i, 2, 1);
    }
}

// Marks corners denoted by an array of descriptors.
// image im: image to mark.
// descriptor *d: corners in the image.
// int n: number of descriptors to mark.
void mark_corners(image im, descriptor *d, int n)
{
    int i;
    for(i = 0; i < n; ++i){
        mark_spot(im, d[i].p);
    }
}

// Creates a 1d Gaussian filter.
// float sigma: standard deviation of Gaussian.
// returns: single row image of the filter.
image make_1d_gaussian(float sigma)
{
    int size = roundf(6.0 * sigma);
    size = (size % 2 == 0) ? size + 1 : size;
    
    image filter = make_image(size, 1, 1);
    float v;
    for (int i = 0; i < size; i++) {
        // Adjust coordinates
        int x = i - size / 2;
        v = 1.0 / (sqrtf(TWOPI) * sigma);
        v = v * expf(-1.0 * (x * x) / (2.0 * sigma * sigma));
        set_pixel(filter, i, 0, 0, v);
    }

    l1_normalize(filter);
    return filter;
}

// Smooths an image using separable Gaussian filter.
// image im: image to smooth.
// float sigma: std dev. for Gaussian.
// returns: smoothed image.
image smooth_image(image im, float sigma)
{
    if(0){
        image g = make_gaussian_filter(sigma);
        image s = convolve_image(im, g, 1);
        free_image(g);
        return s;
    } else {
        // If you implement, disable the above if check.
        image gx = make_1d_gaussian(sigma);
        image gy = make_image(1, gx.w, 1);
        // Flip N x 1 -> 1 x N
        float v;
        for (int i = 0; i < gy.h; i++) {
            v = get_pixel(gx, i, 0, 0);
            set_pixel(gy, 0, i, 0, v);
        }

        image sx = convolve_image(im, gx, 1);
        image sy = convolve_image(sx, gy, 1);

        free_image(gx);
        free_image(gy);
        free_image(sx);

        return sy;
    }
}

// Calculate the structure matrix of an image.
// image im: the input image.
// float sigma: std dev. to use for weighted sum.
// returns: structure matrix. 1st channel is Ix^2, 2nd channel is Iy^2,
//          third channel is IxIy.
image structure_matrix(image im, float sigma)
{
    image S = make_image(im.w, im.h, 3);
    float x, y;
    
    // Derivative
    image fx = make_gx_filter();
    image fy = make_gy_filter();
    image ix = convolve_image(im, fx, 0);
    image iy = convolve_image(im, fy, 0);
    
    // Calculate IxIx, IyIy, IxIy
    for (int row = 0; row < im.h; row++) {
        for (int col = 0; col < im.w; col++) {
            x = get_pixel(ix, col, row, 0);
            y = get_pixel(iy, col, row, 0);

            set_pixel(S, col, row, 0, x * x);
            set_pixel(S, col, row, 1, y * y);
            set_pixel(S, col, row, 2, x * y);
        }
    }

    // Weighted Sum of Nearby
    S = smooth_image(S, sigma);

    return S;
}

// Estimate the cornerness of each pixel given a structure matrix S.
// image S: structure matrix for an image.
// returns: a response map of cornerness calculations.
image cornerness_response(image S)
{
    image R = make_image(S.w, S.h, 1);
    // TODO: fill in R, "cornerness" for each pixel using the structure matrix.
    // We'll use formulation det(S) - alpha * trace(S)^2, alpha = .06.

    // Each pixel has structure matrix
    // | a  b |  ->  | IxIx  IxIy |
    // | c  d |      | IxIy  IyIy |
    float det, trace, a, b, d;
    for (int row = 0; row < S.h; row++) {
        for (int col = 0; col < S.w; col++) {
            // Determinant: ad - bc, here b == c, ad - bb
            a = get_pixel(S, col, row, 0); // wIxIx
            d = get_pixel(S, col, row, 1); // wIyIy
            b = get_pixel(S, col, row, 2); // wIxIy
            det = a * d - b * b;

            // Trace: a + d
            trace = a + d;

            // cornerness using formula
            set_pixel(R, col, row, 0, det - ALPHA * trace * trace);
        }
    }

    return R;
}

// Perform non-max supression on an image of feature responses.
// image im: 1-channel image of feature responses.
// int w: distance to look for larger responses.
// returns: image with only local-maxima responses within w pixels.
image nms_image(image im, int w)
{
    assert(im.c == 1 && w >= 1);
    image r = copy_image(im);
    // TODO: perform NMS on the response map.
    // for every pixel in the image:
    //     for neighbors within w:
    //         if neighbor response greater than pixel response:
    //             set response to be very low (I use -999999 [why not 0??])
    float v, nbr;
    for (int row = 0; row < im.h; row++) {
        for (int col = 0; col < im.w; col++) {
            v = get_pixel(im, col, row, 0);
            for (int y = row - w; y <= row + w; y++) {
                for (int x = col - w; x <= col + w; x++) {
                    // Check Neighbors
                    nbr = get_pixel(im, x, y, 0);
                    if (nbr > v) {
                        set_pixel(r, col, row, 0, -999999);
                    }
                }
            }
        }
    }
    
    return r;
}

// Perform harris corner detection and extract features from the corners.
// image im: input image.
// float sigma: std. dev for harris.
// float thresh: threshold for cornerness.
// int nms: distance to look for local-maxes in response map.
// int *n: pointer to number of corners detected, should fill in.
// returns: array of descriptors of the corners in the image.
descriptor *harris_corner_detector(image im, float sigma, float thresh, int nms, int *n)
{
    // Calculate structure matrix
    image S = structure_matrix(im, sigma);

    // Estimate cornerness
    image R = cornerness_response(S);

    // Run NMS on the responses
    image Rnms = nms_image(R, nms);

    //TODO: count number of responses over threshold
    int count = 0;
    for (int row = 0; row < Rnms.h; row++) {
        for (int col = 0; col < Rnms.w; col++) {
            if (get_pixel(Rnms, col, row, 0) >= thresh) {
                count++;
            }
        }
    }

    printf("count: %d\n", count);
    *n = count; // <- set *n equal to number of corners in image.
    descriptor *d = calloc(count, sizeof(descriptor));
    //TODO: fill in array *d with descriptors of corners, use describe_index.
    int i = 0;
    for (int row = 0; row < Rnms.h; row++) {
        for (int col = 0; col < Rnms.w; col++) {
            if (get_pixel(Rnms, col, row, 0) > thresh) {
                int index = col + (row * im.w);
                *(d + i) = describe_index(Rnms, index);
                i++;
            }
        }
    }

    free_image(S);
    free_image(R);
    free_image(Rnms);
    return d;
}

// Find and draw corners on an image.
// image im: input image.
// float sigma: std. dev for harris.
// float thresh: threshold for cornerness.
// int nms: distance to look for local-maxes in response map.
void detect_and_draw_corners(image im, float sigma, float thresh, int nms)
{
    int n = 0;
    descriptor *d = harris_corner_detector(im, sigma, thresh, nms, &n);
    mark_corners(im, d, n);
}
