#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>


// Forward declaration of functions
void convertToArray(char input[], char *array);

void execute_command(char *args[]);
void execute_command_substring(char *args[], int startIndex, int endIndex);

bool isRedirecting(char *args[], int startIndex, int endIndex);
void redirect(char *args[]);


/**
 * Dash program
 */

int main(int argc, char *argv[]) {
	
	// Start out by checking if we are running in batch mode or interactive
	// Invalid if there are two arguments or more with ./dash
    if (argc > 2) {
        fprintf(stderr, "Too many arguments\n");
        exit(1);
    }
    
    // Batch mode if there is one argument
    else if (argc == 2) {
    	printf("Running in batch mode\n");
    }
    
    // Interactive mode if there are 0 arguments
    else {
    	printf("Running in interactive mode\n");
    }

    // Variables for reading the command
    char *input;
    size_t input_size = 100;
    char *arguments[20];

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
        
        // Find the array length
        int argsLength = i - 1;
        
        // Find where to split parallel commands (&)
        int parallelSplits[10];
		int splitCount = 0;
	
		// Iterate through arguments in search of &
		for (i = 0; i < argsLength; i++) {
			if (strcmp(arguments[i], "&") == 0) {
		
				// Add the index of the split to the list
				parallelSplits[splitCount] = i;
				splitCount++;
			}
			
		}
		
		// Add the last index of the split
		parallelSplits[splitCount] = argsLength;
		splitCount++;
		
		// Begin execution
		int startExec = 0;
		for (i = 0; i < splitCount; i++) {
		
			// Here is where we will need to check for redirection
			// Also where to do the FORK
			execute_command_substring(arguments, startExec, parallelSplits[i]);
			startExec = parallelSplits[i] + 1;
		}     

    }
}

/**
 * Executes a command using execv (the fork part will be done in the main method)
 */
void execute_command(char *args[]) {
    // Just print for now, but will be replaced by execv
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
 * FUNCTION NEEDS TO BE FIXED FOR STARTING/ENDING INDEX
 */
bool isRedirecting(char *args[], int startIndex, int endIndex) {
	
	// Finding the length of the array
    int arrayLen = strlen(*args) + 1;
    
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
void redirect(char *args[]) {
	
}