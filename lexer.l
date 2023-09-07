%{
    /* definition */
    #include "parser.tab.h"
%}


DIGIT [0-9]


/* rules */
%%
[ \t]     ; // Ignore whitespace
{DIGIT}+  { yylval.num = atoi(yytext); return NUMBER; }
"+"       { return PLUS; }
"-"       { return MINUS; }
"*"       { return TIMES; }
"/"       { return DIVIDE; }
"**"      { return POWER; }
"("       { return LPAREN; }
")"       { return RPAREN; }
\n        { return EOL; }
.         { return yytext[0]; }
%%


int yywrap() {
    return 1;
}
