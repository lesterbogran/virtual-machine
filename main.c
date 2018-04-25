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

 int prog [11] = {
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
int stack[10] = {};
int pos = 0;
int result = 0;

void init_prog1(){
//    prog = { (PUSHC<<24)|IMMEDIATE(3),
//             (PUSHC<<24)|IMMEDIATE(3),
//             ADD<<24,
//             (PUSHC<<24)|IMMEDIATE(10),
//             (PUSHC<<24)|IMMEDIATE(6),
//             SUB<<24,
//             MUL<<24,
//             WRINT<<24,
//             (PUSHC<<24)|IMMEDIATE(10),
//             WRCHR<<24,
//             HALT
//    };
}
void init_prog2(){
//TODO initialize arrays
}
void init_prog3(){
//TODO initialize arrays
}

void push(int el){
    if(pos < 11){
        stack[pos++] = el;
    } else{
        exit(1);
    }
}

int pop(){
    if(pos - 1 < 0){
        exit(1);
    } else {
        int stack_var = stack[pos - 1];
        pos--;
        if (stack_var & 0x00800000) {
            stack_var |= 0xFF000000;
            return stack_var;
        } else {
            return stack_var;
        }
    }
}

void exec(unsigned int IR){
    unsigned int i = IR >> 24;

    if(i == PUSHC){
        if(pos < sizeof(stack)/ 4) {
            int to_push = IR & 0x00FFFFFF;
            push(to_push);
            printf("pushc %d\n", to_push);
        }else{
            printf("stack is full\nhalt\n");
            exit(1);
        }
    }else if(i == ADD){

        int f_elem = pop();
        int s_elem = pop();
        int sum = s_elem + f_elem;
        push(sum);

        printf("add\n");
    }else if(i == SUB){

        int f_elem = pop();
        int s_elem = pop();
        int sub = s_elem - f_elem;
        push(sub);

        printf("sub\n");
    }else if(i == MUL){

        int f_elem = pop();
        int s_elem = pop();
        int mul = f_elem * s_elem;
        push(mul);

        printf("mul\n");
    }else if(i == WRINT){
        result = pop();

        printf("wrint\n");
    }else if(i == WRCHR){

        printf("wrchr\n");
    }else if(i == HALT){

        printf(":\thalt\n%d\n",result);
    }
}

void exec_prog(){
    int PC = 0;
    unsigned int IR = prog[PC];
    while (IR != HALT && PC <= 11){
        printf("%0*d", (2 - PC / 10), 0);
        printf("%d:\t",PC);
        exec(IR);

        PC += 1;
        IR = prog[PC];
    }
    printf("%0*d", (2 - PC / 10), 0);
    printf("%d",PC);
    exec(HALT);
}


int main(int argc, char *argv[]) {
//    int g =   (PUSHC<<24)|IMMEDIATE(SIGN_EXTEND(-1));

//    printf("%x\n", g);
/*    g = SIGN_EXTEND(g);
    printf("%d\n",k);
    printf("%x\n",k);*/


    init_prog1();
//    if (argc < 1)
//        printf("Error, no program is selected");
//    else if (strcmp("--help", argv[1]) == 0) {
//        printf("--prog1\tselect program 1 to execute\n"
//               "--prog2\tselect program 2 to execute\n"
//               "--prog3\tselect program 3 to execute\n"
//               "--version\tshow version and exit\n"
//               "--help\tshow this and exit\n");
//    }else if(strcmp("--version", argv[1]) == 0){
//        printf("Ninja Virtual Machine version 1 \n");
//    }else if(strcmp("--prog1", argv[1]) == 0){
//        init_prog1();
//    }else if(strcmp("--prog2", argv[1]) == 0){
//        init_prog2();
//    }else if(strcmp("--prog3", argv[1]) == 0) {
//        init_prog3();
//    } else{
//        printf("Unknown command line argument '%s'\n", argv[1]);
//    }

    exec_prog();

    return 0;
}





