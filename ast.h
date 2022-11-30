/* File ast.h: Abstract-syntax-tree data structure for DJ */

#ifndef AST_H
#define AST_H

#include <stdlib.h>

/* define types of AST nodes */
typedef enum {
  /* program, class, field, method, and parameter declarations: */
  PROGRAM, 
  CLASS_DECL_LIST,
  CLASS_DECL,
  VAR_DECL_LIST,
  VAR_DECL,   
  METHOD_DECL_LIST,
  METHOD_DECL, 
  PARAM_DECL_LIST,
  PARAM_DECL, 
  /* types, including generic IDs: */
  INT_TYPE,
  AST_ID, 
  /* expression-lists: */
  EXPR_LIST,
  /* expressions: */
  DOT_METHOD_CALL_EXPR, /* E.ID(As) */
  METHOD_CALL_EXPR,     /* ID(As) */
  DOT_ID_EXPR,          /* E.ID */
  ID_EXPR,              /* ID */
  DOT_ASSIGN_EXPR,      /* E.ID = E */
  ASSIGN_EXPR,          /* ID = E */
  PLUS_EXPR,            /* E + E */
  MINUS_EXPR,           /* E - E */
  TIMES_EXPR,           /* E * E */
  EQUALITY_EXPR,        /* E==E */
  GREATER_THAN_EXPR,       /* E > E */
  NOT_EXPR,             /* !E */
  AND_EXPR,              /* E&&E */
  IF_THEN_ELSE_EXPR,    /* if(E) {Es} else {Es} */
  WHILE_EXPR,           /* while(E) {Es} */
  PRINT_EXPR,           /* print(E) */
  READ_EXPR,            /* read() */
  THIS_EXPR,            /* this */
  NEW_EXPR,             /* new */
  NULL_EXPR,            /* null */
  INTEGER_EXPR,     /* N */
  /* argument-lists: */
  ARG_LIST,
} ASTNodeType;

/* define a list of AST nodes */
typedef struct astlistnode {
  struct astnode *data;
  struct astlistnode *next;
} ASTList;

/* define the actual AST nodes */
typedef struct astnode {
  ASTNodeType typ;
  /* list of children nodes: */
  ASTList *children; /* head of the list of children */
  ASTList *childrenTail;
  /* which source-program line does this node end on: */
  unsigned int lineNumber;
  /* node attributes: */
  unsigned int intVal;
  char *idVal;
  
  unsigned int staticClassNum; /* class number in which this member resides */
  unsigned int staticMemberNum; /* when set to i, this member is the ith
                                   method/var in the staticClassNum-th class */
} ASTree;


ASTree *newNode(ASTNodeType t, unsigned int lineNum, unsigned int intAttribute,
  char *idAttribute);

/* Append an AST node onto a parent's list of children */
void appendChild(ASTree *parent, ASTree *newChild);

/* Print the AST to stdout with indentations marking tree depth. */
void printAST(ASTree *t);

#endif

