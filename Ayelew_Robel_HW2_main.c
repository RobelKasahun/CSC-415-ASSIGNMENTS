/**************************************************************
 * Class::  CSC-415-01 Summer 2024
 * Name:: Robel Ayelew
 * Student ID:: 922419937
 * GitHub-Name:: RobelKasahun
 * Project:: Assignment 2 – Stucture in Memory and Buffering
 *
 * File:: Ayelew_Robel_HW2_main.c
 *
 * Description:: This assignment has two parts. The first part is to populate
 *               the struct named personalInfo with our personal information
 *               and the second part of this assignment is to write a large
 *               byte of information into the buffer of size 256 bytes
 *
 **************************************************************/
#include <stdio.h>       // for standard input and output
#include <assignment2.h> // include assignment2.h header file
#include <stdlib.h>      // for standard library functions
#include <string.h>      // for string

const unsigned int SIZE = 100;
const unsigned int SUCCESS_CODE = 0;
const unsigned int FAILURE_CODE = -1;
const char *next_string_value;
int next_string_size = 0;
int remaining_buffer_size = BLOCK_SIZE;
int position_in_buffer = 0;
int remaining_bytes = 0;
int copy_remaining_buffer_size = 0;

int main(int argc, char *argv[])
{
    // Allocate memory for personalInfo on the heap
    personalInfo *personal_info = malloc(sizeof(personalInfo));

    // check if malloc allocate memory successfully
    if (personal_info == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for personalInfo struct.\n");
        return FAILURE_CODE;
    }

    // populate personInfo
    (*personal_info).firstName = argv[1];
    (*personal_info).lastName = argv[2];

    int total_language_knowledges = 0;
    total_language_knowledges |= KNOWLEDGE_OF_CPLUSPLUS;
    total_language_knowledges |= KNOWLEDGE_OF_HTML;
    total_language_knowledges |= KNOWLEDGE_OF_PYTHON;
    total_language_knowledges |= KNOWLEDGE_OF_JAVA;
    total_language_knowledges |= KNOWLEDGE_OF_JAVASCRIPT;
    total_language_knowledges |= KNOWLEDGE_OF_SQL;
    total_language_knowledges |= KNOWLEDGE_OF_INTEL_ASSEMBLER;
    total_language_knowledges != KNOWLEDGE_OF_C;
    (*personal_info).languages |= total_language_knowledges;
    (*personal_info).studentID = 922419937;
    (*personal_info).level = SENIOR;

    // copy the first 100 characters of the argv at index 3 into the
    // personalInfo's message property
    strncpy((*personal_info).message, argv[3], SIZE);

    // write the personal info
    int result_code = writePersonalInfo(personal_info);

    if (result_code != 0)
    {
        fprintf(stderr, "writePersonalInfo() failed to write the personal info data.\n");
        return FAILURE_CODE;
    }

    // freeing the dynamically allocated memory
    free(personal_info);

    // reserve buffer that is 256 bytes big
    char *buffer = (char *)malloc(BLOCK_SIZE);

    if (buffer == NULL)
    {
        fprintf(stderr, "Failed to allocate dynamic memory.\n");
        return FAILURE_CODE;
    }

    while ((next_string_value = getNext()) != NULL)
    {
        // current string size
        next_string_size = strlen(next_string_value);

        // fill the buffer with string value as long as the buffer is not full
        if (next_string_size <= remaining_buffer_size)
        {
            memcpy(buffer + position_in_buffer, next_string_value, next_string_size);
            /**
             * move the position to where the second string should start
             * adding string value if the buffer is not full
             */
            position_in_buffer += next_string_size;

            // subtract the number of bytes added from the actual buffer size
            remaining_buffer_size -= next_string_size;
        }
        else
        {
            /**
             * At this point the buffer is full or the current string value
             * is greater than the remaining buffer size, therefore the logic
             * here will start overwriting string value coming from the getNext()
             * starting from the very beginning position of the buffer
             */

            remaining_bytes = next_string_size - remaining_buffer_size;
            memcpy(buffer + position_in_buffer, next_string_value, remaining_buffer_size);
            copy_remaining_buffer_size = remaining_buffer_size;
            position_in_buffer += remaining_buffer_size;
            remaining_buffer_size -= remaining_buffer_size;

            // commit if the buffer is full
            if (strlen(buffer) == BLOCK_SIZE)
            {
                commitBlock(buffer);
                remaining_buffer_size = BLOCK_SIZE;
                position_in_buffer = 0;
                next_string_size = 0;
            }

            memcpy(buffer + position_in_buffer, next_string_value + copy_remaining_buffer_size,
                   remaining_bytes);
            position_in_buffer += remaining_bytes;
            remaining_buffer_size -= remaining_bytes;
        }
    }

    // commit the block if buffer has some bytes init
    if (strlen(buffer) > 0)
    {
        commitBlock(buffer);
    }

    // free the dynamically allocated memory
    free(buffer);

    const int check_it_result_code = checkIt();

    if (check_it_result_code != 0)
    {
        fprintf(stderr, "checkIt() has been failed!\n");
    }

    return SUCCESS_CODE;
}