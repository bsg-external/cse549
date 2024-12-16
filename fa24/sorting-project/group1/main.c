#include <bsg_manycore_tile.h>
#include <bsg_manycore_errno.h>
#include <bsg_manycore_tile.h>
#include <bsg_manycore_loader.h>
#include <bsg_manycore_cuda.h>
#include <math.h>
#include <complex.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <bsg_manycore_regression.h>

#define ALLOC_NAME "default_allocator"

#ifndef SIZE
Please define SIZE in Makefile.
#endif

/*Define key-value pair structure*/
typedef struct {
  int key;
  int value;
} data;

/*Insertion sort algorithm to verify correctness on Host*/
void insertionSort(data *A, int n){
    int i, j;
    for (i = 1; i < n; i++){
        data key = A[i];

        /*Move key to correct position by shifting all previous entries that are greater than key one index ahead*/
        for(j = i-1; j >= 0 && A[j].value > key.value; j--){
            A[j+1] = A[j];
        }
        A[j+1] = key;
    }

  return;
}

int kernel_sorter(int argc, char **argv) {

  int rc;
  char *bin_path, *test_name;
  struct arguments_path args = {NULL, NULL};
  
  argp_parse(&argp_path, argc, argv, 0, 0, &args);
  bin_path = args.path;
  test_name = args.name;

  bsg_pr_test_info("Running kernel_sorter.\n");
  srand(time(NULL));
 
  // Initialize Device.
  hb_mc_device_t device;
  BSG_CUDA_CALL(hb_mc_device_init(&device, test_name, 0));

  hb_mc_pod_id_t pod;
  hb_mc_device_foreach_pod_id(&device, pod)
  {
    bsg_pr_info("Loading program for pod %d\n.", pod);
    BSG_CUDA_CALL(hb_mc_device_set_default_pod(&device, pod));
    BSG_CUDA_CALL(hb_mc_device_program_init(&device, bin_path, ALLOC_NAME, 0));

    // Allocate a block of memory in host; kernel is passed A_host and A_sorted_host
    data *A_host = (data*) malloc(sizeof(data)*SIZE);
    data *A_sorted_host = (data*) malloc(sizeof(data)*SIZE);
    data *A_true_sorted_host = (data*) malloc(sizeof(data)*SIZE);

    //Initialize arrays with random values
    for (int i = 0; i < SIZE; i++) {
      A_host[i].value = rand();
      A_host[i].key = i;
      A_true_sorted_host[i].value = A_host[i].value;
      A_true_sorted_host[i].key = i;     
      
    }

    insertionSort(A_true_sorted_host, SIZE);

//     // Make it pod-cache aligned
// #define POD_CACHE_ALIGNED
// #ifdef POD_CACHE_ALIGNED
//     eva_t temp_device1, temp_device2;
//     BSG_CUDA_CALL(hb_mc_device_malloc(&device, CACHE_LINE_WORDS*sizeof(int), &temp_device1));
//     printf("temp Addr: %x\n", temp_device1);
//     int align_size = (32)-1-((temp_device1>>2)%(CACHE_LINE_WORDS*32)/CACHE_LINE_WORDS);
//     BSG_CUDA_CALL(hb_mc_device_malloc(&device, align_size*sizeof(int)*CACHE_LINE_WORDS, &temp_device2));
// #endif


    // Allocate a block of memory in device.
    eva_t A_device;
    eva_t A_sorted_device;

    BSG_CUDA_CALL(hb_mc_device_malloc(&device, SIZE * sizeof(data), &A_device));
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, SIZE * sizeof(data), &A_sorted_device));

    printf("A_device Addr: %x\n", A_device);
 
    // DMA Transfer to device.
    hb_mc_dma_htod_t htod_job [] = {
      {
        .d_addr = A_device,
        .h_addr = (void *) &A_host[0],
        .size = SIZE * sizeof(data)
      }
    };

    BSG_CUDA_CALL(hb_mc_device_dma_to_device(&device, htod_job, 1));


    // CUDA arguments
    hb_mc_dimension_t tg_dim = { .x = bsg_tiles_X, .y = bsg_tiles_Y};
    hb_mc_dimension_t grid_dim = { .x = 1, .y = 1};
    #define CUDA_ARGC 4
    uint32_t cuda_argv[CUDA_ARGC] = {A_device, A_sorted_device, SIZE};
    
    // Enqueue Kernel.
    BSG_CUDA_CALL(hb_mc_kernel_enqueue (&device, grid_dim, tg_dim, "kernel_sorter", CUDA_ARGC, cuda_argv));
    
    // Launch kernel.
    hb_mc_manycore_trace_enable((&device)->mc);
    BSG_CUDA_CALL(hb_mc_device_tile_groups_execute(&device));
    hb_mc_manycore_trace_disable((&device)->mc);


    // Copy result and validate.
    hb_mc_dma_dtoh_t dtoh_job [] = {
      {
        .d_addr = A_sorted_device,
        .h_addr = (void *) &A_sorted_host[0],
        .size = SIZE * sizeof(data)
      }
    };

    BSG_CUDA_CALL(hb_mc_device_dma_to_host(&device, &dtoh_job, 1));

    // Validate
    for (int i = 0; i < SIZE; i++) {
      if (A_true_sorted_host[i].value != A_sorted_host[i].value) {
        printf("FAIL A_sorted_host[%d] = %x\n", i, A_sorted_host[i].value);
        printf("FAIL A_true_sorted_host[%d] = %x\n", i, A_true_sorted_host[i].value);
        printf("FAIL A_sorted_host = %x\n", A_sorted_host);
        for (int i = 0; i < SIZE; i++) {
          printf("%d ", A_sorted_host[i].key);
        }
        printf("\n");
        printf("FAIL A_true_sorted_host = %x\n", A_true_sorted_host);
        for (int i = 0; i < SIZE; i++) {
          printf("%d ", A_true_sorted_host[i].key);
        }
        printf("\n");

        printf("FAIL A_sorted_host Values \n");
        for (int i = 0; i < SIZE; i++) {
          printf("%d ",A_sorted_host[i].value);
        }
        printf("\n");
        printf("FAIL A_true_sorted_host Values \n");
        for (int i = 0; i < SIZE; i++) {
          printf("%d ", A_true_sorted_host[i].value);
        }
        printf("\n");
        BSG_CUDA_CALL(hb_mc_device_finish(&device));
        return HB_MC_FAIL;
      }
    }

    // Freeze tiles.
    BSG_CUDA_CALL(hb_mc_device_program_finish(&device));
  }

  BSG_CUDA_CALL(hb_mc_device_finish(&device));
  return HB_MC_SUCCESS; 
}


declare_program_main("sorter", kernel_sorter);
