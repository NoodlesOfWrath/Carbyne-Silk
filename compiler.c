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
Replacement replacementMap[21] = {
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
    {"range", {"function", "RANGE"}},
    {"for", {"function", "FOR"}},
    {"in", {"keyword", "IN"}},
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
            char *type = malloc(7);
            type = "string";
            tokenList[ReplacementIndex].type = type;
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
        else if (strncmp(line, replacementMap[i].original, strlen(replacementMap[i].original)) == 0) // if it matches the current replacement
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
            char *type = malloc(sizeof(char) * 7);
            type = "number";
            tokenList[ReplacementIndex].type = type; // assign the type of the next token to number
            // int *number = malloc(sizeof(int));
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
        else if (i == sizeof(replacementMap) / sizeof(replacementMap[0]) - 1) // If it is not a recognized token
        {
            // merge onto the previous token if it is also unknown
            if (ReplacementIndex != 0 && strcmp(tokenList[ReplacementIndex - 1].type, "unknown") == 0)
            {
                char *newChar = malloc(256); // max length of a string is 256 characters
                memset(newChar, 0, 256);     // clear the string
                strcat(newChar, tokenList[ReplacementIndex - 1].value);
                strcat(newChar, line[0]);
                tokenList[ReplacementIndex - 1].value = newChar;
                line++;
                i = -1;
                // do not increase the ReplacementIndex
            }
            else
            {
                tokenList[ReplacementIndex].type = "unknown";
                char *newChar = malloc(256);
                memset(newChar, 0, 256); // clear the string
                newChar[0] = line[0];
                tokenList[ReplacementIndex].value = newChar;
                ReplacementIndex++;
                line++;
                i = -1; // reset the search
            }
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
    while (*code != 0)
    {
        if (strncmp(code, "$$", 2) == 0)
        {
            code += 2;
            // check if the next char is a number
            if (code[0] > 47 && code[0] < 58)
            {
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
                    strcat(newCode, TokenToString(tokens[tokenNumber]));
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

TokenToString(Token token)
{
    char *output = malloc(100);
    memset(output, 0, 100); // clear the string

    if (strcmp(token.type, "string") == 0 || strcmp(token.type, "unknown") == 0)
    {
        output = token.value;
    }
    else if (strcmp(token.type, "number") == 0)
    {
        sprintf(output, "%d", (int)*(token.value));
    }
    return output;
}

char *TokensCompare(Syntax *currentSyntax, Token *line, int lineTokenCount)
{
    char *generatedCode = malloc(10000);
    memset(generatedCode, 0, 10000); // clear the string

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
                strcat(generatedCode, ReplaceDollarDollar(currentSyntax->code, strlen(currentSyntax->code), line, lineTokenCount));
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
    return generatedCode; // no syntax was found generatedCode is empty
}

char *CompilePrint(Token *line, int tokenCount)
{
    char *generatedCode = malloc(10000);
    memset(generatedCode, 0, 10000); // clear the string

    // NULL means that the value does not matter
    Token stringPrintSyntax[6] = {{"function", "PRINT"}, {"symbol", "LPAREN"}, {"symbol", "QUOTE"}, {"string", NULL}, {"symbol", "QUOTE"}, {"symbol", "RPAREN"}};
    Token numberPrintSyntax[4] = {{"function", "PRINT"}, {"symbol", "LPAREN"}, {"number", NULL}, {"symbol", "RPAREN"}};
    Token variablePrintSyntax[4] = {{"function", "PRINT"}, {"symbol", "LPAREN"}, {"unknown", NULL}, {"symbol", "RPAREN"}};
    Syntax possibleSyntaxes[3] = {{stringPrintSyntax, 6, "printf(\"$$3\\n\");"}, {numberPrintSyntax, 4, "printf(\"%d\\n\", $$2);"}, {variablePrintSyntax, 4, "printf(\"%d\\n\", $$2);"}};

    for (int syntaxIndex = 0; syntaxIndex < 3; syntaxIndex++)
    {
        Syntax *currentSyntax = &possibleSyntaxes[syntaxIndex];
        char *generatedCode = TokensCompare(currentSyntax, line, tokenCount);

        if (generatedCode[0] != 0)
        {
            return generatedCode;
        }
    }

    fprintf(stderr, "\nError in line %d: invalid syntax for print function\n", currentLine);
    exit(1);
}

char *CompileFor(Token *line, int tokenCount)
{
    char *generatedCode = malloc(10000);
    memset(generatedCode, 0, 10000); // clear the string

    // NULL means that the value does not matter
    Token forRangeSyntax[6] = {{"function", "FOR"}, {"unknown", "i"}, {"keyword", "IN"}, {"function", "RANGE"}, {"symbol", "LPAREN"}, {"number", NULL}, {"symbol", "RPAREN"}};
    Syntax possibleSyntaxes[1] = {{forRangeSyntax, 6, "for(int $$1 = 0; $$1 < $$5; $$1++){"}};

    for (int syntaxIndex = 0; syntaxIndex < 1; syntaxIndex++)
    {
        Syntax *currentSyntax = &possibleSyntaxes[syntaxIndex];
        char *generatedCode = TokensCompare(currentSyntax, line, tokenCount);

        if (generatedCode[0] != 0)
        {
            return generatedCode;
        }
    }

    fprintf(stderr, "\nError in line %d: invalid syntax in for function\n", currentLine);
    exit(1);
}

char *CallFunction(Token *line, int size)
{
    if (strcmp(line[0].type, "function") != 0)
    {
        fprintf(stderr, "\nError in line %d: invalid syntax for function call\n", currentLine);
        exit(1);
    }

    if (strcmp(line[0].value, "PRINT") == 0)
    {
        return CompilePrint(line, size);
    }
    else if (strcmp(line[0].value, "FOR") == 0)
    {
        return CompileFor(line, size);
    }
    else
    {
        fprintf(stderr, "\nError in line %d: function %s not found\n", currentLine, line[0].value);
        exit(1);
    }
}

char *GenerateCode(Token *program, int size)
{
    char *generatedCode = malloc(10000);
    memset(generatedCode, 0, 10000); // clear the string

    for (int i = 0; i < size; i++) // for token in program
    {
        Token token = program[0];
        if (strcmp(token.type, "function") == 0) // only for print function temporarily
        {
            char *functionCode = CallFunction(program, size);
            strcat(generatedCode, functionCode);
            i += sizeof(functionCode) / sizeof(functionCode[0]);
        }
        if (strcmp(token.type, "symbol") == 0 && strcmp(token.value, "EOL") == 0)
        {
            strcat(generatedCode, "\n");
        }
        if (strcmp(token.type, "symbol") == 0 && strcmp(token.value, "RBRACE") == 0)
        {
            strcat(generatedCode, "}");
        }
        program++;
    }

    return generatedCode;
}

void printTokens(Token *tokens, int tokenCount)
{
    Token replacementsTemp[tokenCount];
    for (size_t i = 0; i < tokenCount; i++) // for every token in the line
    {
        replacementsTemp[i] = tokens[i];
    }
    for (size_t i = 0; i < tokenCount; i++) // for every token in the line
    {
        printf("%s : ", tokens[i].type);

        if (strcmp(tokens[i].type, "number") == 0)
        {
            printf("%d ", *tokens[i].value);
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
        memset(trimmedReplacements, 0, tokenCount * sizeof(Token)); // clear the string

        int trimmedReplacementsIndex = 0;
        int newTokenCount = tokenCount;
        for (size_t i = 0; i < tokenCount; i++) // for every token in the line
        {
            if (strcmp(Replacements[i].type, "replacement") != 0 && strcmp(Replacements[i].value, "") != 0)
            {
                trimmedReplacements[trimmedReplacementsIndex] = Replacements[i];
                trimmedReplacementsIndex++;
            }
            else
            {
                newTokenCount--; // remove the token from the count
            }
        }

        printf("\n");

        // printTokens(trimmedReplacements, newTokenCount);

        // printf("%s", ReplaceDollarDollar("for(int $$1 = 0; $$1 < $$5; $$1++){", 32, trimmedReplacements, newTokenCount));

        printf("%s \n", GenerateCode(trimmedReplacements, newTokenCount));

        currentLine++;
    }

    printf("\n return 0;\n}\n");

    return 0;
}
