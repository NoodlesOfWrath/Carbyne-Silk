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
            int number = 0;
            // check if the next character is still a number
            while (line[0] > 47 && line[0] < 58)
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
            fprintf(stderr, "Error: unknown character %c on line %d\n", line[0], currentLine);
            exit(1);
        }
    }
    *tokenCount = ReplacementIndex;
}

typedef struct Syntax
{
    Token *tokens;
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
                    strcat(newCode, tokens[tokenNumber].value);
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

// make the tree for the print function

char *CompilePrint(Token *line, int tokenCount)
{
    char *generatedCode = malloc(10000);
    memset(generatedCode, 0, 10000); // clear the string
    // NULL means that the value does not matter
    Token stringPrintSyntax[6] = {{"function", "PRINT"}, {"symbol", "LPAREN"}, {"symbol", "QUOTE"}, {"string", NULL}, {"symbol", "QUOTE"}, {"symbol", "RPAREN"}};
    Token numberPrintSyntax[4] = {{"function", "PRINT"}, {"symbol", "LPAREN"}, {"number", NULL}, {"symbol", "RPAREN"}};
    Syntax *possibleSyntaxes[2] = {{stringPrintSyntax, "printf(\"%s\\n\", $$4);"}, {numberPrintSyntax, "printf(\"%d\\n\", $$3);"}};

    bool foundSyntax = false;
    for (int i = 0; i < 2; i++)
    {
        Syntax *currentSyntax = possibleSyntaxes[i];
        // loop through possible syntaxes
        for (int j = 0; j < sizeof(currentSyntax) / sizeof(currentSyntax[0]); j++)
        {
            if (strcmp(line[j].type, currentSyntax->tokens[j].type) == 0)
            {
                if (currentSyntax->tokens[j].value != NULL)
                {
                    if (strcmp(line[j].value, currentSyntax->tokens[j].value) == 0)
                    {
                        if (j == sizeof(currentSyntax) / sizeof(currentSyntax[0]) - 1) // if the last token matches
                        {
                            foundSyntax = true;
                            strcat(generatedCode, ReplaceDollarDollar(currentSyntax->code, j, line, tokenCount));
                            line += sizeof(currentSyntax) / sizeof(currentSyntax[0]); // move the pointer to the next character after the closing parenthesis
                        }
                        else
                        {
                            continue; // move on to the next token
                        }
                        continue; // move on to the next token
                    }
                }
            }
            else
            {
                break; // move on to the next syntax
            }
        }
    }

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
            if (strlen(line[3].value) != 0)
            {
                strcat(generatedCode, line[3].value);
            }
            else
            {
                strcat(generatedCode, "");
            }
        }
        else
        {
            fprintf(stderr, "Error in line %d: print called with non-string or non-number type %s %s\n", currentLine, line[3].type, line[3].value);
            exit(1);
        }
        if (strcmp(line[4].type, "symbol") == 0 && strcmp(line[4].value, "QUOTE") == 0)
        {
            strcat(generatedCode, "\\n"); // add a newline
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

        printf("%s", ReplaceDollarDollar("printf(\"$$3\\n\");", 15, trimmedReplacements, tokenCount));

        // printf("%s ", GenerateCode(trimmedReplacements, tokenCount));

        currentLine++;
    }

    printf("\n return 0;\n}\n");

    return 0;
}
