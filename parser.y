%{
    /* definitions */
    #include <math.h>
%}

%union {
    int num;
    char sym;
}

%token EOL
%token<num> NUMBER
%type<num> exp
%token PLUS
%token MINUS
%token TIMES
%token DIVIDE
%token POWER
%token LPAREN
%token RPAREN

/* rules */
%%

input:
|   line input;
;

line: 
    exp EOL { printf("%d\n", $1); }
|   EOL;

exp: NUMBER { $$ = $1;}
|    LPAREN exp RPAREN { $$ = $2; }
|    exp PLUS exp { $$ = $1 + $3; }
|    exp MINUS exp { $$ = $1 - $3; }
|    exp TIMES exp { $$ = $1 * $3; }
|    exp DIVIDE exp { $$ = $1 / $3; }
|    exp POWER exp { $$ = pow($1, $3); }
;

%%

int main()
{
    yyparse();

    return 0;
}

yyerror(char *s)
{
    printf(stderr, "ERROR: %s\n", s);

    return 0;
}
