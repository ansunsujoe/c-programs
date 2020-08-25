#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// Forward declaration of functions
void convertToArray(char input[], char *array);
void execute_command();
bool isRedirecting();
void redirect(char *filename);

/**
 * Dash program
 */

int main() {
    // Variables for reading the command
    char *input;
    size_t input_size = 100;
    char *arguments[10];

    while (1) {
        printf("dash> ");

        // Get the input using getline
        getline(&input, &input_size, stdin);
        arguments[0] = strtok(input, " ");
        
        // Convert to array of arguments
        int i = 1;
        while (arguments[i-1] != NULL) {
            printf("Argument %d is %s\n", i - 1, arguments[i-1]);
            arguments[i] = strtok(NULL, " ");
            i++;
        }

        // Check if input equals exit
        if (strcmp(input, "exit") == 0) {
            printf("Exiting now...\n");
            exit(0);
        }

        // Check if input equals ls
        else if (strcmp(input, "ls") == 0) {
            system("ls");
        }

        // If invalid, print that an invalid command was typed and contine
        else {
            printf("Command %s is not recognized\n", input);
        }
    }
}

/**
 * Executes a command using execv and fork
 */
void execute_command() {

}

/**
 * Identifies if an array of characters contains redirection
 */
bool isRedirecting() {
    return true;
}

/**
 * Perform the redirect to a file
 */
void redirect(char *filename) {

}