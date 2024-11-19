/**************************************************************
 * Class::  CSC-415-01 Summer 2024
 * Name:: Robel Ayelew
 * Student ID:: 922419937
 * GitHub-Name:: RobelKasahun
 * Project:: Assignment 3 – Simple Shell with Pipes
 *
 * File:: Robel_Ayelew_HW3_main.c
 *
 * Description:: The aim of this assignment is to make a simple shell that runs on top of
 *               of regular command-line interpreter where the simple shell executes
 *               commands such as ls, ls -l, as well as commands that contains pipe
 *               such as cat myfile.txt | wc -l.
 *
 **************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>

#define SIZE 4
#define FAILURE_CODE -1
#define SUCCESS_CODE 0
#define BUFFER_SIZE 163
#define MAXIMUM_ARGUMENTS ((BUFFER_SIZE / 2) + 1)

// Function Prototypes
char *get_user_input();
char *to_lower(char *);
char **parse_input(char *user_input, char *delimeter);
void execute_commands(char *user_input, char *lower_cased_string, char **args);
void free_memory(char *user_input, char *lower_cased_string, char **args);
void execute_commands_with_pipe(char *user_input);

int main(int argc, char *argv[])
{
    printf("+++++++++ Welcome to our Simple Shell +++++++++\n");
    bool flag = false;
    char *user_input;
    char **args;
    int prompt_counter = 1;
    while (!flag)
    {

        if (prompt_counter == 1)
        {
            printf("%s ", "prompt$");
        }
        else
        {
            if (prompt_counter > 1)
            {
                printf("%s ", ">");
            }
        }

        user_input = get_user_input();

        bool has_pipe = false;
        for (int i = 0; i < strlen(user_input); i++)
        {
            if (user_input[i] == '|')
            {
                has_pipe = true;
                break;
            }
        }

        if (user_input[0] == '\n')
        {
            ++prompt_counter;
            free(user_input);
            continue;
        }

        char *lower_cased_string = to_lower(user_input);
        // exit the loop when the current input is 'exit'
        if (strcmp(lower_cased_string, "exit\n") == 0)
        {
            printf("+++++++++ Exiting... Thank you for using our Simple Shell +++++++++\n");
            // free the dynaically allocated memories
            free(user_input);
            free(lower_cased_string);
            break;
        }

        args = parse_input(user_input, " \t\n\r\a");

        if (has_pipe)
        {
            execute_commands_with_pipe(user_input);
        }
        else
        {
            execute_commands(user_input, lower_cased_string, args);
        }

        free_memory(user_input, lower_cased_string, args);
    }

    return 0;
}

// --- Function Definitions --- //

// get user input from the standard input
char *get_user_input()
{
    // Allocate dynamic storage for the user input of size 163
    char *buffer = malloc(BUFFER_SIZE);
    // check if memory allocation has been successful
    if (buffer == NULL)
    {
        fprintf(stderr, "Failed to allocate dynamic memory for user input.\n");
        exit(FAILURE_CODE);
    }

    // store the user input into the buffer
    char *user_input = fgets(buffer, BUFFER_SIZE, stdin);
    // check if reading from the standard input has been successful
    if (user_input == NULL)
    {
        // if failed exit
        fprintf(stderr, "%s\n", strerror(errno));
        exit(FAILURE_CODE);
    }

    // checking for empty user input
    if (strcmp(user_input, "\n") == 0)
    {
        printf("user input is required.\n");
    }

    // return the number of bytes of data of size BUFFER_SIZE
    // coming from the standard input
    return buffer;
}

// convert the given arg to a lower case arg
char *to_lower(char *arg)
{
    char *str = malloc(strlen(arg));
    if (arg == NULL || strlen(arg) == 0 || strcmp(arg, "\n") == 0)
    {
        printf("Cannot convert the given input to lower case string\n");
        exit(FAILURE_CODE);
    }

    int index = 0;
    while (index < strlen(arg))
    {
        str[index] = tolower(arg[index]);
        ++index;
    }

    return str;
}

// parse a user input string into array of substrings
char **parse_input(char *user_input, char *delimiter)
{
    char **args = malloc(MAXIMUM_ARGUMENTS);
    int index = 0;
    // checking if dynamic memory allocation is successful
    if (args == NULL)
    {
        // if memory allocation failed, exit the program
        fprintf(stderr, "Failed to allocate dynamic memory for arguments\n");
        exit(FAILURE_CODE);
    }
    // parse the user input using the given delimeter
    char *arg = strtok(user_input, delimiter);

    // parse the user input into array of a pointer to strings
    while (arg != NULL)
    {
        args[index] = arg;
        ++index;
        arg = strtok(NULL, " \t\n\r\a");
    }

    // appending NULL at the end of args that is an array of pointer to strings
    // to indicate the end of the array of pointer to srings
    args[index] = NULL;

    return args;
}

// execute the parsed commands from the args that is an array of pointer to strings
void execute_commands(char *user_input, char *lower_cased_string, char **args)
{
    int pid_status_code = 0;

    // execute the commands
    pid_t pid = fork();

    if (pid < 0)
    {
        fprintf(stderr, "Fork has been failed: %s\n", strerror(errno));
    }
    else if (pid == 0)
    {
        int execvp_result = execvp(args[0], args);
        if (execvp_result == -1)
        {
            fprintf(stderr, "%s: %s\n", args[0], strerror(errno));
            free_memory(user_input, lower_cased_string, args);
            exit(FAILURE_CODE);
        }
    }
    else
    {
        waitpid(pid, &pid_status_code, 0);
        printf("Child %d, exited with %d\n", pid, WEXITSTATUS(pid_status_code));
    }
}

void execute_commands_with_pipe(char *user_input)
{

    // parse the user input using the pipe delimeter
    char **args = parse_input(user_input, "|");
    int fd[2]; // reading and write file descriptors
    if (pipe(fd) != 0)
    {
        fprintf(stderr, "Failed to create a pipe\n");
        free(args);
        return;
    }

    static int pid1_status_code = 0;
    int result;
    pid_t pid1 = fork();

    if (pid1 < 0)
    {
        fprintf(stderr, "Fork has been failed: %s\n", strerror(errno));
    }
    else if (pid1 == 0)
    { // --- child process 1 --- //
        dup2(fd[1], 1);
        close(fd[0]);
        close(fd[1]);
        result = execvp(args[0], args);

        if (result == -1)
        {
            fprintf(stderr, "%s: %s\n", args[0], strerror(errno));
            free(args);
            exit(FAILURE_CODE);
        }
    }
    else
    {
        // parent process
        waitpid(pid1, &pid1_status_code, 0);
        printf("Child %d, exited with %d\n", pid1, WEXITSTATUS(pid1_status_code));
    }

    // --- for second child
    static int pid2_status_code = 0;
    pid_t pid2 = fork();

    if (pid2 < 0)
    {
        fprintf(stderr, "Fork has been failed: %s\n", strerror(errno));
    }
    else if (pid2 == 0)
    { // --- for the second child 2 --- //
        dup2(fd[0], 0);
        close(fd[0]);
        close(fd[1]);
        result = execvp(args[0], args);

        if (result == -1)
        {
            fprintf(stderr, "%s: %s\n", args[0], strerror(errno));
            free(args);
            exit(FAILURE_CODE);
        }
    }
    else
    {
        // parent process
        waitpid(pid2, &pid2_status_code, 0);
        printf("Child %d, exited with %d\n", pid2, WEXITSTATUS(pid2_status_code));
    }

    free(args);
}

// frees dynamically allocated memories
void free_memory(char *user_input, char *lower_cased_string, char **args)
{
    free(user_input);
    free(lower_cased_string);
    free(args);
}
