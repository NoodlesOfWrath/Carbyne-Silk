#include <stdio.h>
#include <stdbool.h>
#include <string.h>

int currentLine = 0;

typedef struct Token
{
    char *type;
    char *value;
} Token;

typedef struct Replacement
{
    char *original;
    Token replacementToken;
} Replacement;

// Define Replacement replacement mappings
Replacement replacementMap[19] = {
    {" ", {"replacement", ""}},
    {"\t", {"replacement", ""}},
    {"\n", {"symbol", "EOL"}},
    {"'", {"symbol", "QUOTE"}},
    {"\"", {"symbol", "QUOTE"}},
    {"print", {"function", "PRINT"}},
    {"(", {"symbol", "LPAREN"}},
    {")", {"symbol", "RPAREN"}},
    {"{", {"symbol", "LBRACE"}},
    {"}", {"symbol", "RBRACE"}},
    {"if", {"function", "IF"}},
    {"==", {"function", "ISEQUALTO"}},
    {"else", {"function", "ELSE"}},
    {"while", {"function", "WHILE"}},
    {"for", {"function", "FOR"}},
    {"*", {"operator", "TIMES"}},
    {"/", {"operator", "DIVIDE"}},
    {"+", {"operator", "PLUS"}},
    {"-", {"operator", "MINUS"}}};

// Define operators

char *ReplaceReplacements(char *line)
{
    Token tokenList[256];
    int ReplacementIndex = 0;

    for (size_t i = 0; i < sizeof(replacementMap); i++)
    {
        if (line[0] == 0)
        {
            break;
        }
        if (strncmp(line, replacementMap[i].original, strlen(replacementMap[i].original)) == 0)
        {
            printf("%s ", replacementMap[i].replacementToken.value);
            tokenList[ReplacementIndex] = replacementMap[i].replacementToken;
            ReplacementIndex++;
            line += strlen(replacementMap[i].original); // move the pointer to the next character
            i = -1;                                     // reset the search
        }
        // check if it is a digit
        else if (i == 0 && line[0] > 48 && line[0] < 58) // If it is a number
        {
            tokenList[ReplacementIndex].type = "number"; // assign the type of the next token
            char number[256];
            int numberIndex = 0;
            // check if the next character is a number
            while (line[0] > 48 && line[0] < 58)
            {
                number[numberIndex] = line[0];
                line++; // move the pointer to the next character
                numberIndex++;
            }
            printf("%s ", number);
            i = -1; // reset the search
            tokenList[ReplacementIndex].value = number;
            ReplacementIndex++;
        }
        else if (i == sizeof(replacementMap) / sizeof(replacementMap[0]) - 1) // If it is not a Replacement
        {
            tokenList[ReplacementIndex].type = "string";
            printf("%c ", line[0]);
            char newChar = *line;
            tokenList[ReplacementIndex].value = &newChar;

            ReplacementIndex++;
            line++; // move the pointer to the next character
            i = -1; // reset the search
        }
    }
    printf("\nactual string start \n");
    for (size_t i = 0; i < ReplacementIndex; i++)
    {
        printf("%s ", tokenList[i].value);
    }
    printf("\nactual string end \n");
    return &tokenList;
}

void parsetokenList(char *line)
{
    char tokenList[sizeof(line)] = "tokenList<";
    for (size_t i = 0; i < sizeof(line); i++)
    {
        if (line[i] == "\"" || line[i] == "'")
        {
            line++;
            tokenList[i + 8] = '>';
            return tokenList;
        }
        else if (i != sizeof(line) - 1)
        {
            tokenList[i + 8] = line[i];
            line++;
        }
        else
        {
            fprintf(stderr, "Error in line %d: tokenList not closed\n", currentLine);
            return;
        }
    }
}

char *GenerateCode(char *program)
{
    char generatedCode[10000];

    return &generatedCode;
}

int main()
{
    FILE *file;
    char line[100]; // Assuming a maximum line length of 100 characters

    // Open the file for reading
    file = fopen("test.cbs", "r");

    if (file == NULL)
    {
        perror("Error opening the file");
        return 1;
    }

    printf("int main()\n{\n");

    // Read and print each line
    while (fgets(line, sizeof(line), file) != NULL)
    {
        char *Replacements = ReplaceReplacements(line);

        for (size_t i = 0; i < sizeof(Replacements) / sizeof(Replacements[0]); i++)
        {
            // printf("accessing Replacement %d ", i);
            // printf("%s\n", Replacements[i]);
        }
        currentLine++;
    }

    printf("\n return 0;\n}\n");

    // Close the file
    fclose(file);

    return 0;
}
