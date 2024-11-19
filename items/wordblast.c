/**************************************************************
 * Class:: CSC-415-01 Summer 2024
 * Name:: Robel Ayelew
 * Student ID:: 922419937
 * GitHub-Name:: RobelKasahun
 * Project:: Assignment 4 – Word Blast
 *
 * File:: Ayelew_Robel_HW4_main.c
 *
 * Description:: This assignment is about reading data from the file named
 *               named WarAndPeace.txt and count and print the 10 words that
 *               are 6 or more in length and have highest word tallies or frequencies
 *               using threads.
 *
 **************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>

#define TOTAL_NUMBER_OF_WORDS 2024
#define WORD_SIZE 50

int chunk_size_in_bytes; // bytes that each thread will process
int remaining_bytes;     // will contain the last bytes
int file_descriptor;
int file_size;
int total_number_of_threads;
char *unique_words[TOTAL_NUMBER_OF_WORDS];
pthread_mutex_t lock;

typedef struct Word
{
    char *word;
    int frequency;
} Word;

struct Word array_of_words[TOTAL_NUMBER_OF_WORDS];

void *process(void *ptr);
void init();
void sort();
void print(char *args[]);

// You may find this Useful
char *delim = "\"\'.“”‘’?:;-,—*($%)! \t\n\x0A\r";

int main(int argc, char *argv[])
{
    // to exit gracefully when number of threads is greater than 26
    if (atoi(argv[2]) > 26)
    {
        fprintf(stderr, "\nPlease choose number of threads less than 26.\n");
        fprintf(stderr, "Error will occure when number of threads is greater than 26.\n\n");
        exit(EXIT_SUCCESS);
    }
    //**************************************************************
    // DO NOT CHANGE THIS BLOCK
    // Time stamp start
    struct timespec startTime;
    struct timespec endTime;

    clock_gettime(CLOCK_REALTIME, &startTime);
    //**************************************************************

    //***TO DO***  Look at arguments, open file, divide by threads
    //             Allocate and Initialize any storage structures

    /**
     * mutex must be initialized before
     * using pthread_mutex_lock() and
     * pthread_mutex_unlock()
     */
    int thread_init_result = pthread_mutex_init(&lock, NULL);
    if (thread_init_result == -1)
    {
        fprintf(stderr, "Error occurred while initializing mutex.\n");
        exit(EXIT_FAILURE);
    }
    // open the file in a read mode only
    file_descriptor = open(argv[1], O_RDONLY);
    if (file_descriptor == -1)
    {
        fprintf(stderr, "Error occurred while opening the file named %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    // get the thread counts and convert the data to an integer
    total_number_of_threads = atoi(argv[2]);
    /**
     * calculate the file size by moving the pointer
     * starting from the beginning of the file to the
     * end of the file
     */
    file_size = lseek(file_descriptor, 0, SEEK_END);
    // setting the position to the beginning of the file
    lseek(file_descriptor, 0, SEEK_SET);
    /**
     * every threads will have this many bytes of data
     * for processing
     */
    chunk_size_in_bytes = file_size / total_number_of_threads;
    // initialize the array of word structures
    init();

    // *** TO DO ***  start your thread processing
    //                wait for the threads to finish

    // for thread unique ids
    int *thread_ids[total_number_of_threads];

    // create total_number_of_threads of threads
    pthread_t threads[total_number_of_threads];
    int index = 0;
    while (index < total_number_of_threads)
    {
        /**
         * allocate dynamic memory for the current index to be used as a unique id
         * when creating threads, therefore each thread will have unique id
         */
        thread_ids[index] = malloc(sizeof(int));
        if (index == total_number_of_threads - 1)
        {
            remaining_bytes = file_size % total_number_of_threads;
        }
        *thread_ids[index] = index;
        int result = pthread_create(&threads[index], NULL, process, thread_ids[index]);
        if (result != 0)
        {
            printf("Error has been occurred while creating the threads.\n");
        }
        ++index;
    }

    // join the threads
    for (int i = 0; i < total_number_of_threads; i++)
    {
        int result = pthread_join(threads[i], NULL);
        if (result != 0)
        {
            printf("Error has been occurred while joining the threads.\n");
            return 1;
        }
    }

    // ***TO DO *** Process TOP 10 and display
    sort();
    print(argv);

    //**************************************************************
    // DO NOT CHANGE THIS BLOCK
    // Clock output
    clock_gettime(CLOCK_REALTIME, &endTime);
    time_t sec = endTime.tv_sec - startTime.tv_sec;
    long n_sec = endTime.tv_nsec - startTime.tv_nsec;
    if (endTime.tv_nsec < startTime.tv_nsec)
    {
        --sec;
        n_sec = n_sec + 1000000000L;
    }

    printf("Total Time was %ld.%09ld seconds\n", sec, n_sec);
    //**************************************************************

    // ***TO DO *** cleanup
    close(file_descriptor);
    pthread_mutex_destroy(&lock);

    // freeing the dynamically allocated memories for thread ids
    index = 0;
    while (index < total_number_of_threads)
    {
        free(thread_ids[index]);
        ++index;
    }

    // freeing the dynamically allocated memories for array_of_words
    index = 0;
    while (index < TOTAL_NUMBER_OF_WORDS)
    {
        free(array_of_words[index].word);
        ++index;
    }

} // end of main function

void *process(void *ptr)
{
    int index;
    int count = 0;
    int result;
    int thread_id = *((int *)ptr);
    char *buffer = malloc(chunk_size_in_bytes + remaining_bytes);
    /**
     * read (chunk_size_in_bytes + remaining_bytes) bytes od data
     * and store the data into the buffer for processing
     */
    long offset = thread_id * chunk_size_in_bytes;
    int pread_result_code = pread(file_descriptor, buffer, chunk_size_in_bytes + remaining_bytes, offset);
    if (pread_result_code == -1)
    {
        fprintf(stderr, "Error while reading the file descriptor.\n");
        exit(EXIT_FAILURE);
    }
    char *saveptr;
    // tokenize the current buffer using the given delimiter
    char *token = strtok_r(buffer, delim, &saveptr);

    if (token == NULL)
    {
        fprintf(stderr, "Failed to parse the current buffer\n");
        exit(EXIT_FAILURE);
    }

    while (token != NULL)
    {
        index = 0;
        if (strlen(token) >= 6)
        {
            while (index < TOTAL_NUMBER_OF_WORDS)
            {
                result = strcasecmp(token, array_of_words[index].word);
                if (result == 0)
                {
                    // beginning and ending critical sections
                    pthread_mutex_lock(&lock);
                    /**
                     * the token is already in the array of words
                     * therefore increment increment its frequency
                     */
                    ++array_of_words[index].frequency;
                    pthread_mutex_unlock(&lock);
                    break;
                }
                ++index;
            }

            if (count < TOTAL_NUMBER_OF_WORDS)
            {
                if (result != 0)
                {
                    // beginning and ending critical sections
                    pthread_mutex_lock(&lock);
                    /**
                     * the token is not in the array, thus
                     * add the token and increment its frequency
                     */
                    strcpy(array_of_words[count].word, token);
                    ++array_of_words[count].frequency;
                    pthread_mutex_unlock(&lock);
                    ++count;
                }
            }
        }

        token = strtok_r(NULL, delim, &saveptr);
    }
    // this will indcate the end of the array of structures
    // will also help us when we want to print the structures
    // no need to use index variable when we want to print
    array_of_words[count].word = NULL;
    array_of_words[count].frequency = -1;

    // freeing the dynamically allocated storage reserved for buffer
    free(buffer);
}

/**
 * initialize the array of structures with default word and frequency value
 */
void init()
{
    int index = 0;
    while (index < TOTAL_NUMBER_OF_WORDS)
    {
        array_of_words[index].word = malloc(WORD_SIZE + 1);
        if (array_of_words[index].word == NULL)
        {
            printf("array of words at index %d is NULL\n", index);
        }
        strcpy(array_of_words[index].word, "dummyy data");
        array_of_words[index].frequency = 0;
        ++index;
    }
}

/**
 * sort array of structures using frequency in descending order
 */
void sort()
{
    Word temp_word_structure;
    for (int i = 0; i < TOTAL_NUMBER_OF_WORDS; i++)
    {
        for (int j = i + 1; j < TOTAL_NUMBER_OF_WORDS; j++)
        {
            if (array_of_words[i].frequency < array_of_words[j].frequency)
            {
                temp_word_structure = array_of_words[i];
                array_of_words[i] = array_of_words[j];
                array_of_words[j] = temp_word_structure;
            }
        }
    }
}

/**
 * Print the ten words which has most tallies or frequencies
 * in descending order
 */
void print(char *args[])
{
    printf("\n\n");
    printf("Word Frequency Count on %s with %d threads\n", args[1], total_number_of_threads);
    printf("Printing top 10 words 6 characters or more.\n");

    const unsigned int COUNT = 10;
    int index = 0;
    while (index < COUNT)
    {
        printf("Number %d is %s with a count of %d\n", (index + 1), array_of_words[index].word, array_of_words[index].frequency);
        ++index;
    }
}
