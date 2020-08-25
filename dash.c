#include <stdio.h>
#include <string.h>
#include <stdbool.h>

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

        // Convert to array of arguments
        *arguments = convertToArray(input);

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
 * Convert the string we received into an array of arguments
 */
char* convertToArray(char *input) {
    char *arguments[10];
    char *name = strtok(input, " ");
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