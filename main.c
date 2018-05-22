//
// Created by przb86 on 22.04.18.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

#define STACK_SIZE 20
#define LOCAL_STACK_SIZE 1000

#define IMMEDIATE(x) ((x)&0x00FFFFFF)
#define SIGN_EXTEND(i) ((i) & 0x00800000 ? (i) | 0xFF000000 : (i))

#define VERSION 2

const char format_ids [4] = {'N', 'J', 'B', 'F'};

//unsigned int prog[11];
/** Normal stack */
int *int_stack;
/** next free place*/
int int_pos = 0;
/** result of calculation*/
int int_result = 0;

/** array with instructions for VM*/
unsigned int *program;
/** number of instructions in program*/
int instruction_number = 0;

/** array with global variables */
int *global_stack;
/** global stack pointer */
int global_stack_pointer = 0;
/** global stack size*/
int global_stack_size = 0;


/** array with local variables */
int *local_stack;
/** stack pointer */
int sp = 0;
/** frame pointer */
int fp = 0;

/**
 * Stopping VM
 */
void vm_stop(){
    printf("Ninja Virtual Machine stopped\n");
    exit(1);
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
        //printf(":%x", c);
        pos++;
    }
    printf("\n");
    //printf("correct\n");
}

/**
 * Check VM version
 * @param file
 */
void version_checking(FILE *file){
    unsigned int version;

    fread(&version, 4, 1, file);
    if(version != VERSION){
        printf("ERROR: file version ist incorrect. NJVM version %d\n", VERSION);
        close_file(file);
        vm_stop();
    }
    //printf(":%x\n", version);
    //printf("correct\n");
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
    //printf(":%x\n", instruction_number);
    //printf("correct\n");
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
   // printf("correct\n");
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
    //printf(":%x\n", globals);
    //printf("correct\n");
}

/**
 * Allocating memory for local stack
 */
void create_local_stack(){
    local_stack = malloc(LOCAL_STACK_SIZE * sizeof(int));
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
        int version;

        //correct
        identifiers_format_checking(file);
        //printf("correct\n");
        //correct
        version_checking(file);
        //correct
        instructions_number_check(file);
        //correct
        global_variables_check(file);
        //correct
        read_instructions(file);

        create_local_stack();
        

        printf("\n");
//        global_stack[0] = 10;
//        printf("%d", global_stack[0]);

        //printf("Num of globals: %d\n", global_variables_num);

    }else{
        printf("ERROR: Cannot open file\n");
        vm_stop();
    }
    fclose(file);
}
//
//void init_prog1(){
//    //PROG1
//    int arr [11] = {
//            (PUSHC<<24)|IMMEDIATE(SIGN_EXTEND(3)),
//            (PUSHC<<24)|IMMEDIATE(SIGN_EXTEND(4)),
//            ADD<<24,
//            (PUSHC<<24)|IMMEDIATE(SIGN_EXTEND(10)),
//            (PUSHC<<24)|IMMEDIATE(SIGN_EXTEND(6)),
//            SUB<<24,
//            MUL<<24,
//            WRINT<<24,
//            (PUSHC<<24)|IMMEDIATE(SIGN_EXTEND(10)),
//            WRCHR<<24,
//            HALT
//    };
//    for(int i = 0; i < 11; i++){
//        prog[i] = arr[i];
//    }
//}
//void init_prog2(){
////PROG2
//    int arr [9] = {
//            (PUSHC<<24)|IMMEDIATE(SIGN_EXTEND(-2)),
//            RDINT<<24,
//            MUL<<24,
//            (PUSHC<<24)|IMMEDIATE(SIGN_EXTEND(3)),
//            ADD<<24,
//            WRINT<<24,
//            (PUSHC<<24)|IMMEDIATE(SIGN_EXTEND('\n')),
//            WRCHR<<24,
//            HALT
//    };
//    for(int i = 0; i < 9; i++){
//        prog[i] = arr[i];
//    }
//}
//void init_prog3(){
//    //PROG3
//    int arr [5] = {
//            RDCHR<<24,
//            WRINT<<24,
//            (PUSHC<<24)|IMMEDIATE(SIGN_EXTEND('\n')),
//            WRCHR<<24,
//            HALT
//    };
//
//    for(int i = 0; i < 5; i++){
//        prog[i] = arr[i];
//    }
//}

void int_stack_overflow(){
    printf("Error. Stack is full!\n");
    vm_stop();
}

void empty_int_stack(){
    printf("Error. Stack is empty!\n");
    vm_stop();
}

void push_int(int el){
    if(int_pos < 11){
        int_stack[int_pos++] = el;
    } else{
        int_stack_overflow();
    }
}

int pop_int(){
    if(int_pos - 1 < 0){
        empty_int_stack();
    } else {
        int stack_var = int_stack[int_pos - 1];
        int_pos--;
        if (stack_var & 0x00800000) {
            stack_var |= 0xFF000000;
            return stack_var;
        } else {
            return stack_var;
        }
    }
    return 0;
}

/**
 * Creating global stack
 */
void create_global_stack(){
    global_stack = malloc(global_stack_size * sizeof(int));
}

/**
 * Pushing global variable onto stack
 * @param position of the variable in stack
 */
void pushg(int position){
    if(position >= 0 && global_stack_size > position){
        push_int(global_stack[position]);
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
        int elem = pop_int();
        global_stack[position] = elem;
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
    // printf("to push: %d\n", n);
    //bug is because of rsf => fp = -1
    if(LOCAL_STACK_SIZE > fp && fp > -1){
        local_stack[fp] = n;
        fp++;
        printf("first elem: %d\n", local_stack[0]);
    }else{
         printf("LocalStackOverflowError\n");
         vm_stop();
    }
}

/**
 * pop up first element
 * @return
 */
int pop_local(){
    if(fp > -1 && fp < LOCAL_STACK_SIZE){
        fp--;
        return local_stack[fp];
    } else{
        printf("LocalStackOutOfBoundsError\n");
        vm_stop();
    }
}

/**
 * allocate stack frame
 * @param n
 */
void asf(int n){
    local_stack[fp] = fp;

    //printf("pushed local: %d\n", n);
    // printf("fp prev: %d\n", sp);
    sp++;
    fp = sp;
    //printf("fp next: %d\n", fp);

    sp += n;
    //printf("sp after asf: %d\n", sp);
}

/**
 * release stack frame
 */
void rsf(){
    sp = fp;
    fp = pop_local();
}

void print_command(unsigned int IR){
    unsigned int i = IR >> 24;

    if(i == PUSHC){
        if(int_pos < sizeof(int_stack)/ 4) {
            int to_push = IR & 0x00FFFFFF;
            printf("pushc %d\n", to_push & 0x00800000 ?
                                 (to_push | 0xFF000000) : to_push);
        }else{
            printf("stack is full\nhalt\n");
            vm_stop();
        }
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
    }else if(i == HALT){
        printf(":\thalt\n");
    }else if(i == ASF){
        int to_push = IR & 0x00FFFFFF;
        printf("asf %d\n", to_push & 0x00800000 ?
                             (to_push | 0xFF000000) : to_push);
    }else if(i == RSF){
        printf("rsf\n");
    }else if(i == POPL){
        int to_push = IR & 0x00FFFFFF;
        printf("popl %d\n", to_push & 0x00800000 ?
                             (to_push | 0xFF000000) : to_push);
    }else if(i == PUSHL){
        int to_push = IR & 0x00FFFFFF;
        printf("pushl %d\n", to_push & 0x00800000 ?
                           (to_push | 0xFF000000) : to_push);
    }
}

void exec(unsigned int IR){
    unsigned int i = IR >> 24;

    if(i == PUSHC){
        if(int_pos < sizeof(int_stack)/ 4) {
            int to_push = IR & 0x00FFFFFF;
            push_int(to_push);
        }else{
            printf("stack is full\nhalt\n");
            printf("Ninja Virtual Machine stopped\n");
            exit(1);
        }
    }else if(i == ADD){

        int f_elem = pop_int();
        int s_elem = pop_int();
        int sum = s_elem + f_elem;
        push_int(sum);
    }else if(i == ASF){
        int n = IR & 0x00FFFFFF;
        asf(n);
    }else if(i == RSF){
        rsf();
    }else if(i == POPL){
        pop_local();
    }else if(i == PUSHL){
        int to_push = IR & 0x00FFFFFF;
        push_local(to_push);
    }else if(i == SUB){

        int f_elem = pop_int();
        int s_elem = pop_int();
        int sub = s_elem - f_elem;
        push_int(sub);
    }else if(i == MUL){

        int f_elem = pop_int();
        int s_elem = pop_int();
        int mul = f_elem * s_elem;
        push_int(mul);
    }else if(i == WRINT){
        int_result = pop_int();

    }else if(i == WRCHR){


    }else if(i == POPG){
        int position = IR & 0x00FFFFFF;
        int to_push = pop_int();

        if(position >= 0 && position < global_stack_size){
            global_stack[position] = to_push;
        }else{
            printf("GlobalStackOutOfBoundsError\n");
            vm_stop();
        }
    }else if(i == PUSHG){
        if(int_pos < sizeof(int_stack)/ 4) {
            int to_push = IR & 0x00FFFFFF;
            push_int(to_push);
        }else{
            printf("StackOutOfBoundsError\n");
            vm_stop();
        }
    }else if(i == RDINT) {
        int i;
        scanf("%d", &i);
        push_int(i);

    }else if(i == RDCHR){
        char i;
        scanf("%c", &i);
        push_int(i);

    }else if(i == DIV){
        int f_elem = pop_int();
        int s_elem = pop_int();
        if(f_elem != 0){
            int mul = f_elem * s_elem;
            push_int(mul);
        }else{
            printf("Divide by zero Error!\n");
            printf("Ninja Virtual Machine stopped\n");
            exit(1);
        }
    }else if(i == HALT){
            printf("%d\n",int_result);
    }else{
        printf("Ninja Virtual Machine stopped\n");
    }
}

void exec_prog(){
    int PC = 0;
    unsigned int IR = program[PC];
    int size = instruction_number;

    while (IR != HALT && PC < size){
        printf("%0*d", (2 - PC / 10), 0);
        printf("%d:\t",PC);
        print_command(IR);

        PC += 1;
        IR = program[PC];
    }
    printf("%0*d", (2 - PC / 10), 0);
    printf("%d",PC);
    print_command(HALT);

    PC = 0;
    IR = program[PC];
    while (IR != HALT && PC < size){
        exec(IR);

        PC += 1;
        IR = program[PC];
    }
    exec(HALT);
}

int main(int argc, char *argv[]) {
    printf("Ninja Virtual Machine started");
    if (argc < 1) {
        printf("Error, no program is selected\n");
    }else if (strcmp("--help", argv[1]) == 0) {
        printf("usage: ./njvm [options] <code file>\n"
               "--version        show version and exit\n"
               "--help           show this help and exit\n");
    }else if(strcmp("--version", argv[1]) == 0){
        printf("Ninja Virtual Machine version %d \n", VERSION);
    }else{
        //printf("%s", argv[1]);
        //printf("ppp\n");
        open_file(argv[1]);
        exec_prog();
    }
    printf("Ninja Virtual Machine stopped\n");
    return 0;
}





