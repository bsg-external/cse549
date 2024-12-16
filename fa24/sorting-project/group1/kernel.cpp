#include <bsg_manycore.h>
#include <bsg_manycore_atomic.h>
#include <bsg_cuda_lite_barrier.h>
#include <math.h>
#include <atomic>
#include <algorithm>

/*Define key-value pair structure*/
typedef struct {
  uint32_t key;
  uint32_t value;
} data;

/*Warmup cache*/
void warmCache(data* X, uint32_t n){
    uint32_t myN = bsg_tiles_X * bsg_tiles_Y;
    for (uint32_t i = __bsg_id; i < myN; i += myN) {
        uint32_t value = X[i].value;
        asm volatile ("" : : "r" (value) : "memory");
    }  
}


/*Merges two sorted subarrays from arr X into arr Y*/
void mergeArr(data *X, data *Y, uint32_t left, uint32_t mid, uint32_t right) {
    uint32_t i = left, j = mid + 1, k = left;

    /*Merge the two halves into Y*/
    while (i <= mid && j <= right) {
        if (X[i].value <= X[j].value) {
            Y[k] = X[i];
            i++;
        } else {
            Y[k] = X[j];
            j++;
        }
        k++;
    }

    /*Copy remaining elements from the left half*/
    while (i <= mid) {
        Y[k] = X[i];
        i++;
        k++;
    }

    /*Copy remaining elements from the right half*/
    while (j <= right) {
        Y[k] = X[j];
        j++;
        k++;
    }
}

/*Perform a single step of MergeSort*/
void mergeSortStep(data *X, data *Y, uint32_t n, uint32_t step) {
    uint32_t myN = n / (bsg_tiles_X * bsg_tiles_Y);
    uint32_t e, idx;

    for (e = 0; e < myN; e += 1) {
        idx = __bsg_id * myN + e;

        /*If the data does not evenly distribute across tiles, exit*/
        if (idx >= n || (idx * step * 2) >= n) return;

        uint32_t left = idx * step * 2;
        uint32_t mid = (left + step - 1) > (n - 1) ? (n - 1) : (left + step - 1);
        uint32_t right = (left + 2 * step - 1) > (n - 1) ? (n - 1) : (left + 2 * step - 1);

        /* Merge subarrays from X into Y */
        mergeArr(X, Y, left, mid, right);
    }
}

/*Arguments:
  -- X: Unsorted Data
  -- Y: Unitialized Memory buffer for Sorted Array
  -- n: Number of elements in array  
*/
extern "C" __attribute__ ((noinline))
int kernel_sorter(data *X, data *Y, int n){
    /* Initialize tiles and warm up the cache */
    bsg_barrier_hw_tile_group_init();
    warmCache(X, n);
    bsg_barrier_hw_tile_group_sync();
    bsg_cuda_print_stat_kernel_start();

    int step;
    data *src = X;
    data *dest = Y;

    for (step = 1; step < n; step *= 2) {
        mergeSortStep(src, dest, n, step);
        bsg_barrier_hw_tile_group_sync();

        /*Swap source and destination arrays for the next step*/
        data *temp = src;
        src = dest;
        dest = temp;
    }

    /*Ensure sorted result is in Y if the number of steps is odd*/
    if (step / 2 % 2 != 0) {
        for (int i = 0; i < n; i++) {
            Y[i] = X[i];
        }
    }

    bsg_fence();
    bsg_cuda_print_stat_kernel_end();
    bsg_fence();
    bsg_barrier_hw_tile_group_sync();

    return 0;
}


