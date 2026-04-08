#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

/*
 * program to find the average of a set of random numbers
 * which forks off #t# threads to do the sum computation in parallel
 *
 * usage: avg-manythread count threads
 *
 * where count is the number of random values to generate
 */

 char *usage = "usage: avg-manythread count threads";

 #define RAND() (drand48()) /* basic linux random number generator */

 /*
  * data type definition for arguments passed to thread
  */
struct arg_struct
{
    int id;
    int size;
    double *data; // array of random doubles
    int starting_i;
};

/*
 * data type definition for the result passed back from the threads
 */
struct result_struct
{
    double sum;
};

void *SumThread(void *arg)
{
    int i;
    double my_sum;
    struct arg_struct *my_args;
    int my_size;
    double *my_data;
    struct result_struct *result;
    int my_start;
    int my_end;
    int my_id;

    my_args = (struct arg_struct *)arg;
    result = (struct result_struct *)malloc(sizeof(struct result_struct));
    if (result == NULL) {
        exit(1);
    }

    printf("sum thread %d running, starting at %d for %d\n",
            my_args->id,
            my_args->starting_i,
            my_args->size);
    fflush(stdout);

    /*
     * unmarshalling
     */
    my_id = my_args->id;
    my_size = my_args->size;
    my_start = my_args->starting_i;
    my_data = my_args->data;

    /*
     * free args
     */
    free(arg);

    /*
     * figure out the ending index for my subregion
     */
    my_end = my_start + my_size;

    my_sum = 0.0;
    for (i = my_start; i < my_size; i++) {
        my_sum += my_data[i];
    }

    result->sum = my_sum;

    return ((void *)result);
}

int main(int argc, char **argv)
{
    int i;
    int t;
    double sum;
    double *data;
    int count;
    int threads;
    struct arg_struct *args;
    struct result_struct *result;
    int err;
    pthread_t *thread_ids;
    int extra;
    int tsize;
    int range_size;
    int index;

    /*
     * sanity check the arguments
     */
    if (argc < 2) {
        fprintf(stderr, "must specify count and number of threads to use\n");
        fprintf(stderr, "%s\n", usage);
        exit(1);
    }

    count = atoi(argv[1]); /* count is tfirst argument */

    if (count <= 0) {
        fprintf(stderr, "invalid count %d\n", count);
        exit(1);
    }

    threads = atoi(argv[2]); /* thread count is second argument */

    if (threads <= 0) {
        fprintf(stderr, "invalid thread count %d\n", threads);
        exit(1);
    }

    /*
     * don't allow more threads than data elements
     */
     if (threads > count) {
        threads = count;
     }

     /*
      * make an array large enough to hold #count# doubles
      */
    data = (double *)malloc(sizeof(double) * count);
    if (data == NULL) {
        exit(1);
    }

    for (i = 0; i < count; i++) {
        data[i] = RAND();
    }

    /*
     * make data structure to hold thead ids so the spawning
     * thread can join with each one seperately
     */
    thread_ids = (pthread_t *)malloc(sizeof(pthread_t) * threads);
    if (thread_ids == NULL) {
        exit(1);
    }

    /*
     * fork off #threads# giving each a more or less even number
     * of values to sum from the masterlist
     *
     * each thread gets its own arg structure with the same pointer to the
     * data
     *
     * first divide the list size by the number if the threads to get an even
     * distribution of work.
     */
    
    range_size = (count / threads);
    extra = (count % threads);

    printf("main thread about to create %d sum threads\n", threads);
    fflush(stdout);
    /*
     * create one thread to sum each subregion of the data array
     * start with index 0
     */
    index = 0;
    for (t = 0; t < threads; t++) {
        /*
         * make a argument structure to use to pass the arguments to the
         * thread
         */
        args = (struct arg_struct *)malloc(sizeof(struct arg_struct));
        if (args == NULL) {
            exit(1);
        }
        /*
         * give this thread an integer id
         */
        args->id = t + 1;
        /*
         * give each thread the size of the subregion and the same pointer to the
         * data array
         */
        args->size = range_size;
        if (extra > 0) {
            args->size += 1;
            extra -= 1;
        }
        tsize = args->size; // remember how much we gave this thread
        args->data = data;
        /*
         * set the starting and ending index for the next thread
         */
        args->starting_i = index;
        printf("main thread creating sum thread %d\n", t+1);
        fflush(stdout);
        /*
         * fork this thread and remember its id in the array of thread
         * ids
         */
        err = pthread_create(&thread_ids[t], NULL, SumThread, (void *)args);
        if (err != 0) {
            fprintf(stderr, "pthread create failed for thread %d\n", t+1);
            exit(1);
        }
        printf("main thread has created sum thread %d\n", t+1);
        /*
         * set the starting index for the next thread
         */
        index += tsize; // bump forward to the next thread range
    }
    /*
     * join with each thread one at a time and
     * get its partial sum and make a global sum
     */
    sum = 0;
    for (t = 0; t < threads; t++) {
        /*
         * wait until the next thread about is finished and get the result back
         */
        printf("main threads about to join with sum thread %d\n", t+1);
        fflush(stdout);
        err = pthread_join(thread_ids[t], (void **)&result);
        if (err != 0) {
            fprintf(stderr, "pthread_join failed for thread %d\n", t+1);
            exit(1);
        }
        printf("main thread joined with sum thread %d\n", t+1);
        fflush(stdout);
        /*
         * add in the partial sum
         */
        sum += result->sum;
        /*
         * free this thread's result structure
         */
        free(result);
    }
    printf("the average over %d random numbers on (0,1) is %f\n", count, sum / (double)count);
    /*
     * free the thread id array
     */
    free(thread_ids);
    /*
     * free the data array
     */
    free(data);

    return(0);
}