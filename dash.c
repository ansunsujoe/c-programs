#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

// Forward declaration of functions
void convertToArray(char input[], char *array);

void execute_command(char *args[]);
void execute_command_substring(char *args[], int startIndex, int endIndex);

bool isRedirecting(char *args[]);
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

        // Execute the command
        execute_command(arguments);

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
void execute_command(char *args[]) {
    // Dummy function - just for testing
    printf("Executing command %s...\n", args[0]);
}

/**
 * Executes a command using a substring of arguments
 * This is needed for redirection and parallel commands.
 */
void execute_command_substring(char *args[], int startIndex, int endIndex) {

    // Create a new array with only the args we want
    char *shortenedArgs[endIndex - startIndex + 1];

    // Fill up the array
    for (int i = startIndex; i < endIndex; i++) {
        shortenedArgs[i - startIndex] = args[i];
    }

    // Execute based on the new arguments array
    execute_command(shortenedArgs);
}

/**
 * Identifies if an array of characters contains redirection
 */
bool isRedirecting(char *args[]) {
    int arrayLen = strlen(*args) - 1;
    
    // Redirection symbol should be second to last in the array
    if (arrayLen > 2) {
    	if (strcmp(args[arrayLen - 2], ">") == 0) {
    		return true;
    	}
    	else {
			return false;    	
    	}
    }
    return false;
}

/**
 * Perform the redirect to a file
 */
void redirect(char *filename) {

}