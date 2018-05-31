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

#define EQ 17
#define NE 18
#define LT 19
#define LE 20
#define GT 21
#define GE 22

#define JMP 23
#define BRF 24
#define BRT 25

#define IMMEDIATE(x) ((x)&0x00FFFFFF)
#define SIGN_EXTEND(i) ((i) & 0x00800000 ? (i) | 0xFF000000 : (i))

#define VERSION 3

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
        printf("ERROR: file version ist incorrect. NJVM version %d\n", VERSION);
        close_file(file);
        vm_stop();
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
    global_stack = malloc(global_stack_size * sizeof(int));

    int i = 0;

    while (i < global_stack_size){
        global_stack[i] = 0;
        i++;
    }
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
void create_stack(){
    int_stack = malloc(int_stack_size * sizeof(int));
}

/**
 * Opens file
 * @param file_name
 * @return
 */
int open_file(char * file_name){
    FILE *file;
    char example[100];
    strcpy(example, "/Users/p.rozbytskyi/CLionProjects/ksp_debug/");
    strcat(example, file_name);

    file = fopen(example, "r");

    if(file != NULL){
        int version;

        identifiers_format_checking(file);
        version_checking(file);
        instructions_number_check(file);
        global_variables_check(file);
        read_instructions(file);
        create_stack();
    }else{
        printf("ERROR: Cannot open file\n");
        fclose(file);
        vm_stop();
    }
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
//PROG2
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

void stack_overflow(){
    printf("Error. Stack is full!\n");
    vm_stop();
}

void empty_stack(){
    printf("Error. Stack is empty!\n");
    vm_stop();
}

void push(int el){
    //printf("pushed %d onto %d position\n", el, int_pos);
    if(int_pos < int_stack_size){
        int_stack[int_pos++] = el;
    } else{
        stack_overflow();
    }
}

int pop(){
    if(int_pos < 0){
        empty_stack();
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
 * Pushing global variable onto stack
 * @param position of the variable in stack
 */
void pushg(int position){
    if(position >= 0 && global_stack_size > position){
        push(global_stack[position]);
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
        int elem = pop();
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
    int to_push = int_stack[fp + n];

    push(to_push);
}

/**
 * pop up first stack element to n position in local stack
 * @return
 */
void pop_local(int n){
    int poped = pop();

    int_stack[fp + n] = poped;
}

/**
 * allocate stack frame
 * @param n
 */
void asf(int n){
    push(fp);

    int_pos++;
    fp = int_pos;

    int_pos += n;
}

/**
 * release stack frame
 */
void rsf(){
    int_pos = fp;
    fp = pop();
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
    }
}

void exec(unsigned int IR){
    unsigned int i = IR >> 24;

    if(i == PUSHC){
        if(int_pos < int_stack_size) {
            int to_push = IMMEDIATE(IR);
            push(to_push);
        }else{
            printf("stack is full\nhalt\n");
            printf("Ninja Virtual Machine stopped\n");
            exit(1);
        }
    }else if(i == ADD){

        int f_elem = pop();
        int s_elem = pop();
        int sum = s_elem + f_elem;
        push(sum);
    }else if(i == ASF){
        int n = IMMEDIATE(IR);
        asf(n);
    }else if(i == RSF){
        rsf();
    }else if(i == POPL){
        int to_push = IMMEDIATE(IR);
        pop_local(to_push);
    }else if(i == PUSHL){
        int to_push = IMMEDIATE(IR);
        push_local(to_push);
    }else if(i == SUB){

        int f_elem = pop();
        int s_elem = pop();
        int sub = s_elem - f_elem;
        push(sub);
    }else if(i == MUL){

        int f_elem = pop();
        int s_elem = pop();
        int mul = f_elem * s_elem;
        push(mul);
    }else if(i == WRINT){
        int_result = pop();
    }else if(i == WRCHR){

    }else if(i == POPG){
        int position = IMMEDIATE(IR);
        int to_push = pop();

        if(position >= 0 && position < global_stack_size){
            global_stack[position] = to_push;
            global_stack_pointer++;
        }else{
            printf("GlobalStackOutOfBoundsError\n");
            vm_stop();
        }
    }else if(i == PUSHG){
        if(int_pos < sizeof(int_stack)/ 4) {
            int to_push = IMMEDIATE(IR);
            push(global_stack[to_push]);
        }else{
            printf("StackOutOfBoundsError\n");
            vm_stop();
        }
    }else if(i == RDINT) {
        int i;
        scanf("%d", &i);
        push(i);

    }else if(i == RDCHR){
        char i;
        scanf("%c", &i);
        push(i);

    }else if(i == DIV){
        int f_elem = pop();
        int s_elem = pop();
        if(f_elem != 0){
            int mul = f_elem * s_elem;
            push(mul);
        }else{
            printf("Divide by zero Error!\n");
            printf("Ninja Virtual Machine stopped\n");
            exit(1);
        }
    }else if(i == HALT){
            printf("%d\n", int_result);
    }else if(i == EQ){
        int first_pop = pop();
        int second_pop = pop();

        if(first_pop == second_pop){
            push(1);
        }else{
            push(0);
        }
    }else if(i == NE){
        int first_pop = pop();
        int second_pop = pop();

        if(first_pop != second_pop){
            push(1);
        }else{
            push(0);
        }
    }else if(i == LT){
        int first_pop = pop();
        int second_pop = pop();

        if(first_pop < second_pop){
            push(1);
        }else{
            push(0);
        }
    }else if(i == LE){
        int first_pop = pop();
        int second_pop = pop();

        if(first_pop <= second_pop){
            push(1);
        }else{
            push(0);
        }
    }else if(i == GT){
        int first_pop = pop();
        int second_pop = pop();

        if(first_pop < second_pop){
            push(1);
        }else{
            push(0);
        }
    }else if(i == GE){
        int first_pop = pop();
        int second_pop = pop();

        if(first_pop >= second_pop){
            push(1);
        }else{
            push(0);
        }
    }else if(i == BRF){
        int poped = pop();

        if(poped == 0){
            int value = IMMEDIATE(IR);
            ProgramCounter = value;
        }
    }else if(i == BRT){
        int poped = pop();

        if(poped == 1){
            int value = IMMEDIATE(IR);
            ProgramCounter = value;
        }
    }else if(i == JMP){
        ProgramCounter = IMMEDIATE(IR);
    }else{
        printf("Ninja Virtual Machine stopped\n");
    }
}


void print_prog(){
    int PC = 0;
    unsigned int IR = 1;
    int size = instruction_number;

    while (IR != HALT && PC < size){
        IR = program[PC];
        printf("%0*d", (2 - PC / 10), 0);
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
        printf("%d]:\t %d \n",i, global_stack[i]);
        i++;
    }
    printf("      --- end of data ---\n");
}

/** prints state of stack */
void print_stack_state(){
    int SP = int_pos;
    int FP = fp;

    printf("sp         ---> ");
    printf("%0*d", (2 -  SP/ 10), 0);
    printf("%d:\t xxxx \n",SP - 1);
    SP--;

    while (SP > FP){
        printf("        \t%0*d", (2 -  SP/ 10), 0);
        printf("%d:\t %d \n", SP - 1, int_stack[SP]);

        SP--;
    }

    printf("fp         ---> ");
    printf("%0*d", (2 -  FP/ 10), 0);
    printf("%d:\t %d \n", FP - 1, int_stack[FP]);
    FP--;

    while (FP > 0){
        printf("        \t%0*d", (2 -  FP/ 10), 0);
        printf("%d:\t %d \n", FP - 1, int_stack[FP]);

        FP--;
    }

    printf("    --- bottom of stack ---\n");
}

void exec_prog(){
    printf("Ninja Virtual Machine started\n");

    int size = instruction_number;
    ProgramCounter = 0;
    unsigned int IR;
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
                    printf("DEBUG [inspect]: stack, data?\n");
                    scanf("%s", input);
                    if(strcmp("data", input) == 0){
                        print_global_stack();
                        ProgramCounter--;
                    }else if(strcmp("stack", input) == 0){
                        print_stack_state();
                        ProgramCounter--;
                    }
                    break;
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
    open_file(file_name);
    printf("DEBUG: file \'%s\' loaded (code size = %d, data size = %d)\n",
           file_name, instruction_number, global_stack_size);

    exec_prog();
}

int main(int argc, char *argv[]) {
    if (argc < 1) {
        printf("Error, no program is selected\n");
    }else if (strcmp("--help", argv[1]) == 0) {
        printf("usage: ./njvm [options] <code file>\n"
               "--version        show version and exit\n"
               "--help           show this help and exit\n"
               "--debug          start virtual machine in debug mode\n");
    }else if(strcmp("--version", argv[1]) == 0){
        printf("Ninja Virtual Machine version %d \n", VERSION);
    }else if(strcmp("--debug", argv[1]) == 0){
        debugging(argv[2]);
    }else{
        open_file(argv[1]);
        exec_prog();
    }

    printf("Ninja Virtual Machine stopped\n");
    return 0;
}





