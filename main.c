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

#define IMMEDIATE(x) ((x)&0x00FFFFFF)
#define SIGN_EXTEND(i) ((i) & 0x00800000 ? (i) | 0xFF000000 : (i))

#define VERSION 2

const char format_ids [4] = {'N', 'J', 'B', 'F'};

int prog[11];
int int_stack[10];
int int_pos = 0;
int int_result = 0;

void vm_stop(){
    printf("Ninja Virtual Machine stopped\n");
    exit(1);
}

void identifiers_format_checking(FILE *file){
    char c;
    int pos = 0;

    while (pos < 4){
        c = fgetc(file);

        if(format_ids[pos] != c){
            printf("ERROR: format identifiers are false\n");
            vm_stop();
        }
        pos++;
    }
}

void version_checking(FILE *file){
    unsigned int version;

    fread(&version, 4, 1, file);
    if(version != VERSION){
        printf("ERROR: file version ist incorrect. NJVM version %d", VERSION);
        vm_stop();
    }
}

int open_file(char * file_name){
    FILE *file;

    file = fopen(file_name, "r");

    if(file != NULL){
        int version;

        identifiers_format_checking(file);
        version_checking(file);
        printf("IDis are correct\n");

    }else{
        printf("ERROR: Cannot open file\n");
        vm_stop();
    }
    fclose(file);
}

void init_prog1(){
    //PROG1
    int arr [11] = {
            (PUSHC<<24)|IMMEDIATE(SIGN_EXTEND(3)),
            (PUSHC<<24)|IMMEDIATE(SIGN_EXTEND(4)),
            ADD<<24,
            (PUSHC<<24)|IMMEDIATE(SIGN_EXTEND(10)),
            (PUSHC<<24)|IMMEDIATE(SIGN_EXTEND(6)),
            SUB<<24,
            MUL<<24,
            WRINT<<24,
            (PUSHC<<24)|IMMEDIATE(SIGN_EXTEND(10)),
            WRCHR<<24,
            HALT
    };
    for(int i = 0; i < 11; i++){
        prog[i] = arr[i];
    }
}
void init_prog2(){
//TODO initialize arrays
//PROG2
    int arr [9] = {
            (PUSHC<<24)|IMMEDIATE(SIGN_EXTEND(-2)),
            RDINT<<24,
            MUL<<24,
            (PUSHC<<24)|IMMEDIATE(SIGN_EXTEND(3)),
            ADD<<24,
            WRINT<<24,
            (PUSHC<<24)|IMMEDIATE(SIGN_EXTEND('\n')),
            WRCHR<<24,
            HALT
    };
    for(int i = 0; i < 9; i++){
        prog[i] = arr[i];
    }
}
void init_prog3(){
    //PROG3
    int arr [5] = {
            RDCHR<<24,
            WRINT<<24,
            (PUSHC<<24)|IMMEDIATE(SIGN_EXTEND('\n')),
            WRCHR<<24,
            HALT
    };

    for(int i = 0; i < 5; i++){
        prog[i] = arr[i];
    }
}

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

void print_command(unsigned int IR){
    unsigned int i = IR >> 24;

    if(i == PUSHC){
        if(int_pos < sizeof(int_stack)/ 4) {
            int to_push = IR & 0x00FFFFFF;
            printf("pushc %d\n", to_push & 0x00800000 ?
                                 (to_push | 0xFF000000) : to_push);
        }else{
            printf("stack is full\nhalt\n");
            printf("Ninja Virtual Machine stopped\n");
            exit(1);
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
    unsigned int IR = prog[PC];
    int size = sizeof(prog)/4;

    while (IR != HALT && PC <= size){
        printf("%0*d", (2 - PC / 10), 0);
        printf("%d:\t",PC);
        print_command(IR);

        PC += 1;
        IR = prog[PC];
    }
    printf("%0*d", (2 - PC / 10), 0);
    printf("%d",PC);
    print_command(HALT);

    PC = 0;
    IR = prog[PC];
    while (IR != HALT && PC <= size){
        exec(IR);

        PC += 1;
        IR = prog[PC];
    }
    exec(HALT);
}

int main(int argc, char *argv[]){
    open_file("prog1.bin");
}

//int main(int argc, char *argv[]) {
//    printf("Ninja Virtual Machine started\n");
//    if (argc < 1)
//        printf("Error, no program is selected\n");
//    else if (strcmp("--help", argv[1]) == 0) {
//        printf("--prog1\tselect program 1 to execute\n"
//               "--prog2\tselect program 2 to execute\n"
//               "--prog3\tselect program 3 to execute\n"
//               "--version show version and exit\n"
//               "--help show this and exit\n");
//    }else if(strcmp("--version", argv[1]) == 0){
//        printf("Ninja Virtual Machine version 1 \n");
//    }else if(strcmp("--prog1", argv[1]) == 0){
//        init_prog1();
//        exec_prog();
//    }else if(strcmp("--prog2", argv[1]) == 0){
//        init_prog2();
//        exec_prog();
//    }else if(strcmp("--prog3", argv[1]) == 0) {
//        init_prog3();
//        exec_prog();
//    } else{
//        printf("Unknown command line argument '%s'\n", argv[1]);
//        vm_stop();
//    }
//    printf("Ninja Virtual Machine stopped\n");
//    return 0;
//}





