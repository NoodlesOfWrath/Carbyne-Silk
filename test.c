#include <stdio.h>
#include <string.h>

int main() {
    char str[] = "This is a string";
    char *token;

    // Get the first token
    token = strtok(str, " ");

    // Loop to get all tokens
    while (token != NULL) {
        printf("Token: %s\n", token);
        token = strtok(NULL, " "); // Pass NULL to continue from where it left off
    }

    return 0;
}