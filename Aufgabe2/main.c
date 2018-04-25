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

unsigned int prog [11] = { (PUSHC<<24)|IMMEDIATE(3),
         (PUSHC<<24)|IMMEDIATE(4),
         ADD<<24,
         (PUSHC<<24)|IMMEDIATE(10),
         (PUSHC<<24)|IMMEDIATE(6),
         SUB<<24,
         MUL<<24,
         WRINT<<24,
         (PUSHC<<24)|IMMEDIATE(10),
         WRCHR<<24,
         HALT
};
unsigned int stack[10] = {};
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

void exec(unsigned int IR){
    unsigned int i = IR >> 24;

    if(i == PUSHC){
        if(pos < sizeof(stack)/ 4) {
            int to_push = IR & 0x00FFFFFF;
            stack[pos] = to_push;
            pos++;
            printf("pushc %d\n", to_push);
        }else{
            printf("stack is full\nhalt\n");
            exit(1);
        }
    }else if(i == ADD){
        int sum = stack[pos - 1] + stack[pos - 2];
        stack[pos - 2] = sum;
        pos--;
        printf("add\n");
    }else if(i == SUB){
        int sub = stack[pos - 2] - stack[pos - 1];
        stack[pos - 2] = sub;
        pos--;
        printf("sub\n");
    }else if(i == MUL){
        int mul = stack[pos - 1] * stack[pos - 2];
        stack[pos - 2] = mul;
        pos--;
        printf("mul\n");
    }else if(i == WRINT){
        result = stack[pos - 1];
        pos--;
        printf("wrint\n");
    }else if(i == WRCHR){
        printf("wrchr\n");
    }else if(i == HALT){
        printf("halt\n%d\n",result);
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
    printf("%d\t",PC);
    exec(HALT);
}


int main(int argc, char *argv[]) {
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





