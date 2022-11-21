%option yylineno
%pointer

%{
    #include <cstdlib>
    #include <cerrno>
    #include <climits>
    #include <limits>
    #include "ast.hpp"
    #include "parser.hpp"
    
	void yyerror(const char *);
%}

%x R_COMMENT

%%

" "                   {}
"\t"                  {}
<R_COMMENT>"\n"       {}
"\n"                  {}

"/*"                  {BEGIN(R_COMMENT);}
<R_COMMENT>"*/"       {BEGIN(INITIAL);}
<R_COMMENT><<EOF>>    {yyerror("dangling comment");}
<R_COMMENT>.          {}

"print"               {return T_PRINT;}
"return"              {return T_RETURN;}
"if"                  {return T_IF;}
"else"                {return T_ELSE;}
"new"                 {return T_NEW;}
"int"                 {return T_INT;}
"boolean"             {return T_BOOLEAN;}
"none"                {return T_NONE;}
"True"                {return T_TRUE;}
"False"               {return T_FALSE;}
"extends"             {return T_EXTENDS;}
"while"               {return T_WHILE;}
"do"                  {return T_DO;}

"?"                   {return T_QUESTION;}
":"                   {return T_COLON;}
"or"                  {return T_OR;}
"and"                 {return T_AND;}
">"                   {return T_GREATER;}
">="                  {return T_GREATER_EQUAL;}
"=="                  {return T_EQUAL_EQUAL;}
"+"                   {return T_PLUS;}
"-"                   {return T_MINUS;}
"*"                   {return T_MULTIPLIES;}
"/"                   {return T_DIVIDES;}
"not"                 {return T_NOT;}
"{"                   {return T_LEFT_BRACKET;}
"}"                   {return T_RIGHT_BRACKET;}
"("                   {return T_LEFT_PARENTHESIS;}
")"                   {return T_RIGHT_PARENTHESIS;}
";"                   {return T_SEMICOLON;}

","                   {return T_COMMA;}
"."                   {return T_PERIOD;}
"="                   {return T_EQUAL;}
"->"                  {return T_ARROW;}

[a-zA-Z][a-zA-Z0-9]*  {yylval.base_char_ptr = yytext; return T_ID;}
0|[1-9][0-9]*         {yylval.base_int = atoi(yytext); return T_INTEGER;}

.                     {yyerror("invalid character");}

%%

int yywrap(void) {
  return 1;
}
