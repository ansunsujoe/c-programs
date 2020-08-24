#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main() {
    // Will print once
    printf("Hello before fork\n");

    fork();

    // Will print twice
    printf("Hello after fork\n");
}