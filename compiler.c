#include <stdio.h>
#include <stdbool.h>
#include <string.h>

int main() {
    FILE *file;
    char line[100]; // Assuming a maximum line length of 100 characters
    
    // Open the file for reading
    file = fopen("test.cbs", "r");
    
    if (file == NULL) {
        perror("Error opening the file");
        return 1;
    }

    printf("int main()\n{\n");
    
    // Read and print each line
    while (fgets(line, sizeof(line), file) != NULL) {
        ReplaceTokens(line);
        printf("\n");
    }
    
    printf("\n return 0;\n}\n");

    // Close the file
    fclose(file);
    
    return 0;
}

void ReplaceTokens(char *line)
{
    // Define token replacement mappings
    char *replacementMap[][2] = {
        {" ", ""},
        {"\t", ""},
        {"\n", "\n"},
        {"'", "QUOTE"},
        {"\"", "QUOTE"},
        {"print", "PRINT"},
        {"exp", "EXP"},
        {"(", "LPAREN"},
        {")", "RPAREN"},
        {"{", "LBRACE"},
        {"}", "RBRACE"},
        {"if", "IF"},
        {"==", "ISEQUALTO"},
        {"else", "ELSE"},
        {"while", "WHILE"},
        {"for", "FOR"}
        // Add more replacements as needed
    };

    bool hasTokens = false;
    bool string = false;

    for (size_t i = 0; i < sizeof(replacementMap) / sizeof(replacementMap[0]); i++)
    {
        if(line[0] == 0)
        {
            return;
        }
        if (strncmp(line, replacementMap[i][0], strlen(replacementMap[i][0])) == 0)
        {
            printf("%s ", replacementMap[i][1]);
            line += strlen(replacementMap[i][0]);
            hasTokens = true;
            i=-1; //reset the search
        }
        // check if it is a digit
        else if (i == 0 && line[0] > 48 && line[0] < 58)
        {
            printf("%c ", line[0]);
            hasTokens = true;
            line += 1;
            i=-1; //reset the search
        }
        else if (i == sizeof(replacementMap) / sizeof(replacementMap[0]) - 1)
        {
            printf("Character not recognized: String: %c, Decimal: %d ", line[0], line[0]);
            line += 1;
            i=-1; //reset the search
        }
    }
}