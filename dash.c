#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>


// Forward declaration of functions
bool execute_command_substring(char *args[], char *possiblePaths[], int startIndex, int endIndex);
bool isRedirecting(char *args[], int startIndex, int endIndex);

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

    // Dealing with parallel commands
    int parallelSplits[10];
    int splitCount;
    
    // Dealing with redirection
    int redirectOut;
    char* redirectFilename;
    int saveOut;

    // Iterators
    int i;
    int startExec;
    int pathIter;

    // Variables for forking and waiting
    int rc;
    int status;
    int pid;
    int numForkedProcesses = 0;
	int pids[10];
    
    // Error message
    char error_message[30] = "An error has occurred\n";
    
    // Path variable(s)
    char *possiblePaths[10];
    possiblePaths[0] = "/bin";
    possiblePaths[1] = NULL;
	
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
    char *input = NULL;
    size_t input_size = 100;
    char *arguments[20];

    while (1) {
    
    	// Some initializations
    	numForkedProcesses = 0;

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

		// Make sure there actually is input (this will catch empty string)
		if (arguments[0] == NULL) {
			write(STDERR_FILENO, error_message, strlen(error_message));
			continue;
		}
        
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
			
			// BUILT IN COMMAND: Exit
			if (strcmp(arguments[startExec], "exit") == 0) {
				
				// Check if there is a correct number of args (only 1)
				if ((parallelSplits[i] - startExec) == 1) {
					printf("Exiting\n");
					exit(0);
				}
				else {
					write(STDERR_FILENO, error_message, strlen(error_message));
					exit(1);
				}
			}
			
			// BUILT IN COMMAND: Cd
			else if (strcmp(arguments[startExec], "cd") == 0) {
			
				// Check if there is a correct number of args (only 2)
				if ((parallelSplits[i] - startExec) == 2) {
				
					// Print error if the change of directory cannot be done
					if (chdir(arguments[startExec + 1]) != 0) {
						write(STDERR_FILENO, error_message, strlen(error_message));
						exit(1);
					}
				}
				else {
					write(STDERR_FILENO, error_message, strlen(error_message));
					exit(1);
				}
				
				// Anticipate next command
				startExec = parallelSplits[i] + 1;
				
			}
			
			// BUILT IN COMMAND: Path
			else if (strcmp(arguments[startExec], "path") == 0) {
			
				printf("Executing path and size is %lu\n", sizeof(possiblePaths));
				
				// Empty the possible paths array
				for (pathIter = 0; pathIter < 10; pathIter++) {
					possiblePaths[pathIter] = NULL;
				}
				
				// Add our new arguments (if there are any)
				for (pathIter = 0; pathIter < parallelSplits[i] - startExec - 1; pathIter++) {
					possiblePaths[pathIter] = arguments[pathIter + startExec + 1];
				}
				
				// Anticipate next command
				startExec = parallelSplits[i] + 1;
				
			}
			
			else {
				// Increase the forked processes tally
				numForkedProcesses++;
			
				// Fork another process
				pids[numForkedProcesses - 1] = fork();

				// Fork failed
				if (pids[numForkedProcesses - 1] < 0) {
					fprintf(stderr, "Fork failed\n");
					exit(1);
				}
				// This is the child and the exec happens here
				else if (pids[numForkedProcesses - 1] == 0) {

					// Check for redirection
					if (isRedirecting(arguments, startExec, parallelSplits[i])) {

						// Find the name of the file to redirect to
					    redirectFilename = arguments[parallelSplits[i] - 1];
	                    printf("Writing to file %s...\n", redirectFilename);

						// Open that file
	                    redirectOut = open(redirectFilename, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
	                    saveOut = dup(fileno(stdout));
	                    
	                    // Redirect output to that file or error
	                    if (dup2(redirectOut, fileno(stdout)) == -1) {
	                    	write(STDERR_FILENO, error_message, strlen(error_message));
	                    }
	                    
	                    // Execute the rest of the command
	                    if (execute_command_substring(arguments, possiblePaths, startExec, parallelSplits[i] - 2) == false) {
							write(STDERR_FILENO, error_message, strlen(error_message));
						}
						
						// Make sure everything in the execution is printed to the file
						fflush(stdout);
						close(redirectOut);
						
						// Switch control back to stdout
						dup2(saveOut, fileno(stdout));
						close(saveOut);
						
	                }
	                
	                // Not redirection
	                else {
	                	// Execute the command or return an error if the executable file cannot be found
						if (execute_command_substring(arguments, possiblePaths, startExec, parallelSplits[i]) == false) {
							write(STDERR_FILENO, error_message, strlen(error_message));
						}
	                }
					
				}

				else {
					printf("PID of the Child: %d\n", pids[numForkedProcesses - 1]);
				}
				
				// Anticipate the next instruction
				startExec = parallelSplits[i] + 1;
			}
			
		}
        
        // Wait until the child processes have finished
        for (i = 0; i < numForkedProcesses; i++) {
            pid = waitpid(pids[i], &status, 0);
            printf("Child with PID %d exited with status %d\n", (int) pid, status);
        }

        printf("------------------\n");
    }
}

/**
 * Executes a command using a substring of arguments
 * This is needed for redirection and parallel commands.
 * Return whether the command could be executed via the executable in path.
 */
bool execute_command_substring(char *args[], char *possiblePaths[], int startIndex, int endIndex) {

	// Identify whether the executable is found
	bool execFound = false;

	// Identify the path
	char currentPath[50];

    // Create a new array with only the args we want
    char *shortenedArgs[endIndex - startIndex + 1];

    // Fill up the array
    int i = 0;
    for (i = startIndex; i < endIndex; i++) {
        shortenedArgs[i - startIndex] = args[i];
    }
    shortenedArgs[endIndex - startIndex] = NULL;
    
    // Check if the paths can be accessed
    for (i = 0; possiblePaths[i] != NULL; i++) {
    
    	sprintf(currentPath, "%s/%s", possiblePaths[i], shortenedArgs[0]);
    	
    	// Try to access the path
    	if (access(currentPath, X_OK) != -1) {
    	
    		// Execute based on the new arguments array
    		printf("Executing command %s <-> %s...\n", shortenedArgs[0], shortenedArgs[endIndex-startIndex-1]);
    		execv(currentPath, shortenedArgs);
    		execFound = true;
    		break;
    	}
    	else {
    		printf("Couldn't find %s\n", currentPath);
    	}
    }
    
    // Return the status of if we could find the executable
    return execFound;

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