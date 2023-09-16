#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

int currentLine = 1;

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

void *ReplaceReplacements(char *line, Token *tokenList, int *tokenCount)
{
    // Token tokenList[256];
    int ReplacementIndex = 0;

    for (size_t i = 0; i < sizeof(replacementMap) / sizeof(replacementMap[0]); i++)
    {
        if (line[0] == 0) // null character
        {
            break;
        }
        if (strncmp(line, replacementMap[i].original, strlen(replacementMap[i].original)) == 0)
        {
            tokenList[ReplacementIndex] = replacementMap[i].replacementToken;
            ReplacementIndex++;
            line += strlen(replacementMap[i].original); // move the pointer to the next character
            i = -1;                                     // reset the search
        }
        // check if it is a digit
        else if (i == 0 && line[0] > 48 && line[0] < 58) // If it is a number
        {
            tokenList[ReplacementIndex].type = "number"; // assign the type of the next token to number
            int number = 0;
            // check if the next character is still a number
            while (line[0] > 48 && line[0] < 58)
            {
                number *= 10;           // shift the number over one place
                number += line[0] - 48; // convert the character to an integer
                line++;                 // move the pointer to the next character
            }
            i = -1; // reset the search
            tokenList[ReplacementIndex].value = number;
            ReplacementIndex++;
        }
        else if (i == sizeof(replacementMap) / sizeof(replacementMap[0]) - 1) // If it is not a Replacement
        {
            tokenList[ReplacementIndex].type = "string";
            char *newChar = malloc(256);  // max length of a string is 256 characters
            for (int i = 0; i < 256; i++) // for every character in the string
            {
                if (line[0] == 34 || line[0] == 39) // end string
                {
                    newChar[i] = '\0';
                    // do not move the pointer to the next character so the closing quote token can be read
                    break;
                }
                else if (line[0] == "\n" || line[0] == "\0") // end of line
                {
                    fprintf(stderr, "Error: string not closed on line %d\n", currentLine);
                    exit(1);
                }
                else // not the end of string
                {
                    newChar[i] = line[0];
                    line++; // move the pointer to the next character
                }
            }

            tokenList[ReplacementIndex].value = newChar;

            ReplacementIndex++;
            i = -1; // reset the search
        }
    }
    *tokenCount = ReplacementIndex;
}

char *CallFunction(Token *line, int size)
{
    char *generatedCode = malloc(1000);
    memset(generatedCode, 0, 1000); // clear the string

    printf("%s", generatedCode);

    if (strcmp(line[0].type, "function") != 0)
    {
        fprintf(stderr, "Error in line %d: CallFunction used with non-function\n", currentLine);
        exit(1);
    }
    if (strcmp(line[0].value, "PRINT") == 0)
    {
        strcat(generatedCode, "printf");
        if (strcmp(line[1].type, "symbol") == 0 && strcmp(line[1].value, "LPAREN") == 0)
        {
            strcat(generatedCode, "(");
        }
        else
        {
            fprintf(stderr, "Error in line %d: PRINT called with no opening parenthesis\n", currentLine);
            exit(1);
        }
        if (strcmp(line[2].type, "symbol") == 0 && strcmp(line[2].value, "QUOTE") == 0)
        {
            strcat(generatedCode, "\"");
        }
        else
        {
            fprintf(stderr, "Error in line %d: PRINT called with no opening quote\n", currentLine);
            exit(1);
        }
        if (strcmp(line[3].type, "string") == 0)
        {
            strcat(generatedCode, line[3].value);
        }
        else
        {
            fprintf(stderr, "Error in line %d: PRINT called with non-string or non-number type %c\n", currentLine, line[2].type);
            exit(1);
        }
        if (strcmp(line[4].type, "symbol") == 0 && strcmp(line[4].value, "QUOTE") == 0)
        {
            strcat(generatedCode, "\"");
        }
        else
        {
            fprintf(stderr, "Error in line %d: PRINT called with no closing quote\n", currentLine);
            exit(1);
        }
        if (strcmp(line[5].type, "symbol") == 0 && strcmp(line[5].value, "RPAREN") == 0)
        {
            strcat(generatedCode, ")");
            line += 6; // move the pointer to the next character after the closing parenthesis
        }
        else
        {
            fprintf(stderr, "Error in line %d: PRINT called with no closing parenthesis\n", currentLine);
            exit(1);
        }
    }
    return generatedCode;
}

char *GenerateCode(Token *program, int size)
{
    char *generatedCode = malloc(1000);
    memset(generatedCode, 0, 1000); // clear the string

    for (int i = 0; i < size; i++) // for token in program
    {
        Token token = program[i];
        if (strcmp(token.type, "function") == 0 && strcmp(token.value, "PRINT") == 0) // only for print function temporarily
        {
            strcat(generatedCode, CallFunction(program, size));
        }
        if (strcmp(token.type, "symbol") == 0 && strcmp(token.value, "EOL") == 0)
        {
            strcat(generatedCode, ";\n");
        }
    }

    return generatedCode;
}

void printTokens(Token *tokens, int tokenCount)
{
    for (size_t i = 0; i < tokenCount; i++) // for every token in the line
    {
        if (tokens[i].type == "number")
        {
            printf("%d ", tokens[i].value);
        }
        else
        {
            printf("%s ", tokens[i].value);
        }
    }
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
    while (fgets(line, sizeof(line), file) != NULL) // for every line in the file
    {
        Token Replacements[256];
        int tokenCount = 100;
        ReplaceReplacements(line, Replacements, &tokenCount); // replace the replacements in the line with their tokens

        Token *trimmedReplacements = malloc(tokenCount * sizeof(Token));

        for (size_t i = 0; i < tokenCount; i++) // for every token in the line
        {
            trimmedReplacements[i] = Replacements[i];
        }

        printf("\n");

        printf("%s ", GenerateCode(trimmedReplacements, tokenCount));

        currentLine++;
    }

    printf("\n return 0;\n}\n");

    return 0;
}
