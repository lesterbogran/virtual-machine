//
// Created by przb86 on 22.04.18.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mach/machine.h>
#include <stdbool.h>
#include "bigint.h"
#include "support.h"

void print_stack_state();

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

#define IMMEDIATE(x) ((x)&0x00FFFFFF)
#define SIGN_EXTEND(i) ((i) & 0x00800000 ? (i) | 0xFF000000 : (i))

#define VERSION 6
#define STACK_SIZE 10000
#define NULL 0

//typedef struct{
//    unsigned int size;
//    unsigned char data[1];
//} *ObjRef;

typedef struct {
    boolean_t isObjRef;
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
int const return_register_size = 100;
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
    printf("Fatal error: %s\n", msg);
    exit(1);
}

ObjRef newPrimObject(int dataSize) {
    ObjRef objRef;

    objRef = malloc(sizeof(unsigned int) +
                    dataSize * sizeof(unsigned char));
    if (objRef == NULL) {
        fatalError("newPrimObject() got no memory");
    }
    objRef->size = dataSize;
    return objRef;
}

StackSlot createStackSlot(int value, boolean_t is_ref){
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
 * Allocating memory for local stack
 */
//void create_stack(){
//    int_stack = malloc(int_stack_size * sizeof(int));
//}

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

    if(file != NULL){
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
    printf("Error. Stack is full!\n");
    vm_stop();
}

void empty_stack(){
    printf("Error. Stack is empty!\n");
    vm_stop();
}

void push(StackSlot stackSlot){
//    if(stackSlot.isObjRef)
//        vm_stop();
    if(int_pos < int_stack_size){
        int_stack_slot[int_pos++] = stackSlot;
        printf("pushed stackslot with pointer: %p\n", stackSlot.u.objRef);

        if(stackSlot.isObjRef) {
            bip.op1 = stackSlot.u.objRef;
            printf("pushed this value: ");
            bigPrint(stdout);
            printf("\n ");
        }
        print_stack_state();
        ///////BIS HIERHI
    } else{
        stack_overflow();
    }
}

StackSlot pop(){
    if(int_pos < 0){
        empty_stack();
    } else {
        StackSlot stack_var = int_stack_slot[int_pos - 1];
        int_pos--;
        /** Why i did it??????? foobar*/
//        if(!stack_var.isObjRef){
//            if (stack_var.u.number & 0x00800000) {
//                stack_var.u.number |= 0xFF000000;
//                return stack_var.u.number;
//            } else {
//                return stack_var.u.number;
//            }
//        }
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

    push(to_push);
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
    fp = *(int *)fp_slot.u.objRef->data;
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

void print_command(unsigned int IR){
    unsigned int i = IR >> 24;

    if(i == PUSHC){
        int to_push = IMMEDIATE(IR);
        printf("pushc %d\n", SIGN_EXTEND(to_push));
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
        int to_push = IMMEDIATE(IR);
        printf("asf %d\n", SIGN_EXTEND(to_push));
    }else if(i == RSF){
        printf("rsf\n");
    }else if(i == POPL){
        int to_push = IMMEDIATE(IR);
        printf("popl %d\n", SIGN_EXTEND(to_push));
    }else if(i == PUSHL){
        int to_push = IMMEDIATE(IR);
        printf("pushl %d\n", SIGN_EXTEND(to_push));
    }else if(i == POPG){
        int to_push = IMMEDIATE(IR);
        printf("popg %d\n", SIGN_EXTEND(to_push));
    }else if(i == PUSHG){
        int to_push = IMMEDIATE(IR);
        printf("pushg %d\n", SIGN_EXTEND(to_push));
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
        printf("jmp %d\n", IMMEDIATE(IR));
    }else if(i == BRF){
        printf("brf %d\n", IMMEDIATE(IR));
    }else if(i == BRT){
        printf("brt %d\n", IMMEDIATE(IR));
    }else if(i == CALL){
        printf("call %d\n", IMMEDIATE(IR));
    }else if(i == RET){
        printf("ret \n");
    }else if(i == DROP){
        printf("drop %d\n", IMMEDIATE(IR));
    }else if(i == PUSHR){
        printf("pushr \n");
    }else if(i == POPR){
        printf("popr \n");
    }else if(i == DUP){
        printf("dup \n");
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

    printf("creating stack slot for :");
    bip.op1 = objRef;
    bigPrint(stdout);
    printf("\n");
    return stackSlot;
}

/** before each operation must be used this function */
void setOp(){
    StackSlot f_elem = pop();
    StackSlot s_elem = pop();

    bip.op1 = f_elem.u.objRef;
    bip.op2 = s_elem.u.objRef;
}

void exec(unsigned int IR){
    unsigned int i = IR >> 24;

    if(i == PUSHC){
        if(int_pos < int_stack_size) {
            int to_push = IMMEDIATE(IR);
            StackSlot stackSlot = createStackSlot(to_push, true);
            push(stackSlot);
        }else{
            printf("stack is full\nhalt\n");
            printf("Ninja Virtual Machine stopped\n");
            exit(1);
        }
    }else if(i == ADD){
//        StackSlot f_elem = pop();
//        StackSlot s_elem = pop();
//        int sum = get_int_from_ref_slot(s_elem) + get_int_from_ref_slot(f_elem);
//        push(createStackSlot(sum, true));

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
        int to_push = IMMEDIATE(IR);
        pop_local(SIGN_EXTEND(to_push));
    }else if(i == PUSHL){
        int to_push = IMMEDIATE(IR);
        push_local(SIGN_EXTEND(to_push));
    }else if(i == SUB){
        setOp();
        bigSub();
        StackSlot stackSlot = create_stack_slot(bip.res);
        push(stackSlot);
    }else if(i == MUL){
//        StackSlot f_elem = pop();
//        StackSlot s_elem = pop();
//        int mul = get_int_from_ref_slot(f_elem) * get_int_from_ref_slot(s_elem);
//        push(createStackSlot(mul, true));
        setOp();
        bigMul();
        StackSlot stackSlot = create_stack_slot(bip.res);
        push(stackSlot);
    }else if(i == WRINT){
        StackSlot poped = pop();
        if(poped.isObjRef){
            bip.op1 = poped.u.objRef;
            bigPrint(stdout);
            printf("\n");
        }else{
            //int_result = get_int_from_ref_slot(poped);
            printf("%p", poped.u.objRef);
        }
    }else if(i == WRCHR){
        StackSlot poped = pop();
        int_result = get_int_from_ref_slot(poped);
        printf("%c", int_result);
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
        if(int_pos < sizeof(int_stack)/ 4) {
            int to_push = IMMEDIATE(IR);
            push(global_stack_slot[to_push]);
        }else{
            printf("StackOutOfBoundsError\n");
            vm_stop();
        }
    }else if(i == RDINT) {

        //int i;
        //scanf("%d", &i);
        //push(createStackSlot(i, true));

        bigRead(stdin);
        StackSlot stackSlot;
        stackSlot.isObjRef = true;
        stackSlot.u.objRef = bip.res;

        push(stackSlot);
        //bigPrint(stdout);
    }else if(i == RDCHR){
        char i;
        scanf("%c", &i);
        push(createStackSlot(i, true)); /// what should i do with char??
    }else if(i == DIV){
//        StackSlot f_elem = pop();
//        StackSlot s_elem = pop();

//        if(get_int_from_ref_slot(f_elem) != 0){
//            int div = get_int_from_ref_slot(s_elem) / get_int_from_ref_slot(f_elem);
//            push(createStackSlot(div, true));
//        }else{
//            printf("Error: division by zero \n");
//            exit(1);
//        }
        setOp();
        bigDiv();
        StackSlot stackSlot = create_stack_slot(bip.res);
        push(stackSlot);
    }else if(i == MOD){
//        StackSlot f_elem = pop();
//        StackSlot s_elem = pop();
//        if(get_int_from_ref_slot(f_elem) != 0){
//            int div = get_int_from_ref_slot(s_elem) % get_int_from_ref_slot(f_elem);
//            push(createStackSlot(div, true));
//        }else{
//            printf("Error: division by zero\n");
//            exit(1);
//        }
        setOp();
        bigDiv();
        StackSlot stackSlot = create_stack_slot(bip.rem);
        push(stackSlot);
    }
    else if(i == HALT){
    }else if(i == EQ){
        StackSlot first_pop = pop();
        StackSlot second_pop = pop();

        if(get_int_from_ref_slot(first_pop) == get_int_from_ref_slot(second_pop)){
            push(createStackSlot(1, true));
        }else{
            push(createStackSlot(0, true));
        }
    }else if(i == NE){
        StackSlot first_pop = pop();
        StackSlot second_pop = pop();

        if(get_int_from_ref_slot(first_pop) != get_int_from_ref_slot(second_pop)){
            push(createStackSlot(1, true));
        }else{
            push(createStackSlot(0, true));
        }
    }else if(i == LT){
        StackSlot first_pop = pop();
        StackSlot second_pop = pop();

        if(get_int_from_ref_slot(second_pop) < get_int_from_ref_slot(first_pop)){
            push(createStackSlot(1, true));
        }else{
            push(createStackSlot(0, true));
        }
    }else if(i == LE){
        StackSlot first_pop = pop();
        StackSlot second_pop = pop();

        if(get_int_from_ref_slot(second_pop) <= get_int_from_ref_slot(first_pop)){
            push(createStackSlot(1, true));
        }else{
            push(createStackSlot(0, true));
        }
    }else if(i == GT){
        StackSlot first_pop = pop();
        StackSlot second_pop = pop();

        if(get_int_from_ref_slot(second_pop) > get_int_from_ref_slot(first_pop)){
            push(createStackSlot(1, true));
        }else{
            push(createStackSlot(0, true));
        }
    }else if(i == GE){
        StackSlot first_pop = pop();
        StackSlot second_pop = pop();

        if(get_int_from_ref_slot(second_pop) >= get_int_from_ref_slot(first_pop)){
            push(createStackSlot(1, false));
        }else{
            push(createStackSlot(0, false));
        }
    }else if(i == BRF){
        StackSlot poped = pop();

        if(get_int_from_ref_slot(poped) == 0){
            jump(IMMEDIATE(IR));
        }
    }else if(i == BRT){
        StackSlot poped = pop();

        if(get_int_from_ref_slot(poped) == 1){
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
    }else{
        //printf("Ninja Virtual Machine stopped\n");
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
        if(global_stack_slot[i].isObjRef != NULL){
            printf("(objref)");
            printf(" %p \n", global_stack_slot[i].u.objRef);
        }else{
            printf("(nil)\n");
        }
        i++;
    }
    printf("      --- end of data ---\n");
}

void print_stack_slot(StackSlot stackSlot, int SP){
    if(stackSlot.isObjRef){
        printf("%d:\t (objref) %p \n", SP, stackSlot.u.objRef);
    }else{
        printf("%d:\t %d \n", SP, stackSlot.u.number);
    }
}
/** fucking foobar function prints stack state. 2 fucking hours debugging */
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
        ObjRef ref = global_stack_slot[i].u.objRef;
        if(global_stack_slot[i].isObjRef && ref == obj){
            printf("value = ");
            bip.op1 = ref;
            bigPrint(stdout);
            printf("\n");
            return;
        }
    }

    for(int i = 0; i < int_pos; i++){
        ObjRef ref = int_stack_slot[i].u.objRef;
        if(int_stack_slot[i].isObjRef && ref == obj){
            printf("value = ");
            bip.op1 = ref;
            bigPrint(stdout);
            printf("\n");
            return;
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
                        scanf("%p", &pointer);

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
                }else if(strcmp("step", input) == 0){ //ok
                    exec(IR);
                    break;
                }else if(strcmp("run", input) == 0){ //ok

                    if(breakpoint_mode == 1){
                        printf("%0*d", (2 - breakpoint_instruction_number / 10), 0);
                        printf("%d:\t",breakpoint_instruction_number);
                        print_command(program[breakpoint_instruction_number]);
                        breakpoint_mode = 0;
                    }

                    debug_mode = 0;
                    exec(IR);
                    break;
                }else if(strcmp("quit", input) == 0) { //ok
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

    // comment out below
    open_file(file_name);

    //remove comment below comment to debug in intelli
    // open_file("/Users/p.rozbytskyi/Desktop/out.bin");
    printf("DEBUG: file \'%s\' loaded (code size = %d, data size = %d)\n",
           file_name, instruction_number, global_stack_size);

    exec_prog();
}

int main(int argc, char *argv[]) {
    //debugging("/Users/p.rozbytskyi/Desktop/out.bin");
    //open_file("~/Desktop/out.bin");

    if (argv[1] == NULL) {
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
            if(argv[2] != NULL){
                debugging(argv[2]);
                printf("Ninja Virtual Machine stopped\n");
            }else{
                printf("ERROR: Cannot open file\n");
            }
        }else{
            if(argv[1] != NULL){
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

