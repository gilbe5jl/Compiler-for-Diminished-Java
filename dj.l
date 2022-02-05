%option yylineno
%option noyywrap

%{
  #define DEBUG 1
  typedef int Token;
  Token scanned(Token t);
%}

digit	  [0-9]*
wspace	[ \t\r\n]+ 
id      [a-zA-Z][a-z0-9A-Z_]*

%%

main  {return scanned(MAIN);}
class {return scanned(CLASS);}
extends {return scanned(EXTENDS);}
int  {return scanned(INTTYPE);}
if {return scanned(IF);}
else {return scanned(ELSE);}
while {return scanned(WHILE);}

print {return scanned(PRINT);}
read {return scanned(READ);}
this {return scanned(THIS);}
new {return scanned(NEW);}
null {return scanned(NUL);}
{digit}  {return scanned(INTEGER);}

{id}  {return scanned(ID);}
"="  {return scanned(ASSIGN);}
"+"  {return scanned(PLUS);}
"-"  {return scanned(MINUS);}
"*"  {return scanned(TIMES);}
"==" {return scanned(EQUALITY);}
">"  {return scanned(GREATER);}

"&&" {return scanned(AND);}
"!"  {return scanned(NOT);}
"."  {return scanned(DOT);}
";" {return scanned(SEMICOLON);}
","  {return scanned(COMMA);}


"{" {return scanned(LBRACE);}
"}" {return scanned(RBRACE);}
"(" {return scanned(LPAREN);}
")" {return scanned(RPAREN);}








"//" { char c = input(); while(c!='\n' && c!=EOF) c=input();}
{wspace} { /*skip whitespace*/ }
<<EOF>> {return scanned(ENDOFFILE);}
. { if(DEBUG) printf("\n"); printf("Lex error on line %d: Illegal character %s\n", yylineno,yytext); exit(-1);}
%%

/* process and return scanned token t */
Token scanned(Token t) {
  if(DEBUG==0) return t;
  switch(t) {
    case MAIN: printf("MAIN "); return t;
    case CLASS: printf("CLASS "); return t;
    case EXTENDS: printf("EXTENDS "); return t;
    case INTTYPE: printf("INTTYPE "); return t;
    case IF: printf("IF "); return t;
    case ELSE: printf("ELSE "); return t;
    case WHILE: printf("WHILE "); return t;

    case PRINT: printf("PRINT "); return t;
    case READ: printf("READ"); return t;
    case THIS: printf("THIS"); return t;
    case NEW: printf("NEW"); return t;
    case NUL: printf("NUL "); return t;
    case INTEGER: printf("INTEGER(%s) ",yytext); return t;

    case ID: printf("ID(%s) ",yytext); return t;
    case ASSIGN: printf("ASSIGN "); return t;
    case PLUS: printf("PLUS "); return t;
    case MINUS: printf("MINUS "); return t;
    case TIMES: printf("TIMES "); return t;
    case EQUALITY: printf("EQUALITY "); return t;
    case GREATER: printf("GREATER "); return t;

    case LBRACE: printf("LBRACE "); return t;
    case RBRACE: printf("RBRACE "); return t;
    case LPAREN: printf("LPAREN "); return t;
    case RPAREN: printf("RPAREN "); return t;
    case AND: printf("AND "); return t;
    case NOT: printf("NOT "); return t;
    case DOT: printf("DOT "); return t;
    case COMMA: printf("COMMA "); return t;
    case SEMICOLON: printf("SEMICOLON "); return t;

    case ENDOFFILE: printf("ENDOFFILE\n"); return t;
    default: printf("ERROR: invalid token in scanned().\n"); exit(-1);
  }
}
