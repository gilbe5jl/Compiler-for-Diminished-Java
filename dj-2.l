/* DJ PARSER */

%code provides {
  
  #include "lex.yy.c"

  /* Function for printing generic syntax-error messages */
  void yyerror(const char *str) {
    printf("Syntax error on line %d at token %s\n",yylineno,yytext);
    printf("(This version of the compiler exits after finding the first ");
    printf("syntax error.)\n");
    exit(-1);
  }
}



%token MAIN CLASS EXTENDS INTTYPE IF ELSE WHILE
%token PRINT READ THIS NEW NUL INTEGER
%token ID ASSIGN PLUS MINUS TIMES EQUALITY GREATER
%token AND NOT DOT SEMICOLON COMMA LBRACE RBRACE
%token LPAREN RPAREN ENDOFFILE

%start pgm

%right ASSIGN
%left AND
%nonassoc EQUALITY GREATER
%left PLUS MINUS
%left TIMES
%right NOT
%left DOT

%%

pgm : class_list MAIN LBRACE var_list exp_list RBRACE ENDOFFILE
    {return 0;}
    ;

class_list :
        | class_list class_dec
       ;

class_dec : CLASS ID EXTENDS ID LBRACE var_list  meth_list RBRACE
        | CLASS ID EXTENDS ID LBRACE var_list RBRACE
        ;

meth_list : method
        | meth_list method
        ;

method : INTTYPE ID LPAREN par_list RPAREN LBRACE var_list exp_list RBRACE
      | ID ID LPAREN par_list RPAREN LBRACE var_list exp_list RBRACE
        ;

par_list :
        | par
        | par_list COMMA var
        ;

par : INTTYPE ID
    | ID ID
    ;


var_list :
        | var_list var SEMICOLON
        ;

var : INTTYPE ID
    | ID ID
    ;


exp_list : exp SEMICOLON
        | exp_list exp SEMICOLON
         ;

exp : INTEGER
    | ID
    | NUL
    | exp PLUS exp
    | exp MINUS exp
    | exp TIMES exp
    | exp EQUALITY exp
    | exp GREATER exp
    | NOT exp
    | exp AND exp
    | ID ASSIGN exp
    | exp DOT ID ASSIGN exp
    | exp DOT ID
    | IF LPAREN exp RPAREN LBRACE exp_list RBRACE ELSE LBRACE exp_list RBRACE
    | WHILE LPAREN exp RPAREN LBRACE exp_list RBRACE
    | NEW ID LPAREN RPAREN
    | THIS
    | PRINT LPAREN exp RPAREN
    | READ LPAREN RPAREN
    | ID LPAREN arg_list RPAREN
    | exp DOT ID LPAREN arg_list RPAREN
    | LPAREN exp RPAREN
    ;

arg_list :
        | arg
        | arg_list COMMA arg
        ;
arg : ID
    | INTEGER
    | exp_list
    ;





%%

int main(int argc, char **argv) {
  if(argc!=2) {
    printf("Usage: dj-parse filename\n");
    exit(-1);
  }
  yyin = fopen(argv[1],"r");
  if(yyin==NULL) {
    printf("ERROR: could not open file %s\n",argv[1]);
    exit(-1);
  }

  /* parse the input program */
  return yyparse();
}
