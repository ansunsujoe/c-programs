#include <stdio.h>
#include <string.h>

/**
 * Dash program
 */

int main() {
    char input[30];
    char check[30];

    int sysres = 0;

    while (1) {
        printf("dash> ");
        scanf("%s", input);

        // Check if input equals exit
        if (strcmp(input, "exit") == 0) {
            printf("Exiting now...\n");
            break;  // Break out of loop
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