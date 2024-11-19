/**************************************************************
 * Class::  CSC-415-01 Summer 2024
 * Name:: Robel Ayelew
 * Student ID:: 922419937
 * GitHub-Name:: RobelKasahun
 * Project:: Assignment 1 – Command Line Arguments
 *
 * File:: Ayelew_Robel_HW1_main.c
 *
 * Description:: Assignment #1 is to list or print all arguments from the command line
 *               and list them on the console pane one argument on each line
 *
 **************************************************************/
#include<stdio.h>       // for standard input and output

int main(int argc, char* argv[]){
    /**
     * Iterate over the argv that is an array of string pointers
     * to print each argument
     */
    printf("\nThere were %d arguments on the command line.\n", argc);
    for (int i = 0; i < argc; i++){
        printf("Argument %02d:   %s\n", i, argv[i]);
    }
        return 0;
}