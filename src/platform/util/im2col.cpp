/*
 * Helpful links to understand im2col:
 * https://petewarden.com/2015/04/20/why-gemm-is-at-the-heart-of-deep-learning/
 * http://cs231n.github.io/convolutional-networks/
 * https://github.com/BVLC/caffe
 */

#include <cstring>
#include "im2col.h"

namespace helper {

    /**
    * Checks if a given index lies in the interval [0,boundary).
    *
    * The use case for this method is to check whether a row/column of a matrix is within the boundaries of an image
    * or whether the value of the element in the row/column is zero due to zero-padding.
    *
    * @param index      the index of a row/column of an image (2D matrix)
    * @param boundary   is a positive value
    * @return true      if value is in [0,boundary), false if not.
    */
    inline bool is_value_ge_zero_and_value_lt_boundary(int index, int boundary) {
        /*
         * casting from int to unsigned because we then need to check only one condition instead of two.
         * The boundary parameter is of type signed and is always positive, therefore its value is always lower than 0x800
         * casting a negative value of a parameter converts it to value higher than 0x800
         * thus checking value < boundary is sufficient
         */
        return static_cast<unsigned>(index) < static_cast<unsigned>(boundary);
    }

    template<typename Dtype>
    void set(const int object_size, const Dtype fill_byte, Dtype *object_to_fill) {
        if (fill_byte == 0) {
            std::memset(object_to_fill, 0, sizeof(Dtype) * object_size);
            return;
        }
        for (int i = 0; i < object_size; ++i) {
            object_to_fill[i] = fill_byte;
        }
    }

    // Explicit instantiation
    template void set<int>(const int object_size, const int fill_byte, int *object_to_fill);

    template void set<float>(const int object_size, const float fill_byte, float *object_to_fill);

    template void set<double>(const int object_size, const double fill_byte, double *object_to_fill);

    /*
     * Example:
     * Image 3x3, Kernel 2x2, Padding = 0, Stride = 1
     * Image:
     * 1 2 3
     * 4 5 6
     * 7 8 9
     *
     * 4 possible Filter positions for 2x2 Filter with no Padding
     * top-left quadrant:
     * 1 2
     * 4 5
     * is written as 1 2 4 5
     * top-right quadrant:
     * 2 3
     * 5 6
     * is written as 2 3 5 6
     * etc.
     *
     * Results in following outcome:
     * 1 2 4 5
     * 2 3 5 6
     * 4 5 7 8
     * 5 6 8 9
     */
    template<typename Dtype>
    void im2col_cpu(const Dtype *data_image, const int channels, const int height, const int width,
                    const int kernel_size, const int padding, const int stride,
                    Dtype *data_column) {

        const int output_h = (height - kernel_size + 2 * padding) / stride + 1;
        const int output_w = (height - kernel_size + 2 * padding) / stride + 1;

        // remember the amount of elements inside a channel to move the pointer forward by that many addresses
        const int channel_size = height * width;
        for (int channel = channels; channel--; data_image += channel_size) {
            for (int kernel_row = 0; kernel_row < kernel_size; kernel_row++) {
                for (int kernel_col = 0; kernel_col < kernel_size; kernel_col++) {

                    int input_row = -padding + kernel_row;

                    for (int output_row = 0; output_row < output_h; ++output_row) {
                        if (!is_value_ge_zero_and_value_lt_boundary(input_row, height)) {
                            for (int output_col = 0; output_col < output_w; ++output_col) {
                                *(data_column++) = 0;
                            }
                        } else {
                            int input_col = kernel_col - padding;
                            for (int output_col = 0; output_col < output_w; ++output_col) {
                                if (is_value_ge_zero_and_value_lt_boundary(input_col, width)) {
                                    *(data_column++) = data_image[input_row * width + input_col];
                                } else {
                                    *(data_column++) = 0;
                                }
                                input_col += stride;
                            }
                        }
                        input_row += stride;
                    }

                }
            }
        }
    }

    /*
    // Explicit instantiation
    template void im2col_cpu<int>(const int *data_image, const int channels, const int height, const int width,
                                  const int kernel_size, const int padding, const int stride,
                                  int *data_column);

    template void im2col_cpu<float>(const float *data_image, const int channels, const int height, const int width,
                                    const int kernel_size, const int padding, const int stride,
                                    float *data_column);

    template void im2col_cpu<double>(const double *data_image, const int channels, const int height, const int width,
                                     const int kernel_size, const int padding, const int stride,
                                     double *data_column);
    */

    
    template<typename Dtype>
    void col2im_cpu(const Dtype *data_column, const int channels, const int height, const int width,
                    const int kernel_size, const int padding, const int stride,
                    Dtype *data_image) {

        set(height * width * channels, Dtype(0), data_image);

        const int output_h = (height - kernel_size + 2 * padding) / stride + 1;
        const int output_w = (height - kernel_size + 2 * padding) / stride + 1;

        // remember the amount of elements inside a channel to move the pointer forward by that many addresses
        const int channel_size = height * width;
        for (int channel = channels; channel--; data_image += channel_size) {
            for (int kernel_row = 0; kernel_row < kernel_size; kernel_row++) {
                for (int kernel_col = 0; kernel_col < kernel_size; kernel_col++) {

                    int input_row = -padding + kernel_row;

                    for (int output_row = 0; output_row < output_h; ++output_row) {
                        if (!is_value_ge_zero_and_value_lt_boundary(input_row, height)) {
                            data_column += output_w;
                        } else {
                            int input_col = -padding + kernel_col;

                            for (int output_col = 0; output_col < output_w; ++output_col) {
                                if (is_value_ge_zero_and_value_lt_boundary(input_col, width)) {
                                    data_image[input_row * width + input_col] += *data_column;
                                }
                                data_column++;
                                input_col += stride;
                            }
                        }
                        input_row += stride;
                    }

                }
            }
        }
    }

    /*
    // Explicit instantiation
    template void col2im_cpu<int>(const int *data_column, const int channels, const int height, const int width,
                                  const int kernel_size, const int padding, const int stride,
                                  int *data_image);

    template void col2im_cpu<float>(const float *data_column, const int channels, const int height, const int width,
                                    const int kernel_size, const int padding, const int stride,
                                    float *data_image);

    template void col2im_cpu<double>(const double *data_column, const int channels, const int height, const int width,
                                     const int kernel_size, const int padding, const int stride,
                                     double *data_image);
    */
}
