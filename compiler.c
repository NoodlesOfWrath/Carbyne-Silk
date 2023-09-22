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
    int ReplacementIndex = 0;
    bool isString = false;
    bool justEndedString = false;

    for (int i = 0; i < sizeof(replacementMap) / sizeof(replacementMap[0]); i++)
    {
        if (line[0] == 0) // null character
        {
            break;
        }
        else if (isString) // if the current character is part of a string
        {
            tokenList[ReplacementIndex].type = "string";
            char *newChar = malloc(256); // max length of a string is 256 characters
            memset(newChar, 0, 256);     // clear the string

            for (int j = 0; j < 256; j++) // for every character in the string
            {
                if (line[0] == 34 || line[0] == 39) // end string
                {
                    newChar[j] = '\0';
                    isString = false;
                    justEndedString = true;
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
                    newChar[j] = line[0];
                    line++; // move the pointer to the next character
                }
            }

            if (strlen(newChar) == 0) // if the string is empty
            {
                newChar[0] = "";
            }

            tokenList[ReplacementIndex].value = newChar;

            ReplacementIndex++;
            i = -1; // reset the search
        }
        else if (strncmp(line, replacementMap[i].original, strlen(replacementMap[i].original)) == 0)
        {
            tokenList[ReplacementIndex] = replacementMap[i].replacementToken;
            if (strcmp(tokenList[ReplacementIndex].value, "QUOTE") == 0) // if the token is a quote
            {
                if (!justEndedString)
                {
                    isString = !isString;
                }
                else
                {
                    justEndedString = false;
                }
            }
            ReplacementIndex++;
            line += strlen(replacementMap[i].original); // move the pointer to the next character
            i = -1;                                     // reset the search
        }
        // check if it is a digit
        else if (i == 0 && line[0] > 47 && line[0] < 58) // If it is a number
        {
            tokenList[ReplacementIndex].type = "number"; // assign the type of the next token to number
            int *number = malloc(sizeof(int));
            *number = 0;
            // check if the next character is still a number
            while (line[0] > 47 && line[0] < 58)
            {
                *number *= 10;           // shift the number over one place
                *number += line[0] - 48; // convert the character to an integer
                line++;                  // move the pointer to the next character
            }
            i = -1; // reset the search
            tokenList[ReplacementIndex].value = number;
            ReplacementIndex++;
        }
        else if (i == sizeof(replacementMap) / sizeof(replacementMap[0]) - 1) // If it is not a Replacement
        {
            fprintf(stderr, "Error: unknown character %c on line %d\n", line[0], currentLine);
            exit(1);
        }
    }
    *tokenCount = ReplacementIndex;
}

typedef struct Syntax
{
    Token *tokens;
    int tokenCount;
    char *code;
} Syntax;

// replace $$ with the value of the token
char *ReplaceDollarDollar(char *code, int charLen, Token *tokens, int tokenLen)
{
    char *newCode = malloc(10000);
    memset(newCode, 0, 10000); // clear the string
    char *dollarDollar = strstr(code, "$$");
    for (int i = 0; i < charLen; i++)
    {
        if (strncmp(code, "$$", 2) == 0)
        {
            code += 2;
            // check if the next char is a number
            if (code[0] > 47 && code[0] < 58)
            {
                int tokenIndex = 0;
                int tokenNumber = 0;
                while (code[0] > 47 && code[0] < 58)
                {
                    tokenNumber *= 10;           // shift the number over one place
                    tokenNumber += code[0] - 48; // convert the character to an integer
                    code++;                      // move the pointer to the next character
                }
                if (tokenNumber > tokenLen)
                {
                    fprintf(stderr, "Error in line %d: $$%d used with only %d tokens\n", currentLine, tokenNumber, tokenLen);
                    exit(1);
                }
                else
                {
                    strcat(newCode, printToken(tokens[tokenNumber]));
                }
            }
            else
            {
                fprintf(stderr, "Error in line %d: $$ used with no number\n", currentLine);
                exit(1);
            }
        }
        else
        {
            strncat(newCode, code, 1);
            code++;
        }
    }
    return newCode;
}

printToken(Token token)
{
    char *output = malloc(100);
    memset(output, 0, 100); // clear the string

    if (strcmp(token.type, "string") == 0)
    {
        output = token.value;
    }
    else if (strcmp(token.type, "number") == 0)
    {
        sprintf(output, "%d", (int)*(token.value));
    }
    return output;
}
// make the tree for the print function

char *CompilePrint(Token *line, int tokenCount)
{
    char *generatedCode = malloc(10000);
    memset(generatedCode, 0, 10000); // clear the string

    // NULL means that the value does not matter
    Token stringPrintSyntax[6] = {{"function", "PRINT"}, {"symbol", "LPAREN"}, {"symbol", "QUOTE"}, {"string", NULL}, {"symbol", "QUOTE"}, {"symbol", "RPAREN"}};
    Token numberPrintSyntax[4] = {{"function", "PRINT"}, {"symbol", "LPAREN"}, {"number", NULL}, {"symbol", "RPAREN"}};
    Syntax possibleSyntaxes[2] = {{stringPrintSyntax, 6, "printf(\"$$3\\n\");"}, {numberPrintSyntax, 4, "printf(\"%d\\n\", $$2);"}};

    bool foundSyntax = false;
    for (int syntaxIndex = 0; syntaxIndex < 2; syntaxIndex++)
    {
        Syntax *currentSyntax = &possibleSyntaxes[syntaxIndex];

        // loop through possible syntaxes
        for (int j = 0; j < currentSyntax->tokenCount; j++)
        {
            if (strcmp(line[j].type, currentSyntax->tokens[j].type) == 0) // if the types match
            {
                if (currentSyntax->tokens[j].value != NULL) // if the value matters
                {
                    if (strcmp(line[j].value, currentSyntax->tokens[j].value) != 0) // if the values don't match
                    {
                        break; // move on to the next syntax
                    }
                }
                if (j == currentSyntax->tokenCount - 1) // if this is the last token
                {
                    foundSyntax = true;
                    strcat(generatedCode, ReplaceDollarDollar(currentSyntax->code, strlen(currentSyntax->code) - 3, line, tokenCount));
                    // print everything about the replace dollar dollar function

                    line += sizeof(currentSyntax) / sizeof(currentSyntax[0]); // move the pointer to the next character after the closing parenthesis
                    return generatedCode;
                }
            }
            else // if the types don't match
            {
                break; // move on to the next syntax
            }
        }
    }

    if (!foundSyntax) // if no syntax was found
    {
        fprintf(stderr, "\nError in line %d: invalid syntax for print function\n", currentLine);
        exit(1);
    }

    return generatedCode;
}

char *CallFunction(Token *line, int size)
{
    if (strcmp(line[0].type, "function") == 0 && strcmp(line[0].value, "PRINT") == 0)
    {
        return CompilePrint(line, size);
    }
}

char *GenerateCode(Token *program, int size)
{
    char *generatedCode = malloc(10000);
    memset(generatedCode, 0, 10000); // clear the string

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
        printf("%s :", tokens[i].type);

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
    char line[100]; // Assuming a maximum line length of 1000 characters

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
        int tokenCount = 0;
        ReplaceReplacements(line, Replacements, &tokenCount); // replace the replacements in the line with their tokens

        Token *trimmedReplacements = malloc(tokenCount * sizeof(Token));

        int trimmedReplacementsIndex = 0;
        for (size_t i = 0; i < tokenCount; i++) // for every token in the line
        {
            if (strcmp(Replacements[i].type, "replacement") != 0 && strcmp(Replacements[i].value, "") != 0)
            {
                trimmedReplacements[trimmedReplacementsIndex] = Replacements[i];
                trimmedReplacementsIndex++;
            }
            else
            {
                tokenCount--; // remove the token from the count
            }
        }

        printf("\n");

        // printTokens(trimmedReplacements, tokenCount);

        // printf("%s", ReplaceDollarDollar("printf(\"$$3\n\");", 16, trimmedReplacements, tokenCount));

        printf("%s \n", GenerateCode(trimmedReplacements, tokenCount));

        currentLine++;
    }

    printf("\n return 0;\n}\n");

    return 0;
}
