#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>


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
	
	// Control value for batch mode selection
	bool batchMode = false;

    // Reading from a batch file
    FILE *fp;
    int getlineStatus;

    // Formatting our input string
    char *newlinecheck;
    int argsLength;

    // Dealing with redirection/parallel commands
    int parallelSplits[10];
    int splitCount;
    int redirectFd;
    char* redirectFilename;

    // Iterators
    int i;
    int startExec;

    // Variables for forking and waiting
    int rc;
    int wc;
    int status;
    pid_t pid;
	
	// Start out by checking if we are running in batch mode or interactive
	// Invalid if there are two arguments or more with ./dash
    if (argc > 2) {
        fprintf(stderr, "Too many arguments\n");
        exit(1);
    }
    
    // Batch mode if there is one argument
    else if (argc == 2) {
    
    	// Set to batch mode
    	printf("Running in batch mode\n");
    	batchMode = true;
    	
    	// Open the file for reading
    	fp = fopen(argv[1], "r");
    	
    	// Check if file exists
    	if (!fp) {
    		fprintf(stderr, "File %s does not exist!\n", argv[1]);
    		exit(1);
    	}
    	else {
    		printf("Reading from file %s...\n", argv[1]);
    	}
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

        // Get the input in batch mode
        if (batchMode) {
            getlineStatus = getline(&input, &input_size, fp);

            // Check if there is no more to read
            if (getlineStatus <= 0) {
                printf("Finished reading from file.\n");
                exit(0);
            }
        }

        // Get input in interactive mode
        else {
            printf("dash> ");
            getline(&input, &input_size, stdin);
        }

        // Get rid of the new line at the end of a command
        newlinecheck = strchr(input, '\n');
        if (newlinecheck) {
            *newlinecheck = 0;
        }

        arguments[0] = strtok(input, " ");
        
        // Convert to array of arguments
        i = 1;
        while (arguments[i-1] != NULL) {
            printf("Argument %d is %s\n", i - 1, arguments[i-1]);
            arguments[i] = strtok(NULL, " ");
            i++;
        }
        
        // Find the array length
        argsLength = i - 1;
        
        // Find where to split parallel commands (&)
		splitCount = 0;
	
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
		startExec = 0;
		for (i = 0; i < splitCount; i++) {

            // Fork another process
            rc = fork();

            // Fork failed
            if (rc < 0) {
                fprintf(stderr, "Fork failed\n");
                exit(1);
            }
            // This is the child and the exec happens here
            else if (rc == 0) {

                // // Check for redirection
                // if (isRedirecting(arguments, startExec, parallelSplits[i])) {

                //     // Find the name of the file to redirect to
                //     redirectFilename = arguments[parallelSplits[i] - 1];
                //     printf("Writing to file %s...\n", redirectFilename);

                //     // Open that file in place of stdout
                //     close(STDOUT_FILENO);
                //     open(redirectFilename, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
                // }

                // Execute the command
                execute_command_substring(arguments, startExec, parallelSplits[i]);
			    startExec = parallelSplits[i] + 1;
			    exit(0);
            }
			
		}
        
        // Wait until the child processes have finished
        for (i = 0; i < splitCount; i++) {
            pid = wait(&status);
            printf("Child with PID %d exited with status %d\n", (int) pid, status);
        }

        printf("------------------\n");
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