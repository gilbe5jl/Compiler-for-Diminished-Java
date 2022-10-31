#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "typecheck.h"
#include "symtbl.h"
/* Define static types */
#define ERROR_TYPE -4
#define UNDEFINED_TYPE -3
#define NULL_TYPE -2
#define INT_TYPE -1
#define OBJECT_TYPE 0
#define TRUE 1
#define FALSE 0
  void checkVarDeclListForSuperClasses(VarDecl *varDeclList, int listSize, int superclassType){
    // check class has a super-class
    if(superclassType == OBJECT_TYPE){
        return;
    }
    else {
        int i, j;
        for(i=0; i< listSize; i++){
            VarDecl *varDecl = &varDeclList[i];
            for(j=0; j<classesST[superclassType].numVars;j++){
                VarDecl *varDecl2 = &classesST[superclassType].varList[j];
                if(strcmp(varDecl->varName, varDecl2->varName)==0){
                    printSemanticError("Var declared multiple times in a superclass",
							varDecl->varNameLineNumber);
                }
            }
        }
    }
    checkVarDeclListForSuperClasses(varDeclList, listSize, classesST[superclassType].superclass);
}

void checkClassSuperType(ClassDecl classDecl, int classType){
    int superType = classDecl.superclass;
    if(superType == classType){
        printSemanticError("Superclass should have different type", classDecl.superclassLineNumber);
    }
    if(superType < OBJECT_TYPE){
        printSemanticError("Undefined superclass type", classDecl.superclassLineNumber);
    }
}
void checkVarDeclList(VarDecl *mainBlockST, int numMainBlockLocals){
    /* Check var names are unique */
    int i, j;
    for(i=0; i<numMainBlockLocals-1; i++){
        VarDecl *var1 = &mainBlockST[i];
        
        for(j=i+1; j<numMainBlockLocals; j++){
            VarDecl *var2 = &mainBlockST[j];
            
            if(strcmp(var1->varName, var2->varName)==0){
              printf("%s is declared multiple times\n", var1->varName);
                printSemanticError("Var declared multiple times in main method", var1->varNameLineNumber);
            }
        }
    }
    
    /* check var type  >= -1 */
    for(i=0; i<numMainBlockLocals; i++){
        VarDecl *var1 = &mainBlockST[i];
        if(var1->type < INT_TYPE){
            printSemanticError("Undefined variable", var1->varNameLineNumber);
        }
    }
}
void checkClassMethod(ClassDecl classDecl, int classType){
    int i, j, k, m;
    
    /* check parameters in each method */
    for(i=0; i<classDecl.numMethods;i++){
        MethodDecl *methodDecl = &classDecl.methodList[i];
        
        /* chech parameters */
        for(k=0; k<methodDecl->numParams;k++){
            VarDecl *parDecl = &methodDecl->paramST[k];
            for(m = k+1; m<methodDecl->numParams; m++){
                VarDecl *parDecl2 = &methodDecl->paramST[m];
                if(strcmp(parDecl->varName, parDecl2->varName)==0){
                    printSemanticError("Same method arguments",
						methodDecl->methodNameLineNumber);
                }
            }
            if (parDecl->type < INT_TYPE) {
				printSemanticError("Method parameter should have valid type", parDecl->typeLineNumber);
			}
        }
        if (methodDecl->returnType < INT_TYPE) {
			printSemanticError("Method return parameter should have valid type", methodDecl->returnTypeLineNumber);
		}
    }
    
    /* check method names are unique */
    for (i = 0; i < classDecl.numMethods - 1; i++) {
		MethodDecl *methodDecl = &classDecl.methodList[i];
		for (j = i + 1; j < classDecl.numMethods; j++) {
			MethodDecl *methodDecl2 = &classDecl.methodList[j];
			if (strcmp(methodDecl->methodName, methodDecl2->methodName) == 0) {
				printSemanticError("Method declared multiple times",
						methodDecl->methodNameLineNumber);
			}
		}
	}
    
    /* check method body */
    for (i = 0; i < classDecl.numMethods; i++) {
        MethodDecl *methodDecl = &classDecl.methodList[i];
        // check variables are unique in method
        checkVarDeclList(methodDecl->localST, methodDecl->numLocals);
        // check parameter and local vars
        for (j = 0; j < methodDecl->numLocals; j++) {
			VarDecl *varDecl = &methodDecl->localST[j];
			for (k = 0; k < methodDecl->numParams; k++) {
				VarDecl *parDecl = &methodDecl->paramST[k];
				if (strcmp(varDecl->varName, parDecl->varName) == 0) {
					printSemanticError("Var declared multiple times as parameter", varDecl->varNameLineNumber);
				}
			}
		}
        
        /* check method body expressions */
        int methodReturnType = typeExprs(methodDecl->bodyExprs, classType, i);
        if (isSubtype(methodReturnType, methodDecl->returnType) == FALSE) {
			printSemanticError("Return type mismatch", methodDecl->returnTypeLineNumber);
		}
    }
}

void checkSuperClassMethods(ClassDecl classDecl, int classType, int superType){
    if(superType == OBJECT_TYPE){
        return;
    }
    else{
        int i, j, k;
		for (i = 0; i < classDecl.numMethods; i++) {
			MethodDecl *methodDecl = &classDecl.methodList[i];
			for (j = 0; j < classesST[superType].numMethods; j++) {
				MethodDecl *methodDecl2 = &classesST[superType].methodList[j];
				if (strcmp(methodDecl->methodName, methodDecl2->methodName) == 0) {
					if (methodDecl->numParams != methodDecl2->numParams) {
						printSemanticError("Super class method parameter count mismatch",
								methodDecl->methodNameLineNumber);
					}         
                    for (k = 0; k < methodDecl->numParams; k++) {
						VarDecl *parDecl = &methodDecl->paramST[k];
						VarDecl *parDecl2 = &methodDecl2->paramST[k];
						if (parDecl->type != parDecl2->type) {
							printSemanticError("Super class method parameter type mismatch",
								parDecl->typeLineNumber);
						}
                    }
					if (methodDecl->returnType != methodDecl2->returnType) {
						printSemanticError("Super class method return type mismatch", methodDecl->returnTypeLineNumber);
					}
                }
            }
            checkSuperClassMethods(classDecl, classType, classesST[superType].superclass);
        }
    }
}
void checkClasses(){
    /* 1) Check class names are unique */
    int i, j;
    for(i=0; i<numClasses-1; i++){
        ClassDecl *class1 = &classesST[i];
        for(j=i+1; j<numClasses;j++){
            ClassDecl *class2 = &classesST[j];
            if(strcmp(class1->className, class2->className)==0){
                printSemanticError("Class name declared multiple times", class1->classNameLineNumber);
            }
        }
    }
    /* 2) check each class is well typed */
    for(i=1; i< numClasses; i++){
        ClassDecl *class1 = &classesST[i];
        /* c, d) check class variables */
        checkVarDeclList(class1->varList, class1->numVars);
        /* b) check variables in class's superclass are unique */
        checkVarDeclListForSuperClasses(class1->varList, class1->numVars, class1->superclass);
        /* a) check super class type */
        checkClassSuperType(*class1, i);
        /* check methods in class */
        checkClassMethod(*class1, i);
        /* check method in super classes with same name have same type */
        checkSuperClassMethods(*class1, i, class1->superclass);
    }
}




void checkVarDecList(VarDecl *mainBlockST, int numMainBlockLocals){
  int i;
  int j;
  for(i = 0; i < numMainBlockLocals-1; i++){
    VarDecl *var1 = &mainBlockST[i];
    for(j = i+1; j < numMainBlockLocals; j++){
      VarDecl *var2 = &mainBlockST[j];
      if(strcmp(var1->varName, var2->varName) == 0){
        printf("%s is declared multiple times\n", var1->varName);
        printSemanticError("Var declared multiple times in main method", var1->varNameLineNumber);
      }
    }
  }

  // check var type >= -1
  for(i = 0; i < numMainBlockLocals; i++){
    VarDecl *var1 = &mainBlockST[i];
    if(var1->type < INT_TYPE)
      printSemanticError("Undefined variable", var1->varNameLineNumber);
  }
}



/* ========== Helper Function =========== */

int hasCycle(int cType) {
	ClassDecl *classDecl = &classesST[cType];  
  if(strcmp(classDecl->className, "Object")!=0)
  	while (classDecl->superclass != OBJECT_TYPE) {
  		if (classDecl->superclass == cType)
  			return TRUE;
  		else 
  			classDecl = &classesST[classDecl->superclass];
  	}
	return FALSE;
}

int join(int t1, int t2) {
	if (isSubtype(t1, t2) == TRUE) 
		return t2;
	else if (isSubtype(t2, t1) == TRUE)
		return t1;
	else {
		if (t1 < OBJECT_TYPE) {
			return UNDEFINED_TYPE;
		}
		return join(classesST[t1].superclass, t2);
	}
}

/* Returns nonzero iff sub is a subtype of super */
int isSubtype(int sub, int super) {
	if (sub == NULL_TYPE && (super == NULL_TYPE || super >= OBJECT_TYPE)) {
		return TRUE;
	} else if (sub == INT_TYPE && super == INT_TYPE) {
		return TRUE;
	} else if (sub >= OBJECT_TYPE && super == OBJECT_TYPE) {
		return TRUE;
	} else if (sub > UNDEFINED_TYPE && sub == super) {
		return TRUE;
	} else if (sub >= OBJECT_TYPE) {
		/* Walk on the symbol table */
		ClassDecl *classDecl = &classesST[sub];
		if (classDecl->superclass == super) {
			return TRUE;
		} else if (classDecl->superclass != OBJECT_TYPE) {
			if (hasCycle(sub) == FALSE) {
				return isSubtype(classDecl->superclass, super);
			} else {
				return printSemanticError("Cycle in super class declaration", classDecl->superclassLineNumber);
			}
		}
	}
	return FALSE;
}

/* find the type of funtion parameters */
int typeParams(ASTree *t, int classContainingExprs, int methodContainingExprs, MethodDecl *methodDecl) {
	int count = 0;
	ASTList *childListIterator = t->children;
	while (childListIterator != NULL) {
		if (childListIterator->data != NULL)
			count++;
		childListIterator = childListIterator->next;
	}
	if (count != methodDecl-> numParams) 
		return printSemanticError("Method paramters count mismatch",t->lineNumber);
	if (methodDecl-> numParams == 0) 
		return TRUE;
	ASTList *childListIterator2 = t->children;
	count = 0;
	while (childListIterator2 != NULL) {
		int t1 = typeExpr(childListIterator2->data, classContainingExprs, methodContainingExprs);
		VarDecl *varDecl = &methodDecl->paramST[count];
		if (isSubtype(t1, varDecl->type) == TRUE)
			return TRUE;
		else
			printSemanticError("Method call type mismatch", t->lineNumber);
		childListIterator2 = childListIterator2->next;
		count++;
	}
	return FALSE;
}

void setStaticNums(ASTree *astTree, int classNum, int memberNum) {
	astTree->staticClassNum = classNum;
	astTree->staticMemberNum = memberNum;
}

int findIdTypeByClassAndMethod(ASTree *idAst, int classType, int methodType, ASTree *parentAst) {
	int i;
	if (classType <= OBJECT_TYPE) {
		return ERROR_TYPE;
	}
	if (methodType != ERROR_TYPE) {
		VarDecl *parDeclList = classesST[classType].methodList[methodType].paramST;
		int parNum = classesST[classType].methodList[methodType].numParams;
		for (i = 0; i < parNum; i++) {
			VarDecl *parDecl = &parDeclList[i];
			if (strcmp(parDecl->varName, idAst->idVal) == 0) {
				return parDecl->type;
			}
		}
		VarDecl *varDeclList = classesST[classType].methodList[methodType].localST;
		int varNum = classesST[classType].methodList[methodType].numLocals;
		for (i = 0; i < varNum; i++) {
			VarDecl *varDecl = &varDeclList[i];
			if (strcmp(varDecl->varName, idAst->idVal) == 0) {
				return varDecl->type;
			}
		}
	}
	VarDecl *varDeclList = classesST[classType].varList;
	int varNum = classesST[classType].numVars;
	for (i = 0; i < varNum; i++) {
		VarDecl *varDecl = &varDeclList[i];
		if (strcmp(varDecl->varName, idAst->idVal) == 0) {
			setStaticNums(parentAst, classType, i);
			return varDecl->type;
		}
	}
	return findIdTypeByClassAndMethod(idAst, classesST[classType].superclass, ERROR_TYPE, parentAst);
}

MethodDecl findMethodDeclByClassType(ASTree *idAst, int classType, ASTree *parentAst) {
	int i;
	if (classType <= OBJECT_TYPE) {
		printSemanticError("Undefined method call", idAst->lineNumber);
	}
	for (i = 0; i < classesST[classType].numMethods; i++) {
		MethodDecl *methodDecl = &classesST[classType].methodList[i];
		if (strcmp(methodDecl->methodName, idAst->idVal) == 0) {
			setStaticNums(parentAst, classType, i);
			return *methodDecl;
		}
	}
	return findMethodDeclByClassType(idAst, classesST[classType].superclass, parentAst);
}

int findFieldTypeByClass(int classType, ASTree *idAst, ASTree *parentAst) 
{
	int i;
	if (classType <= OBJECT_TYPE) {
		printSemanticError("Undefined method call", idAst->lineNumber);
	}
	for (i = 0; i < classesST[classType].numVars; i++) {
		VarDecl *varDecl = &classesST[classType].varList[i];
		if (strcmp(varDecl->varName, idAst->idVal) == 0) {
			setStaticNums(parentAst, classType, i);
			return varDecl->type;
		}
	}
	return findFieldTypeByClass(classesST[classType].superclass, idAst, parentAst);
}

/* ====================================== */
/* =========== TYPE CHECK PGM =========== */
/* ====================================== */

void typecheckProgram() {
  /* === Level 3 === */
  checkClasses();
  /* === Level 2 === */
  checkVarDeclList(mainBlockST, numMainBlockLocals);
  /* === Level 1 === */
  /* typecheck the main block expressions */
  typeExprs(mainExprs, -1, -1);
}

/* Returns the type of the expression AST in the given context.
*  Also sets t->staticClassNum and t->staticMemberNum attributes as needed.
*   If classContainingExpr < 0 then this expression is in the main block of
*    the program; otherwise the expression is in the given class.
*     */
int typeExpr(ASTree *t, int classContainingExpr, int methodContainingExpr) {
  
  if (t->typ == EXPR_LIST) {
    // if the current note is an expre_list, then recursively call typeExprs function to check its children
    return typeExprs(t, classContainingExpr, methodContainingExpr);
  } else if (t->typ == INTEGER_EXPR) {
    // level 1
        return INT_TYPE;
  } else if (t->typ == NULL_EXPR) {
    // level 1
      return NULL_TYPE;
  } else if (t->typ == PRINT_EXPR) {
    // level 1
    int t0 = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
    if(t0 != INT_TYPE){
      printSemanticError("Non-int type in PRINT expr", t->lineNumber);
    }
    return INT_TYPE;
  } else if (t->typ == READ_EXPR) {
    // level 1
    return INT_TYPE;
  } else if (t->typ == PLUS_EXPR) {
    // level 1
    int t0 = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
    int t1 = typeExpr(t->children->next->data, classContainingExpr, methodContainingExpr);
    //for plus exp both operands must be int type, print error if not 
    if(t0 != INT_TYPE || t1 != INT_TYPE){
            printSemanticError("Non-int type for PLUS expr", t->lineNumber);
    }
    return INT_TYPE;
  } else if (t->typ == MINUS_EXPR) {
    // level 1
    int t0 = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
    int t1 = typeExpr(t->children->next->data, classContainingExpr, methodContainingExpr);
    //for minus exp both operands must be int type, print error if not 
    if(t0 != INT_TYPE || t1 != INT_TYPE){
            printSemanticError("Non-int type for subtraction", t->lineNumber);
    }
    return INT_TYPE;
  } else if (t->typ == TIMES_EXPR) {
     // level 1
    int t0 = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
    int t1 = typeExpr(t->children->next->data, classContainingExpr, methodContainingExpr);
    //for times exp both operands must be int type, print error if not 
    if(t0 != INT_TYPE || t1 != INT_TYPE){
            printSemanticError("Non-int type for multiplication", t->lineNumber);
    }
    return INT_TYPE;
  } else if (t->typ == GREATER_THAN_EXPR) {
  // level 1
    int t0 = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
    int t1 = typeExpr(t->children->next->data,classContainingExpr, methodContainingExpr);
    //for greater than exp both operands must be int type, print error if not 
    if(t0 != INT_TYPE || t1 != INT_TYPE){
            printSemanticError("Non-int type in GREATER-THAN expr", t->lineNumber);
    }
    return INT_TYPE;
  } else if (t->typ == AND_EXPR) {
    // level 1
    int t0 = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
    int t1 = typeExpr(t->children->next->data, classContainingExpr, methodContainingExpr);
    //for and exp both operands must be int type, print error if not 
    if(t0 != INT_TYPE || t1 != INT_TYPE){
            printSemanticError("Non-int type in AND expr", t->lineNumber);
    }
    return INT_TYPE;
  } else if (t->typ == NOT_EXPR) {
    // level 1
    int t1 = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
    if(t1 != INT_TYPE){
      printSemanticError("Non-int type in NOT expr", t->lineNumber);
    }
    return INT_TYPE;
    
  } else if (t->typ == EQUALITY_EXPR) {
    // level 1
    int t1 = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
    int t2 = typeExpr(t->children->next->data, classContainingExpr, methodContainingExpr);

    if(isSubtype(t1,t2) || isSubtype(t2,t1))
      return INT_TYPE;
  else
       printSemanticError("type mismatch in EQUALITY expr", t->lineNumber);

  } else if (t->typ == IF_THEN_ELSE_EXPR) {
    // level 1
    int t1 = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
    int t2 = typeExpr(t->children->next->data, classContainingExpr, methodContainingExpr);
    int t3 = typeExpr(t->children->next->next->data, classContainingExpr, methodContainingExpr);
    if(t1 == INT_TYPE && join(t2,t3) > UNDEFINED_TYPE){
      return join(t2,t3);
    }
    printSemanticError("Type mismatch in IF-ELSE expr", t->lineNumber);
  } else if (t->typ == WHILE_EXPR) {
    // level 1
    //first child must be INT_TYPE
    int t1 = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
    if(t1 != INT_TYPE)
    printSemanticError("non int type in WHILE BOOLEAN TEST expr", t->lineNumber);

        //second child must be well typed
      typeExprs(t->children->next->data, classContainingExpr, methodContainingExpr);
      
    return INT_TYPE;
    
  } else if (t->typ == ASSIGN_EXPR) {
    // level 2  
    //second child is the subtype of the first 
    int t1 = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
    int t2 = typeExpr(t->children->next->data, classContainingExpr, methodContainingExpr);
    if(isSubtype(t1,t2) == TRUE){
      return t1;
    }else{
    printSemanticError("type mismatch type in ASSIGN expr", t->lineNumber);
    }
    
  } else if (t->typ == ID_EXPR) {
    // level 2  
    //only need to check its child 
    return typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
  } else if (t->typ == AST_ID) {

    //need to check where the ID is declared
    // level 2
    //check where this ID is in mainBlockST
    int i = 0;
    if(classContainingExpr < 0 && methodContainingExpr < 0){
      for(i = 0; i<numMainBlockLocals; i++){
        VarDecl * temValDel = &mainBlockST[i];
        // check if the current node t has the same name as the current element 
      if(strcmp(temValDel->varName, t->idVal) == 0){
        return temValDel->type;
        }
      }
    }
    // level 3
    else{
      //....
      int idType = findIdTypeByClassAndMethod(t, classContainingExpr, methodContainingExpr, t);
			if (idType > UNDEFINED_TYPE) {
				return idType;
			}
    }
    printSemanticError("Undeclared var", t->lineNumber);
  } else if (t->typ == NEW_EXPR) {
    // level 3 
    char *idName = t->children->data->idVal;
    int idType = typeNameToNumber(idName);
    if(idType == NULL_TYPE || idType == UNDEFINED_TYPE){
      printSemanticError("Creating undefined object", t->lineNumber);
    }
    return idType;
  } else if (t->typ == THIS_EXPR) {
    // level 3
     if (methodContainingExpr < 0 && classContainingExpr < 0) {
			printSemanticError("Ill typed this expression", t->lineNumber);
		} else {
			return classContainingExpr;
		}
  } else if (t->typ == DOT_ASSIGN_EXPR) {
    // level 3
     int t1 = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
    if (t1 <= OBJECT_TYPE) {
			printSemanticError("Non-object field access", t->lineNumber);
		}
		if (t->children->next->data->typ != AST_ID) {
			printSemanticError("non-id type in assignment", t->children->next->data->lineNumber);
		}
		int t2 = findFieldTypeByClass(t1, t->children->next->data, t);
		int t3 = typeExpr(t->children->next->next->data, classContainingExpr, methodContainingExpr);
		if (isSubtype(t3, t2) == TRUE) {
			return t2;
		}
		printSemanticError("type mismatch in assignment", t->lineNumber);
  } else if (t->typ == METHOD_CALL_EXPR) {
    // level 3
    MethodDecl methodDecl = findMethodDeclByClassType(t->children->data, classContainingExpr, t);

		if (typeParams(t->children->next->data, classContainingExpr, methodContainingExpr, &methodDecl) == TRUE) {
			return methodDecl.returnType;
		} else {
			printSemanticError("Method call type mismatch", t->lineNumber);
		}
  } else if (t->typ == DOT_METHOD_CALL_EXPR) {
    // level 3
     int t1 = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
		if (t1 <= OBJECT_TYPE) {
			printSemanticError("Non-object field access", t->lineNumber);
		}
		MethodDecl methodDecl = findMethodDeclByClassType(t->children->next->data, t1, t);
		
		if (typeParams(t->children->next->next->data, classContainingExpr, methodContainingExpr, &methodDecl) == TRUE) {
			return methodDecl.returnType;
		} else {
			printSemanticError("Method call type mismatch", t->lineNumber);
		}
  } else if (t->typ == DOT_ID_EXPR) {
    // level 3
    int t1 = typeExpr(t->children->data, classContainingExpr, methodContainingExpr);
		if (t1 <= OBJECT_TYPE) {
			printSemanticError("Non-object field access", t->lineNumber);
		}
		return findFieldTypeByClass(t1, t->children->next->data, t);
  }
  return printSemanticError("Not a valid expression", t->lineNumber);
}






/* Returns the type of the EXPR_LIST AST in the given context. */
int typeExprs(ASTree *t, int classContainingExprs, int methodContainingExprs) {
  int returnType;
  
  ASTList *childListIterator = t->children;
  while (childListIterator != NULL) {
    returnType = typeExpr(childListIterator->data, classContainingExprs, methodContainingExprs);
    childListIterator = childListIterator->next;
  }
  return returnType;
}

int printSemanticError(char *message, int lineNumber) {
  printf("Semantic error on line %d\n%s\n", lineNumber, message);
  exit(0);
}

