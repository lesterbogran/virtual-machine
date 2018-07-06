//
// Created by przb86 on 22.04.18.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "bigint.h"
#include "support.h"

void print_stack_state();
ObjRef create_compound_object(int n);

#define HALT 0
#define PUSHC 1
#define ADD 2
#define SUB 3
#define MUL 4
#define DIV 5
#define MOD 6

#define RDINT 7
#define WRINT 8
#define RDCHR 9
#define WRCHR 10
#define PUSHG 11
#define POPG 12
#define ASF 13
#define RSF 14
#define PUSHL 15
#define POPL 16

#define EQ 17
#define NE 18
#define LT 19
#define LE 20
#define GT 21
#define GE 22

#define JMP 23
#define BRF 24
#define BRT 25

#define CALL 26
#define RET 27
#define DROP 28
#define PUSHR 29
#define POPR 30

#define DUP 31

#define NEW 32
#define GETF 33
#define PUTF 34
#define NEWA 35
#define GETFA 36
#define PUTFA 37
#define GETSZ 38
#define PUSHN 39
#define REFEQ 40
#define REFNE 41

#define IMMEDIATE(x) ((x)&0x00FFFFFF)
#define SIGN_EXTEND(i) ((i) & 0x00800000 ? (i) | 0xFF000000 : (i))

#define MSB (1 << (8 * sizeof(unsigned int) - 1))
#define IS_PRIM(objRef) (((objRef)->size & MSB) == 0)
#define GET_SIZE(objRef) ((objRef)->size & ~MSB)
#define GET_REFS(objRef) ( (ObjRef *)(objRef)->data)

#define VERSION 8
#define STACK_SIZE 10000
#define MYNULL 0

typedef struct {
    bool isObjRef;
    union{
        ObjRef objRef;
        int number;
    } u;
} StackSlot;

/**
 * Creating version 5 stack with elements from heap
 */
StackSlot int_stack_slot[STACK_SIZE];

const char format_ids [4] = {'N', 'J', 'B', 'F'};

/** Normal stack */
int *int_stack;
/** next free place*/
int int_pos = 0;
/** result of calculation*/
int int_result = 0;
/** frame pointer */
int fp = 0;
/** normal stack size */
int const int_stack_size = 10000;

/** return register size */
int const return_register_size = 1000;
/** return register position */
int return_register_pos = 0;
/** return register */
StackSlot *return_register;

/** array with instructions for VM*/
unsigned int *program;
/** number of instructions in program*/
int instruction_number = 0;

/** array with global variables */
int *global_stack;


/////////CHANGING GLOBAL STACK
/** global stack pointer */
int global_stack_pointer = 0;
/** global stack size*/
int global_stack_size = 0;
/** created global stack for objects */
StackSlot *global_stack_slot;

/** debug mode flag, 0 by default */
int debug_mode = 0;
/** breakpoint mode */
int breakpoint_mode = 0;
/** instruction number breakpoint */
int breakpoint_instruction_number = -1;

/** Program counter */
int ProgramCounter = 0;

/**
 * Stopping VM
 */
void vm_stop(){
    printf("Ninja Virtual Machine stopped\n");
    exit(1);
}

/*
 * This routine is called in case a fatal error has occurred.
 * It should print the error message and terminate the program.
 */
void fatalError(char *msg) {
    printf("Error: %s\n", msg);
    exit(1);
}

ObjRef newPrimObject(int dataSize) {
    ObjRef objRef;

    objRef = malloc(sizeof(unsigned int) +
                    dataSize * sizeof(unsigned char));
    if (objRef == MYNULL) {
        fatalError("newPrimObject() got no memory");
    }
    objRef->size = dataSize;
    return objRef;
}

ObjRef newCompoundObject(int dataSize){
    ObjRef objRef = malloc(sizeof(unsigned int) + 
                    sizeof(ObjRef) * dataSize);

    for(int i = 0; i < dataSize; i++){
        GET_REFS(objRef)[i] = MYNULL;
    }
    objRef->size = dataSize | MSB;

    return objRef;
}

StackSlot createStackSlot(int value, bool is_ref){
    StackSlot stackSlot;

    if(is_ref){
        ObjRef objRef = malloc(sizeof(unsigned int) + sizeof(int));
        objRef -> size = sizeof(int);
        *(int *) objRef -> data = value;
        stackSlot.isObjRef = true;
        stackSlot.u.objRef = objRef;
    }else{
        stackSlot.isObjRef = false;
        stackSlot.u.number = value;
    }
    return stackSlot;
}

/**
 * Closing file stream
 * @param file
 */
void close_file(FILE *file){
    fclose(file);
}

/**
 * Checking VM identifiers number
 * @param file
 */
void identifiers_format_checking(FILE *file){
    char c;
    int pos = 0;

    while (pos < 4){
        c = fgetc(file);

        if(format_ids[pos] != c){
            printf("ERROR: format identifiers are false\n");
            close_file(file);
            vm_stop();
        }
        pos++;
    }
}

/**
 * Check VM version
 * @param file
 */
void version_checking(FILE *file){
    unsigned int version;

    fread(&version, 4, 1, file);
    if(version != VERSION){
        printf("Error: file has wrong version number \n");
        close_file(file);
        exit(1);
    }
}

/**
 * Reading number of instructions for *program
 * @param file
 */
void instructions_number_check(FILE * file){
    unsigned int instructions;

    fread(&instructions, 4, 1, file);
    if(instructions < 0){
        printf("ERROR: number of instruction is incorrect %d", instructions);
        close_file(file);
        vm_stop();
    }
    //allocate memory for instruction
    program = malloc(instructions * sizeof(int));
    instruction_number = instructions;
}

/**
 * Reading instructions from bin file
 * @param file
 */
void read_instructions(FILE *file){
    int i = 0;
    int instructions_number = 0;

    int read_value;

    while (i < instruction_number){
        fread(&read_value, 4, 1, file);
        program[instructions_number] = read_value;
        instructions_number++;
        i++;
    }
}

/**
 * Creating global stack
 */
void create_global_stack(){
    global_stack_slot = malloc(global_stack_size * sizeof(int));
}

/**
 * Checking number of global variables
 * @param file
 */
void global_variables_check(FILE * file){
    unsigned int globals;

    fread(&globals, 4, 1, file);
    if(globals < 0){
        printf("ERROR: number of global variables is incorrect %d", globals);
        close_file(file);
        vm_stop();
    }
    global_stack_size = globals;
    create_global_stack();
}
/**
 * Allocating memory for return register
 */
void create_return_register(){
    return_register = malloc(return_register_size * sizeof(int));
}

/**
 * Opens file
 * @param file_name
 * @return
 */
int open_file(char * file_name){
    FILE *file;

    file = fopen(file_name, "r");

    if(file != MYNULL){
        identifiers_format_checking(file);
        version_checking(file);
        instructions_number_check(file);
        global_variables_check(file);
        read_instructions(file);
        //create_stack(); dont need, because is created
        create_return_register();
    }else{
        printf("Error: no code file specified\n");
        exit(1);
    }
    printf("Ninja Virtual Machine started\n");
    return 0;
}

void stack_overflow(){
    printf("Error: stack overflow\n");
    vm_stop();
}

void empty_stack(){
    printf("Error. Stack underflow!\n");
    vm_stop();
}

void push(StackSlot stackSlot){
    if(int_pos < STACK_SIZE){
        int_stack_slot[int_pos++] = stackSlot;
    } else{
        stack_overflow();
    }
}

StackSlot pop(){
    if(int_pos < 0){
        StackSlot slot;
        empty_stack();
        return slot;
    } else {
        StackSlot stack_var = int_stack_slot[int_pos - 1];
        int_pos--;
        return stack_var;
    }
}

/**
 * Pushing global variable onto stack
 * @param position of the variable in stack
 */
void pushg(int position){
    if(position >= 0 && global_stack_size > position){
        push(global_stack_slot[position]);
    }else{
        printf("GlobalStackOutOfBoundsError\n");
        vm_stop();
    }
}

/**
 * Pushing the last stack element onto global stack
 * @param position on the global stack
 */
void popg(int position){
    if(position >= 0 && global_stack_size > position){
        StackSlot elem = pop();
        global_stack_slot[position] = elem;
    }else{
        printf("GlobalStackOutOfBoundsError\n");
        vm_stop();
    }
}

/**
 * push local variable
 * @param n
 */
void push_local(int n){
    StackSlot to_push = int_stack_slot[fp + n];
    if(to_push.isObjRef){
        push(to_push);
    }else{
        fatalError("PUSHL detected number in local or parameter variable");
    }
}

/**
 * pop up first stack element to n position in local stack
 * @return
 */
void pop_local(int n){
    StackSlot poped = pop();

    int_stack_slot[fp + n] = poped;
}

/**
 * allocate stack frame
 * @param n
 */
void asf(int n){
    if(int_pos == 0){
        push(createStackSlot(fp, false));
        fp++;
    }else {
        push(createStackSlot(fp, false));
        fp = int_pos;
    }
    int_pos += n;
}

/**
 * release stack frame
 */
void rsf(){
    int_pos = fp;
    StackSlot fp_slot = pop();
    if(fp_slot.isObjRef){
        fp = *(int *)fp_slot.u.objRef->data;
    }else{
        fp = fp_slot.u.number;
    }
}


/**
 * pushing top of stack onto return register
 */
void popr(){
    StackSlot to_push = pop();

    if(return_register_pos < return_register_size){
        return_register[return_register_pos++] = to_push;
    }else{
        printf("RuntimeError: reached maximum recursion depth\n");
        vm_stop();
    }
}

/**
 * poping adress from the return register onto top of stack
 */
void pushr(){
    if(return_register_pos > 0){
        StackSlot to_push = return_register[return_register_pos - 1];

        push(to_push);
    } else{
        printf("RuntimeError: return register is empty\n");
        vm_stop();
    }
}

ObjRef create_compound_object(int n){
    ObjRef objRef = malloc(sizeof(unsigned int) + sizeof(int) * n); //first byte 
    objRef->size = n | MSB;

    for(int i = 0; i < n; i++){
        GET_REFS(objRef)[i] = MYNULL;
    }
    return objRef;
}

void print_command(unsigned int IR){
    unsigned int i = IR >> 24;

    if(i == PUSHC){
        printf("pushc %d\n", SIGN_EXTEND(IMMEDIATE(IR)));
    }else if(i == ADD){
        printf("add\n");
    }else if(i == SUB){
        printf("sub\n");
    }else if(i == MUL){
        printf("mul\n");
    }else if(i == WRINT){
        printf("wrint\n");
    }else if(i == WRCHR){
        printf("wrchr\n");
    }else if(i == RDINT) {
        printf("rdint \n");
    }else if(i == RDCHR){
        printf("rdchr \n");
    }else if(i == DIV){
        printf("div \n");
    }else if(i == MOD){
        printf("mod \n");
    }else if(i == HALT){
        printf("halt\n");
    }else if(i == ASF){
        printf("asf %d\n", SIGN_EXTEND(IMMEDIATE(IR)));
    }else if(i == RSF){
        printf("rsf\n");
    }else if(i == POPL){
        printf("popl %d\n", SIGN_EXTEND(IMMEDIATE(IR)));
    }else if(i == PUSHL){
        printf("pushl %d\n", SIGN_EXTEND(IMMEDIATE(IR)));
    }else if(i == POPG){
        printf("popg %d\n", SIGN_EXTEND(IMMEDIATE(IR)));
    }else if(i == PUSHG){
        printf("pushg %d\n", SIGN_EXTEND(IMMEDIATE(IR)));
    }else if(i == EQ){
        printf("eq\n");
    }else if(i == NE){
        printf("ne\n");
    }else if(i == LT){
        printf("lt\n");
    }else if(i == LE){
        printf("le\n");
    }else if(i == GT){
        printf("gt\n");
    }else if(i == GE){
        printf("ge\n");
    }else if(i == JMP){
        printf("jmp %d\n", SIGN_EXTEND(IMMEDIATE(IR)));
    }else if(i == BRF){
        printf("brf %d\n", SIGN_EXTEND(IMMEDIATE(IR)));
    }else if(i == BRT){
        printf("brt %d\n", SIGN_EXTEND(IMMEDIATE(IR)));
    }else if(i == CALL){
        printf("call %d\n", SIGN_EXTEND(IMMEDIATE(IR)));
    }else if(i == RET){
        printf("ret \n");
    }else if(i == DROP){
        printf("drop %d\n", SIGN_EXTEND(IMMEDIATE(IR)));
    }else if(i == PUSHR){
        printf("pushr \n");
    }else if(i == POPR){
        printf("popr \n");
    }else if(i == DUP){
        printf("dup \n");
    }else if(i == NEW){ // from here is the 7th version
        printf("new %d\n", SIGN_EXTEND(IMMEDIATE(IR)));
    }else if(i == GETF){
        printf("getf %d\n", SIGN_EXTEND(IMMEDIATE(IR)));
    }else if(i == PUTF){
        printf("putf %d\n", SIGN_EXTEND(IMMEDIATE(IR)));
    }else if(i == NEWA){
        printf("newa \n");
    }else if(i == GETFA){
        printf("getfa \n");
    }else if(i == PUTFA){
        printf("putfa \n");
    }else if(i == GETSZ){
        printf("getsz \n");
    }else if(i == PUSHN){
        printf("pushn \n");
    }else if(i == REFEQ){
        printf("refeq \n");
    }else if(i == REFNE){
        printf("refne \n");
    }
}

/** jumping to this position */
void jump(int pos){
    ProgramCounter = pos - 1;
}

/** function removes n last elements from stack */
void drop(int n){
    int i = 0;

    while (i < n){
        pop();
        i++;
    }
}

/** duplicates top of stack */
void duplicate(){
    StackSlot to_duplicate = pop();

    push(to_duplicate);
    push(to_duplicate);
}

int get_int_from_ref_slot(StackSlot stackSlot){
    return * (int *)stackSlot.u.objRef->data;
}

StackSlot create_stack_slot(ObjRef objRef){
    StackSlot stackSlot;
    stackSlot.isObjRef = true;
    stackSlot.u.objRef = objRef;
    return stackSlot;
}

/** before each operation must be used this function */
void setOp(){
    StackSlot f_elem = pop();
    StackSlot s_elem = pop();

    bip.op1 = s_elem.u.objRef;
    bip.op2 = f_elem.u.objRef;
}

void exec(unsigned int IR){
    unsigned int i = IR >> 24;

    if(i == PUSHC){
        if(int_pos < int_stack_size) {
            int to_push = SIGN_EXTEND(IMMEDIATE(IR));
            bigFromInt(to_push);
            StackSlot stackSlot;
            stackSlot.isObjRef = true;
            stackSlot.u.objRef = bip.res;
            push(stackSlot);
        }else{
            printf("Error: stack overflow\nhalt\n");
            printf("Ninja Virtual Machine stopped\n");
            exit(1);
        }
    }else if(i == ADD){
        setOp();
        bigAdd();
        StackSlot stackSlot = create_stack_slot(bip.res);
        push(stackSlot);
    }else if(i == ASF){
        int n = IMMEDIATE(IR);
        asf(n);
    }else if(i == RSF){
        rsf();
    }else if(i == POPL){
        pop_local(SIGN_EXTEND(IMMEDIATE(IR)));
    }else if(i == PUSHL){
        push_local(SIGN_EXTEND(IMMEDIATE(IR)));
    }else if(i == SUB){
        setOp();
        bigSub();
        StackSlot stackSlot = create_stack_slot(bip.res);
        push(stackSlot);
    }else if(i == MUL){
        setOp();
        bigMul();
        StackSlot stackSlot = create_stack_slot(bip.res);
        push(stackSlot);
    }else if(i == WRINT){
        StackSlot poped = pop();
        if(poped.isObjRef){
            bip.op1 = poped.u.objRef;
            bigPrint(stdout);
        }else{
            printf("%p", (void *)(poped.u.objRef));
        }
    }else if(i == WRCHR){
        StackSlot poped = pop();
        bip.op1 = poped.u.objRef;
        int to_print = bigToInt();
        printf("%c", to_print);
    }else if(i == POPG){
        int position = IMMEDIATE(IR);
        StackSlot to_push = pop();

        if(position >= 0 && position < global_stack_size){
            global_stack_slot[position] = to_push;
            global_stack_pointer++;
        }else{
            printf("GlobalStackOutOfBoundsError\n");
            vm_stop();
        }
    }else if(i == PUSHG){
        if(int_pos < STACK_SIZE) {
            int to_push = IMMEDIATE(IR);
            push(global_stack_slot[to_push]);
        }else{
            printf("StackOutOfBoundsError\n");
            vm_stop();
        }
    }else if(i == RDINT) {
        bigRead(stdin);
        StackSlot stackSlot;
        stackSlot.isObjRef = true;
        stackSlot.u.objRef = bip.res;

        push(stackSlot);
    }else if(i == RDCHR){
        char i;
        scanf("%c", &i);
        push(createStackSlot(i, true));
    }else if(i == DIV){
        setOp();
        bigDiv();
        StackSlot stackSlot = create_stack_slot(bip.res);
        push(stackSlot);
    }else if(i == MOD){
        setOp();
        bigDiv();
        StackSlot stackSlot = create_stack_slot(bip.rem);
        push(stackSlot);
    }else if(i == EQ){
        setOp();
        if(bigCmp() == 0){
            bigFromInt(1);
        }else{
            bigFromInt(0);
        }
        StackSlot stackSlot = create_stack_slot(bip.res);
        push(stackSlot);
    }else if(i == NE){
        setOp();
        if(bigCmp() != 0){
            bigFromInt(1);
        }else{
            bigFromInt(0);
        }
        StackSlot stackSlot = create_stack_slot(bip.res);
        push(stackSlot);
    }else if(i == LT){
        setOp();
        if(bigCmp() < 0){
            bigFromInt(1);
        }else{
            bigFromInt(0);
        }
        StackSlot stackSlot = create_stack_slot(bip.res);
        push(stackSlot);
    }else if(i == LE){
        setOp();
        int cmp = bigCmp();
        if(cmp < 0 || cmp == 0){
            bigFromInt(1);
        }else{
            bigFromInt(0);
        }
        StackSlot stackSlot = create_stack_slot(bip.res);
        push(stackSlot);
    }else if(i == GT){
        setOp();
        if(bigCmp() > 0){
            bigFromInt(1);
        }else{
            bigFromInt(0);
        }
        StackSlot stackSlot = create_stack_slot(bip.res);
        push(stackSlot);
    }else if(i == GE){
        setOp();
        int cmp = bigCmp();
        if(cmp > 0 || cmp == 0){
            bigFromInt(1);
        }else{
            bigFromInt(0);
        }
        StackSlot stackSlot = create_stack_slot(bip.res);
        push(stackSlot);
    }else if(i == BRF){
        StackSlot poped = pop();

        bip.op1 = poped.u.objRef;
        bigFromInt(0);
        bip.op2 = poped.u.objRef;
        bip.op1 = bip.res;
        if(bigCmp() == 0){
            jump(IMMEDIATE(IR));
        }
    }else if(i == BRT){
        StackSlot poped = pop();

        bip.op1 = poped.u.objRef;
        bigFromInt(1);
        bip.op1 = bip.res;
        bip.op2 = poped.u.objRef;
        if(bigCmp() == 0){
            jump(IMMEDIATE(IR));
        }
    }else if(i == JMP){
        jump(IMMEDIATE(IR));
    }else if(i == CALL){
        int next_instruction = ProgramCounter + 1;
        //pushing next instruction on stack
        push(createStackSlot(next_instruction, true));
        //jump to the function
        jump(IMMEDIATE(IR));
    }else if(i == RET){
        //loading adress from return register onto to of stack
        StackSlot stackSlot = pop();
        int next_instruction = get_int_from_ref_slot(stackSlot);
        jump(next_instruction);
    }else if(i == DROP){
        int to_drop = IMMEDIATE(IR);
        drop(to_drop);
    }else if(i == PUSHR){
        pushr();
    }else if(i == POPR){
        popr();
    }else if(i == DUP){
        duplicate();
    }else if(i == NEW){ // from here is the 7th version
        StackSlot stackSlot;
        stackSlot.isObjRef = true;
        stackSlot.u.objRef = newCompoundObject(SIGN_EXTEND(IMMEDIATE(IR)));
        push(stackSlot);
    }else if(i == GETF){// pushing value from the object onto stack
        StackSlot slotToPush;
        slotToPush.u.objRef = GET_REFS(pop().u.objRef)[IMMEDIATE(IR)];
        slotToPush.isObjRef = true;
        push(slotToPush);
    }else if(i == PUTF){// pushing value from stack to object     
        StackSlot toPushSlot = pop();    
        GET_REFS(pop().u.objRef)[IMMEDIATE(IR)] = toPushSlot.u.objRef;  
    }else if(i == NEWA){ // creating new array and pop size from stack
        StackSlot poped = pop();
        bip.op1 = poped.u.objRef;
        int arr_size = bigToInt();

        StackSlot stackSlot;
        stackSlot.isObjRef = true;
        stackSlot.u.objRef = newCompoundObject(arr_size);
        push(stackSlot);
    }else if(i == GETFA){ // pushing value from the object onto stack and take place value from pushc
        StackSlot funnySlot = pop();
        bip.op1 = funnySlot.u.objRef;
        int place_from_push = bigToInt();
        StackSlot slot;
        slot.isObjRef = true;  
        StackSlot lolSlot = pop(); 
        slot.u.objRef = GET_REFS(lolSlot.u.objRef)[place_from_push];
        push(slot);
    }else if(i == PUTFA){ // pushing value from stack to object at place from pushc 
        StackSlot num_to_push = pop();
        bip.op1 = pop().u.objRef;
        int place_from_push = bigToInt();
        StackSlot arr_slot = pop();   
        GET_REFS(arr_slot.u.objRef)[place_from_push] = num_to_push.u.objRef;  
    }else if(i == GETSZ){
        StackSlot slot = pop();
        if(slot.u.objRef != MYNULL){
            int size = GET_SIZE(slot.u.objRef);
            bigFromInt(size);
            StackSlot slot;
            slot.isObjRef = true;
            slot.u.objRef = bip.res;
            push(slot);
        }else{
            fatalError("stack underflow");
        }
    }else if(i == PUSHN){
        StackSlot nilSlot;
        nilSlot.isObjRef = true;
        nilSlot.u.objRef = MYNULL;
        push(nilSlot);
    }else if(i == REFEQ){
        StackSlot ref_one = pop();
        StackSlot ref_two = pop();
        StackSlot slot;
        slot.isObjRef = true;
        if(ref_one.u.objRef == ref_two.u.objRef){
            bigFromInt(1);
        }else{
            bigFromInt(0);
        }
        slot.u.objRef = bip.res;
        push(slot);
    }else if(i == REFNE){
        StackSlot ref_one = pop();
        StackSlot ref_two = pop();
        StackSlot slot;
        slot.isObjRef = true;
        if(ref_one.u.objRef != ref_two.u.objRef){
            bigFromInt(1);
        }else{
            bigFromInt(0);
        }
        slot.u.objRef = bip.res;
        push(slot);
    }else if(i == HALT){
    }
}

void print_prog(){
    int PC = 0;
    unsigned int IR = 1;
    int size = instruction_number;

    while (PC < size){
        IR = program[PC];
        printf("%0*d", 3 - PC/10 , 0);
        printf("%d:\t",PC);
        print_command(IR);

        PC += 1;
    }
}

/** prints values of the global stack */
void print_global_stack(){
    int i = 0;
    while (i < global_stack_size){
        printf("data[%0*d", (2 - i / 10), 0);
        printf("%d]: \t", i);
        if(global_stack_slot[i].isObjRef != MYNULL){
            printf("(objref)");
            printf(" %p \n", (void*)global_stack_slot[i].u.objRef);
        }else{
            printf("(nil)\n");
        }
        i++;
    }
    printf("      --- end of data ---\n");
}

void print_stack_slot(StackSlot stackSlot, int SP){
    if(stackSlot.isObjRef){
        printf("%d:\t (objref) %p \n", SP, (void*)(stackSlot.u.objRef));
    }else{
        printf("%d:\t (number) %d \n", SP, stackSlot.u.number);
    }
}
/** fucking foobar function, two fucking hours debugging. Don't even touch this shit, dude :D */
void print_stack_state(){
    int SP = int_pos;
    int FP = fp;

    if(SP != 0 || FP != 0) {
        printf("sp         ---> ");
        printf("%0*d", (2 - SP / 10), 0);
        printf("%d:\t xxxx \n", SP);
        while (SP > FP + 1) {
            SP--;
            printf("        \t%0*d", (2 - SP / 10), 0);
            print_stack_slot(int_stack_slot[SP], SP);
        }
        printf("fp         ---> ");
        printf("%0*d", (2 - FP / 10), 0);
        print_stack_slot(int_stack_slot[FP], FP);

        while (FP > 0) {
            FP--;
            printf("        \t%0*d", (2 - FP / 10), 0);
            print_stack_slot(int_stack_slot[FP], FP);
        }
    }else{
        printf("sp, fp --->     000:   xxxx\n");
    }
    printf("    --- bottom of stack ---\n");
}

/** this function finds pointer in stacks and prints value of this pointer */
void find_pointer_in_stacks(ObjRef *obj){
    for(int i = 0; i < global_stack_pointer; i++){
        if((ObjRef *)(global_stack_slot[i].u.objRef) == obj){
           if(!IS_PRIM(global_stack_slot[i].u.objRef)){
                printf("<compound object>\n");
                int pos = GET_SIZE(global_stack_slot[i].u.objRef);
                for(int i = 0; i < pos; i++){
                    ObjRef my_ref = GET_REFS(global_stack_slot[i].u.objRef)[i];
                    printf("pos: %d\n", i);
                    printf("field[%04d]:\t = (objref) %p\n", i, (void *)my_ref);
                }
                printf("\t--- end of object ---\n");
            }else{
                printf("\t<primitive object>\nvalue:\t ");
                bip.op1 = global_stack_slot[i].u.objRef;
                bigPrint(stdout);
                printf("\n");
                return;
            }
        }
    }

    for(int i = 0; i < int_pos; i++){
         if((ObjRef *)(int_stack_slot[i].u.objRef) == obj){
           if(!IS_PRIM(int_stack_slot[i].u.objRef)){
                printf("<compound object>\n");
                int pos = GET_SIZE(int_stack_slot[i].u.objRef);
                
                for(int i = 0; i < pos; i++){
                    ObjRef my_ref = GET_REFS(int_stack_slot[i].u.objRef)[i];
                    printf("pos: %d\n", i);
                    printf("field[%04d]:\t = (objref) %p\n", i, (void *)my_ref);

                }
                printf("\t--- end of object ---\n");
            }else{
                printf("\t<primitive object>\nvalue:\t ");
                bip.op1 = int_stack_slot[i].u.objRef;
                bigPrint(stdout);
                printf("\n");
                return;
            }
        }
    }
}

void exec_prog(){
    int size = instruction_number;
    ProgramCounter = 0;
    unsigned int IR = -1;
    while (IR != HALT && ProgramCounter < size){
        IR = program[ProgramCounter];

        if(debug_mode != 0){
            printf("%0*d", (2 - ProgramCounter / 10), 0);
            printf("%d:\t",ProgramCounter);
            print_command(IR);
        }

        if(debug_mode == 0 && ProgramCounter == breakpoint_instruction_number){
            debug_mode = 1;
        }

        if(debug_mode == 1){
            printf("DEBUG: inspect, list, breakpoint, step, run, quit?\n");
            char input[20];
            do{
                scanf("%s", input);
                if(strcmp("\n", input) == 0){
                    ProgramCounter--;
                    break;
                }
                if (strcmp("inspect", input) == 0){
                    printf("DEBUG [inspect]: stack, data, object?\n");
                    scanf("%s", input);
                    if(strcmp("data", input) == 0){
                        print_global_stack();
                        ProgramCounter--;
                        break;
                    }else if(strcmp("stack", input) == 0){
                        print_stack_state();
                        ProgramCounter--;
                        break;
                    }else if(strcmp("object", input) == 0){
                        ObjRef *pointer;
                        printf("object reference?\n");
                        scanf("%p", (void **)(&pointer));

                        find_pointer_in_stacks(pointer);
                        ProgramCounter--;
                        break;
                    }else{
                        ProgramCounter--;
                        break;
                    }

                }else if(strcmp("list", input) == 0){
                    print_prog();
                    printf("        --- end of code ---\n");
                    ProgramCounter--;
                    break;
                }else if(strcmp("breakpoint", input) == 0){
                    int instruction = -1;

                    printf("DEBUG [breakpoint]: cleared\nDEBUG [breakpoint]: address to set, -1 to clear, <ret> for no change?\n");
                    scanf("%d", &instruction);

                    breakpoint_instruction_number = instruction;
                    printf("DEBUG [breakpoint]: cleared\n");

                    if(breakpoint_instruction_number > 0) {
                        printf("DEBUG [breakpoint]: now set at %d\n", breakpoint_instruction_number);
                        breakpoint_mode = 1;
                    }
                    ProgramCounter--;
                    break;
                }else if(strcmp("step", input) == 0){
                    exec(IR);
                    break;
                }else if(strcmp("run", input) == 0){

                    if(breakpoint_mode == 1){
                        printf("%0*d", (2 - breakpoint_instruction_number / 10), 0);
                        printf("%d:\t",breakpoint_instruction_number);
                        print_command(program[breakpoint_instruction_number]);
                        breakpoint_mode = 0;
                    }

                    debug_mode = 0;
                    exec(IR);
                    break;
                }else if(strcmp("quit", input) == 0) {
                    vm_stop();
                    break;
                }
            }while (1);
        }else{
            exec(IR);
        }
        ProgramCounter += 1;
    }
}

/** debugging */
void debugging(char *file_name){
    debug_mode = 1;
    open_file(file_name);
    printf("DEBUG: file \'%s\' loaded (code size = %d, data size = %d)\n",
           file_name, instruction_number, global_stack_size);

    exec_prog();
}

int main(int argc, char *argv[]) {
    if (argv[1] == MYNULL) {
        printf("Error: no code file specified\n");
    }else{
        if (strcmp("--help", argv[1]) == 0) {
            printf("usage: ./njvm [options] <code file>\n"
                   "--version        show version and exit\n"
                   "--help           show this help and exit\n"
                   "--debug          start virtual machine in debug mode\n");
        }else if(strcmp("--version", argv[1]) == 0){
            printf("Ninja Virtual Machine version %d \n", VERSION);
        }else if(strcmp("--debug", argv[1]) == 0){
            if(argv[2] != MYNULL){
                debugging(argv[2]);
                printf("Ninja Virtual Machine stopped\n");
            }else{
                printf("ERROR: Cannot open file\n");
            }
        }else{
            if(argv[1] != MYNULL){
                open_file(argv[1]);
                exec_prog();
                printf("Ninja Virtual Machine stopped\n");
            }else{
                printf("ERROR: Cannot open file\n");
            }
        }
    }
    return 0;
}

