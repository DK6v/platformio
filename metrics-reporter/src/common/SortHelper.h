#pragma once

#include <stdint.h>

namespace app
{

template <typename T> inline
void swap(T &a, T  &b) {
    T tmp = a;
    a = b;
    b = tmp;
}

template <typename T>
void bsort(T *data, uint8_t size) {

    if (size > 1) {

        for (uint8_t jx = 1; jx < size; ++jx) {
            bool isSorted = true;

            for (uint8_t ix = 0; ix < size - jx; ++ix) {
                if (data[ix] > data[ix + 1]) {
                    swap(data[ix], data[ix + 1]);
                    isSorted = false;
                }
            }

            if (isSorted) {
                break;
            }
        }
    }
}

template <typename T>
void qsort(T *data, size_t size) {

    if (size > 1) {

        size_t lx = 0;
        size_t rx = size - 1;

        T pivot = data[rx / 2];

        while (true) {
            while (data[lx] < pivot) { ++lx; }
            while (data[rx] > pivot) { --rx; }

            if (lx >= rx) {
                break;
            }

            swap(data[lx++], data[rx--]);
        }

        qsort(data, rx + 1);
        qsort(data + (rx + 1), size - (rx + 1));
    }
}

};