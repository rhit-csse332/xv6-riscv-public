// NAME: Ben Graham

#include <stdio.h> // OH NO
#include <stdlib.h> // OH NO
#include <sys/time.h> // OH NO
#include <time.h> // OH NO
#include "pthread.h"

int verbose = 0;   // Whether to print array before and after sorting
int nthreads = 2;  // Default number of threads is 2

/**
 * Helper function to report timing information.
 */
void output_time_difference(char* name, struct timeval* start,
                            struct timeval* end) {
  long secs_used =
      (end->tv_sec - start->tv_sec);  // avoid overflow by subtracting first
  long usecs_used = (end->tv_usec - start->tv_usec);
  double secs = secs_used + (double)usecs_used / 1000000;
  printf("%s took %f seconds\n", name, secs);
}

/**
 * Helper function to check whether the array is sorted.
 */
int check_sorted(int* arr, int len) {
  for (int i = 1; i < len; i++) {
    if (arr[i-1] > arr[i]) {
      return 0;
    }
  }
  return 1;
}

/**
 * Helper function to print out the array.
 */
void print_array(int* arr, int len) {
  for (int i = 0; i < len; i++) {
    printf("%d ", arr[i]);
  }
}

/**
 * Allocates and returns a new array with the same contents.
 * 
 * @param arr the array to copy
 * @param len the length of the array
 * @return int* : the new array. Should later be freed.
 */
int* copy_array(int* arr, int len) {
  int* cparr = malloc(sizeof(int) * len);
  for (int i = 0; i < len; i++) {
    cparr[i] = arr[i];
  }
  return cparr;
}

/**
 * Fills an array with random data.
 * 
 * @param arr the array to fill
 * @param len the length of the array
 * @param upperbd an upper bound on the random values to fill the array
 */
void fillWithRandom(int* arr, int len, int upperbd) {
  for (int i = 0; i < len; i++) {
    arr[i] = rand() % upperbd;
  }
}

/**
 * Merge: The main "workhorse" operation of the mergesort algorithm.
 * Assumes that the inputs satisfy the following:
 *      - start < mid < end
 *      - The "left half" arr[start], arr[start+1], ..., arr[mid] is sorted
 *      - The "right half" arr[mid+1], arr[mid+2], ..., arr[end] is sorted
 * The merge operation merges "left half" and "right half" so that
 *      - arr[start], ..., arr[end] is sorted
 * 
 * @param arr the array to merge
 * @param arr2 an auxiliary array (used as temporary storage)
 * @param start, @param mid, @param end - array indices
 */
void merge(int* arr, int* arr2, int start, int mid, int end) {
  int i = start;     // start of left half within arr
  int j = mid + 1;   // start of right half within arr
  int k = start;     // start of where to copy to in arr2
  // Merge left half and right half, copying into auxiliary array arr2
  while (i <= mid && j <= end) {
    if (arr[i] < arr[j]) {
      arr2[k++] = arr[i++];
    } else {
      arr2[k++] = arr[j++];
    }
  }
  // Either left or right half is done. Finish copying the other half.
  while (i <= mid) {
    arr2[k++] = arr[i++];
  }
  while (j <= end) {
    arr2[k++] = arr[j++];
  }
  // Copy everything from arr2 back to arr1.
  for (i = start; i <= end; i++) {
    arr[i] = arr2[i];
  }
}

/**
 * Mergesort: a recursive function to sort a part of an array based on 
 * the mergesort algorithm. Sorts the part from index start to index end.
 * 
 * @param arr the array to mergesort
 * @param arr2 an auxiliary array (used as temporary storage)
 * @param start the starting index of the part of arr to sort
 * @param end the ending index of the part of arr to sort
 */
void mergesort(int* arr, int* arr2, int start, int end) {
  // Base case: part is length 0 or 1. No work to do.
  if (start >= end) {
    return;
  }
  int mid = (start + end) / 2;        // Compute middle index
  mergesort(arr, arr2, start, mid);   // Recursively sort left half
  mergesort(arr, arr2, mid + 1, end); // Recursively sort right half
  merge(arr, arr2, start, mid, end);  // Merge left and right halves
}

/**
 * Header function for the mergesort algorithm.
 * Creates the auxiliary array and starts recursive mergesort on the entire
 * array.
 * 
 * @param arr the array to mergesort
 * @param len the length of the array
 */
void mergesort_init(int* arr, int len) {
  int* arr2 = (int*) malloc(sizeof(int) * len); // auxiliary array
  mergesort(arr, arr2, 0, len - 1);
  free(arr2); // free auxiliary array
}
			 

// TODO: add any helper methods here. You will also likely want to define
// a struct that allows you to pass multiple inputs to the function your
// pthreads are called on.

struct mergeinfo {
  int* arr;
  int* arr2;
  int start;
  int end;
  int threads;
};
void* mergesort_thread(void* v_info) {
  struct mergeinfo* info = (struct mergeinfo*) v_info;
  if(info->threads <= 1) {
    mergesort(info->arr, info->arr2, info->start, info->end); // if 1 thread available, proceed as normal
  } else {
    int mid = (info->start+info->end)/2;

    struct mergeinfo lefthalf = *info; // build info structs for each half of the merge
    struct mergeinfo righthalf = *info;
    lefthalf.end = mid;
    righthalf.start = mid+1;
    lefthalf.threads = info->threads/2;
    righthalf.threads = (info->threads+1)/2;
    
    pthread_t partner;
    pthread_create(&partner, NULL, mergesort_thread, (void*) &righthalf); // start the right half in a new thread, giving it half our available threads
    mergesort_thread(&lefthalf); // process the left half here with half the thread allocation
    pthread_join(partner, NULL); // wait for the right half to finish
    merge(info->arr, info->arr2, info->start, mid, info->end); // merge as normal
  }
}


/**
 * Header function for the mergesort algorithm.
 * Creates the auxiliary array and starts recursive mergesort on the entire
 * array.
 * 
 * @param arr the array to mergesort
 * @param len the length of the array
 */
void mergesort_pthread_init(int* arr, int len) {
  int* arr2 = (int*) malloc(sizeof(int) * len); // auxiliary array
  
  // TODO: modify this method to use more than one thread.
  struct mergeinfo initial;
  initial.arr = arr;
  initial.arr2 = arr2;
  initial.start = 0;
  initial.end = len-1;
  initial.threads = nthreads;

  mergesort_thread(&initial); // all we really do here is call the helper function
  free(arr2); // free auxiliary array
}

/**
 * For testing sorting-algorithm code.
 * 
 * @param sort - the sorting algorithm to use, e.g., mergesort_init
 * @param arr - the array to sort
 * @param len - the length of the array
 * @param name - a name for this test
 */
void run_test(void (*sort)(int*, int), int* arr, int len, char* name) {
  struct timeval startTime, endTime;
  if (verbose) {                           // print out input array
    printf("Starting array: \n");
    print_array(arr, len);
    printf("\n");
  }
  gettimeofday(&startTime, NULL);
  sort(arr, len);                          // run the sort
  gettimeofday(&endTime, NULL);
  int wasSorted = check_sorted(arr, len);  // make sure it worked
  if (wasSorted) {
    printf("Sort successful!\n");
  } else {
    printf("Sort did not work.\n");
  }
  if (verbose) {                           // print out sorted array
    printf("Sorted array: \n");
    print_array(arr, len);
    printf("\n");
  }
  output_time_difference(name, &startTime, &endTime); // report timing
}

int main(int argc, char** argv) {
  if (argc < 2) {
    printf("Usage: %s len [verbose] [nthreads]\n", argv[0]);
    return -1;
  }

  // The second argument can be used to turn on/off verbosity.
  // 1 means print the array before & after sort, 0 suppresses this.
  if (argc >= 3) {
    verbose = atoi(argv[2]);
  }

  // The third argument can be used to change the number of threads.
  if (argc >= 4) {
    nthreads = atoi(argv[3]);
  }
  
  int len = atoi(argv[1]);
  int* arr = (int*) malloc(len * sizeof(int)); // array to sort
  fillWithRandom(arr, len, 10*len); // fill the array with some random data
  int* arr_serial = copy_array(arr, len);   // make a copy for each test
  int* arr_threaded = copy_array(arr, len);
  
  // Run tests!
  run_test(mergesort_init, arr_serial, len, "serial");
  run_test(mergesort_pthread_init, arr_threaded, len, "threaded");

  free(arr);
  free(arr_serial);
  free(arr_threaded);
  return 0;
}
