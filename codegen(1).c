//level 2

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "codegen.h"
#include "symtbl.h"

#define MAX_DISM_ADDR 65535

/* Global for the DISM output file */
FILE *fout;

/* Global to remember the next unique label number to use */
unsigned int labelNumber = 0;

/* Declare mutually recursive functions (defs and docs appear below) */
void codeGenExpr(ASTree *t, int classNumber, int methodNumber);
void codeGenExprs(ASTree *expList, int classNumber, int methodNumber);
void codeGenArgs(ASTree *expList, int classNumber, int methodNumber);

int findArgOrder(int c, int m, char *varName);

/* Appends code to fout with params */
void appendCode(char *code, ...) {
    va_list arglist;
    char buffer[128];
    va_start(arglist, code);
    vsprintf(buffer, code, arglist);
    va_end(arglist);
    fprintf(fout, "%s\n", buffer);
}

void initPointers() {
    appendCode("mov 7 %d", MAX_DISM_ADDR);
    appendCode("mov 6 %d", MAX_DISM_ADDR);
    appendCode("mov 5 1");
}

/* Print a message and exit under an exceptional condition */
void internalCGerror(char *msg) {
    printf("Code gen error -> %s\n", msg);
    exit(0);
}

/* Generate code that increments the stack pointer */
void incSP() {
    appendCode(";incSP");
    appendCode("mov 1 1");
    appendCode("add 6 6 1");
}

/* Generate code that decrements the stack pointer */
void decSP() {
    appendCode(";decSP");
    appendCode("mov 1 1");
    appendCode("sub 6 6 1");
}

/* Generate code for expressions */
void codeGenExpr(ASTree *t, int classNumber, int methodNumber) {
    if (t->typ == EXPR_LIST) {
        codeGenExprs(t, classNumber, methodNumber);
    } else if (t->typ == INTEGER_EXPR) {
        // level 1
    appendCode("mov 1 %d \t; INTEGER_EXPR, R1 = int_value",t->intVal);
    appendCode("str 6 0 1 \t; mem[SP+0] = R1");
    decSP();
    } else if (t->typ == NULL_EXPR) {
        // level 1
    appendCode("str 6 0 0 \t; mem[SP+0] = R0");
    decSP();
    } else if (t->typ == PRINT_EXPR) {
        // level 1
        //first do the codeGen for the only child
    codeGenExpr(t->children->data, classNumber, methodNumber);
    //then load the result of the child
    appendCode("lod 1 6 1\t; R1 = mem[SP+1]");
    //print the result
    appendCode("ptn 1 \t; print");
    } else if (t->typ == READ_EXPR) {
        // level 1
        //read input by rdn
    appendCode("rdn 1 \t; R1 = input");
    //then store the result to the stack
    appendCode("str 6 0 1 \t; mem[SP+0] = R1");
    //SP-- decrement stack pointer
    decSP();
    } else if (t->typ == PLUS_EXPR) {
        // level 1
        //anazlyze first child, which is the first operand
    codeGenExpr(t->children->data, classNumber, methodNumber);
        //analyze the second child which is the second operand
    codeGenExpr(t->children->next->data, classNumber, methodNumber);

    //load from memory to perform calculation
    appendCode("lod 1 6 2\t; load first child,  R1 = mem[SP+2]");
    appendCode("lod 2 6 1\t; load second child, R2 = mem[SP+1]");
    //after loading perform addition
    appendCode("add 1 1 2 \t; get the result, R1 = R1 + R2");
    //store result back on the stack at SP+2
    appendCode("str 6 2 1 \t; store result to stack, mem[SP+2] = R1");
    //free one memory block
    incSP();
    } else if (t->typ == MINUS_EXPR) {
        // level 1
       //anazlyze first child, which is the first operand
    codeGenExpr(t->children->data, classNumber, methodNumber);
        //analyze the second child which is the second operand
    codeGenExpr(t->children->next->data, classNumber, methodNumber);

      //load from memory to perform calculation
    appendCode("lod 1 6 2\t; load first child,  R1 = mem[SP+2]");
    appendCode("lod 2 6 1\t; load second child, R2 = mem[SP+1]");

      //after loading perform subtraction
    appendCode("sub 1 1 2 \t; get the result, R1 = R1 - R2");
    //store result back on the stack at SP+2
    appendCode("str 6 2 1 \t; store result to stack, mem[SP+2] = R1");
    //free one memory block
    incSP();

    } else if (t->typ == TIMES_EXPR) {
        // level 1
       //anazlyze first child, which is the first operand
    codeGenExpr(t->children->data, classNumber, methodNumber);
        //analyze the second child which is the second operand
    codeGenExpr(t->children->next->data, classNumber, methodNumber);

      //load from memory to perform calculation
    appendCode("lod 1 6 2\t; load first child,  R1 = mem[SP+2]");
    appendCode("lod 2 6 1\t; load second child, R2 = mem[SP+1]");

      //after loading perform multiplication
    appendCode("mul 1 1 2 \t; get the result, R1 = R1 * R2");
    //store result back on the stack at SP+2
    appendCode("str 6 2 1 \t; store result to stack, mem[SP+2] = R1");
    //free one memory block
    incSP();
    } else if (t->typ == GREATER_THAN_EXPR) {
        // level 1
       //anazlyze first child, which is the first operand
    codeGenExpr(t->children->data, classNumber, methodNumber);
      //analyze the second child which is the second operand
    codeGenExpr(t->children->next->data, classNumber, methodNumber);

     //load from memory to perform calculation
    appendCode("lod 1 6 2\t; load first child,  R1 = mem[SP+2]");
    appendCode("lod 2 6 1\t; load second child, R2 = mem[SP+1]");
    //if r1 > r2 jump to a label to store 1 as a result
    appendCode("blt 2 1 #%d \t; if R2 < R1, jump to a label", labelNumber);
    //r1 is not greater than r2 then store zero as a result
    appendCode("str 6 2 0 \t; mem[SP+2] = 0");
    //skip the first case and jump to the end
    appendCode("jmp 0 #%d \t; jump to the end", labelNumber+1);
    //label for r2<r1
    appendCode("#%d: mov 1 1 \t; R1 = 1", labelNumber);
    appendCode("str 6 2 1 \t; mem[SP+2] = R1");
    //end of greater than
    appendCode("#%d: mov 0 0 \t; This is the end of Greater Than",labelNumber+1);
    incSP();
    
    } else if (t->typ == AND_EXPR) {
        // level 1
    appendCode("\t;this is AND_EXPR");
    // if label is created between two codeGenExpr() we have to use a tem_labelnumber
    int localLabelNumber = labelNumber;
    //update labelNumber
    labelNumber += 3;

    //analyze first child which is first operand
    codeGenExpr(t->children->data, classNumber, methodNumber);
    //load result of first child
    appendCode("lod 1 6 1 \t; R1 = mem[SP+1]");
    appendCode("beq 1 0 #%d \t; if R1 == 0, then jump to store 0", localLabelNumber);
    //if r1 !=0, need to check the second child
    //analyze second child which is first operand
    codeGenExpr(t->children->next->data, classNumber, methodNumber);
    //load result of second child
    appendCode("lod 1 6 1 \t; R1 = memp[SP+1]");
    //if r1 == 0 the push zero as a result
    appendCode("beq 1 0 #%d \t; if R1 == 0, then jump to store 0", localLabelNumber);
    appendCode("mov 2 1 \t; R2 = 1 ");
        //Here the second child is not zero and should store one as a result
    appendCode("#%d: str 6 2 2 \t; mem[SP+2] = R2", localLabelNumber + 2);
    
    //SP++, to free one memory block
    incSP();
    //jump to the end
    appendCode("jmp 0 #%d", localLabelNumber+1);
    //here 2nd child is 0 store 0 as result
    appendCode("#%d: str 6 2 0 \t; mem[SP+2] = R0", localLabelNumber+2);
    //SP++, to free one memory block
    incSP();
    //jump to the end
    appendCode("jmp 0 #%d", localLabelNumber+1);
    
        //The first child is zero, should store zero as a result
    appendCode("#%d: str 6 1 0\t; mem[SP+1] = R0",localLabelNumber);
        //jump to the end
        //appendCode("jmp 0 #%d", labelNumber+1);
        //the end of the AND_EXPR
    appendCode("#%d: mov 0 0 \t; end of AND_EXPR", localLabelNumber+1); //labelNumber+1 is the end of AND_EXPR
    } else if (t->typ == NOT_EXPR) {
        // level 1
    //analyze first child which is first operand
    codeGenExpr(t->children->data, classNumber, methodNumber);
    //load child from memory
    appendCode("lod 1 6 1\t; load first child,  R1 = mem[SP+1]");
    //if r1 == r0
    appendCode("beq 1 0 #%d \t; if R1 == R0, then jump to store 0", labelNumber);
    appendCode("#%d: str 6 1 0\t; mem[SP+1] = R0", labelNumber);
    //jump to the end
    appendCode("jmp 0 #%d", labelNumber+1);
     //Here the second child is not zero and should store one as a result
    appendCode("mov 2 1 \t; R2 = 1");
    appendCode("#%d: str 6 1 2 \t; mem[SP+1] = R2 = 1", labelNumber);
    appendCode("#%d: mov 0 0 \t; end of NOT_EXPR", labelNumber+1); //labelNumber+1 is the end of not_expr
     labelNumber += 2;
    } else if (t->typ == EQUALITY_EXPR) {
         // level 1
       //anazlyze first child, which is the first operand
    codeGenExpr(t->children->data, classNumber, methodNumber);
      //analyze the second child which is the second operand
    codeGenExpr(t->children->next->data, classNumber, methodNumber);

     //load from memory to perform calculation
    appendCode("lod 1 6 2\t; load first child,  R1 = mem[SP+2]");
    appendCode("lod 2 6 1\t; load second child, R2 = mem[SP+1]");
    //if r1 == r2 jump to a label to store 1 as a result
    appendCode("beq 2 1 #%d \t; if R2 == R1, jump to a label", labelNumber);
    //r1 is not greater than r2 then store zero as a result
    appendCode("str 6 2 0 \t; mem[SP+2] = 0");
    //skip the first case and jump to the end
    appendCode("jmp 0 #%d \t; jump to the end", labelNumber+1);
    //label for r2 == r1
    appendCode("#%d: mov 1 1 \t; R1 = 1", labelNumber);
    appendCode("str 6 2 1 \t; mem[SP+2] = R1");
    //end of equality
    appendCode("#%d: mov 0 0 \t; This is the end of Greater Than",labelNumber+1);
    incSP();
    } else if (t->typ == IF_THEN_ELSE_EXPR) {
        // level 1
        codeGenExpr(t->children->data, classNumber, methodNumber);
        int localLabelNumber = labelNumber;
        labelNumber += 2;
        appendCode("lod 1 6 1\t; load first child,  R1 = mem[SP+1]");
       appendCode("beq 1 0 #%d \t; if R1 == R0, then jump to store 0", localLabelNumber);
      codeGenExpr(t->children->next->data, classNumber, methodNumber);
       appendCode("lod 2 6 1 \t; R2 = memp[SP+1]");
        appendCode("#%d: str 6 2 2 \t; mem[SP+2] = R2", localLabelNumber);
      appendCode("jmp 0 #%d", localLabelNumber+1);
      codeGenExpr(t->children->next->next->data, classNumber, methodNumber);
       appendCode("lod 2 6 1\t; load first child,  R2 = mem[SP+1]");
      appendCode("#%d: str 6 2 2 \t; mem[SP+2] = R2", localLabelNumber);
      appendCode("#%d: mov 0 0 \t; end of IF_THEN", localLabelNumber+1); //labelNumber+1 is the end of if then else
      incSP();
    } else if (t->typ == WHILE_EXPR) {
        // level 1
     codeGenExpr(t->children->data, classNumber, methodNumber);
     appendCode("lod 1 6 1\t; load first child,  R1 = mem[SP+1]");
    
    } else if (t->typ == ASSIGN_EXPR) {
        // level 2 & 3
        //check the if the current node is from the main function
       if(classNumber < 0 && methodNumber < 0){
      //level 2

         
        //to get the value call codeGen to check the second child
   codeGenExpr(t->children->next->data, classNumber, methodNumber);
      //get the address of the node variable
       for(int i = 0; i < numMainBlockLocals; i++){
            VarDecl *tempVar = &mainBlockST[i];
         
          //if name of node is same as current element then desired variable is located
            if(strcmp(tempVar->varName, t->children->data->idVal)== 0){
          appendCode("mov 1 %d \t; R1 = #localVars", numMainBlockLocals);
          appendCode("mov 2 %d \t; R2 = the index of the desired node", i);
          //figure out distance between desired node and the frame pointer
          appendCode("sub 1 1 2 \t; R1 = distance");
          //the address of p = FP + distance
          appendCode("add 1 1 7 \t; R1 (the address) = R1 + FP");
          break;
        }
       }
        //here r1 is the address of the node
        //assign value to the address
      //load the value of the second child
   appendCode("lod 2 6 1 \t; R2 = mem[SP+1]");
   //store the value to the stack pointer
   appendCode("str 1 0 2 \t; mem[R1 + 0] = R2");
   
     }
     else{
      //level 3
    }
    
    } else if (t->typ == ID_EXPR) {
        // level 2 & 3
    codeGenExpr(t->children->data, classNumber, methodNumber);
    } else if (t->typ == AST_ID) {
        // level 2 & 3
        //check the if the current node is from the main function
    if(classNumber < 0 && methodNumber < 0){
      //level 2
      for(int i = 0; i < numMainBlockLocals; i++){
        VarDecl *tempVar = &mainBlockST[i];
        if(strcmp(tempVar->varName, t->idVal)== 0){
          //if name of node is same as current element then desired variable is located
          appendCode("mov 1 %d \t; R1 = #localVars", numMainBlockLocals);
          appendCode("mov 2 %d \t; R2 = the index of the desired node", i);
          //figure out distance between desired node and the frame pointer
          appendCode("sub 1 1 2 \t; R1 = distance");
          //the address of p = FP + distance
          appendCode("add 1 1 7 \t; R1 (the address) = R1 + FP");
          //load the variable into R1
          appendCode("lod 1 1 0 \t; R1 = the value of desired var");
          //store it to the stack
          appendCode("str 6 0 1 \t; mem[SP+0] = R1");
          decSP();
          break;
        }
      }
    }
    else{
      //level 3
      
    }
    } else if (t->typ == THIS_EXPR) {
        // level 3
    } else if (t->typ == DOT_ID_EXPR) {
        // level 3
    } else if (t->typ == DOT_ASSIGN_EXPR) {
        // level 3
    } else if (t->typ == NEW_EXPR) {
        // level 3
    } else if (t->typ == DOT_METHOD_CALL_EXPR) {
        // level 3
    } else if (t->typ == METHOD_CALL_EXPR) {
        // level 3
    } else if (t->typ == ARG_LIST) {
        // level 3
    }
}

/* Generate code for expression-list*/
void codeGenExprs(ASTree *expList, int classNumber, int methodNumber) {
    ASTList *childListIterator = expList->children;
    while (childListIterator != NULL) {
        codeGenExpr(childListIterator->data, classNumber, methodNumber);
        childListIterator = childListIterator->next;
        if (childListIterator != NULL) {
            incSP();
        }
    }
}

/* Generate DISM code as the prologue to the given method or main
 *  block. If classNumber < 0 then methodNumber may be anything and we
 *   assume we are generating code for the program's main block. */
void genPrologue(int classNumber, int methodNumber) {
    initPointers();
    if (classNumber < 0) {
        if (numMainBlockLocals > 0) {
            appendCode("mov 1 %d", numMainBlockLocals);
            appendCode("sub 7 7 1");
            appendCode("mov 1 1");
            appendCode("sub 6 7 1");
        }
    }
}

/* Generate DISM code as the epilogue to the given method or main
 *  block. If classNumber < 0 then methodNumber may be anything and we
 *   assume we are generating code for the program's main block. */
void genEpilogue(int classNumber, int methodNumber) {
    if (classNumber < 0) {
        /* Terminate the program */
        appendCode(";Normal termination at the end of the main block");
        appendCode("hlt 0");
    }
}


void generateDISM(FILE *outputFile) {
    fout = outputFile;
    genPrologue(-1, -1);
    codeGenExprs(mainExprs, -1, -1);
    genEpilogue(-1, -1);
    fclose(fout);
}

