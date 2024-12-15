#include <bsg_manycore.h>
#include <bsg_cuda_lite_barrier.h>
#include <math.h>
#include "bsg_manycore_atomic.h"
#include "bsg_cuda_lite_barrier.h"
#include <atomic>
#include <algorithm>
typedef struct {
    uint32_t key;
    uint32_t value;
} ValueKeyPair;
#define BITS_PER_PASS 8  // Sort by 4 bits at a time
#define NUM_BUCKETS (1 << BITS_PER_PASS)  // 16 buckets for 4 bits
#define MAX_BITS 32  // Sorting 32-bit integers
// Local histogram
__attribute__((section(".dram"))) int local_count[bsg_tiles_X * bsg_tiles_Y][NUM_BUCKETS];
__attribute__((section(".dram"))) int global_prefix_sum [NUM_BUCKETS];
void radixSort(ValueKeyPair *d_array, ValueKeyPair *d_buffer, int size, int bit_offset) {
    int tid = __bsg_id;
    int block_size = bsg_tiles_X * bsg_tiles_Y;
    int data_per_core = size / block_size;
    int start = tid * data_per_core;
    int end = start + data_per_core;
    register int local_count_reg[NUM_BUCKETS] = {0};
    register int local_prefix_sum_reg[NUM_BUCKETS]= {0};
    register int local_prefix_sum_buffer_reg[NUM_BUCKETS]= {0};
    register int global_prefix_sum_reg[NUM_BUCKETS];
    // Initialize local count
    for (int bucket = 0; bucket < NUM_BUCKETS; bucket++)
    {
        local_count[tid][bucket] = 0;
    }
    bsg_barrier_hw_tile_group_sync();
    for (int i = __bsg_id; i < block_size; i += block_size)
    {
        for (int j = start; j < end; j++)
        {
            uint32_t value = d_array[j].value;
            int bucket = (value>> bit_offset) & (NUM_BUCKETS - 1);
            local_count_reg[bucket]++;
        }
    }
    bsg_barrier_hw_tile_group_sync();
        // Write local counts to global memory
    for (int i = __bsg_id; i < block_size; i += block_size)
    {
        for (int j= 0; j<NUM_BUCKETS; j++)
        {
            // prefix_sum[i][i]=0;
            local_count[i][j] = local_count_reg[j];
        }
    }
    bsg_barrier_hw_tile_group_sync();
    for (int i = __bsg_id; i < block_size; i += block_size)
    {
        for (int j = 0; j < tid; j++)
        {
            for (int k = 0; k < NUM_BUCKETS; k++)
            {
                local_prefix_sum_reg[k] +=local_count[j][k] ;
            }
        }
    }
    bsg_barrier_hw_tile_group_sync();
    if (__bsg_id == block_size-1)
    {
        global_prefix_sum[0]=0;
        int shift = 0;
        for (int i = 0; i < NUM_BUCKETS; i++)
        {
            shift += local_prefix_sum_reg[i]+local_count_reg[i];
            global_prefix_sum[i+1]=shift;
        }
    }
    bsg_barrier_hw_tile_group_sync();
    for (int i = __bsg_id; i < block_size; i += block_size)
    {
        for (int j = 0; j < NUM_BUCKETS; j++)
        {
            global_prefix_sum_reg[j]=global_prefix_sum[j];
        }
    }
    bsg_barrier_hw_tile_group_sync();
        for (int i = __bsg_id; i < block_size; i += block_size)
    {
        for (int j = start; j < end; j++)
        {
            uint32_t value = d_array[j].value;
            int bucket = (value>> bit_offset) & (NUM_BUCKETS - 1);
            int pos = local_prefix_sum_buffer_reg[bucket] + local_prefix_sum_reg[bucket] + global_prefix_sum_reg[bucket];
            d_buffer[pos] = d_array[j];
            local_prefix_sum_buffer_reg[bucket]++;
        }
    }
    bsg_barrier_hw_tile_group_sync();
    for (int i = __bsg_id; i < size; i += block_size) {
        d_array[i] = d_buffer[i];
    }
    bsg_barrier_hw_tile_group_sync();
}
extern "C" __attribute__ ((noinline))
int kernel_sort_radix(ValueKeyPair * Unsorted, ValueKeyPair * Sorted,int N) {
  bsg_barrier_hw_tile_group_init();
  bsg_barrier_hw_tile_group_sync();
  bsg_cuda_print_stat_kernel_start();
  for (int bit = 0; bit < 32; bit += BITS_PER_PASS) {
      radixSort(Unsorted, Sorted,N, bit );
      bsg_barrier_hw_tile_group_sync();
  }
  bsg_fence();
  bsg_cuda_print_stat_kernel_end();
  bsg_fence();
  bsg_barrier_hw_tile_group_sync();
  return 0;
}