#include "kernel/types.h"
#include "user.h"
#include "kernel/param.h"

int main(int args,char* argv[]){
    int status;
    while(1){
        char buffer[512];
        char* s = gets(buffer,512);
        if(strlen(s) == 0) break;
        s[strlen(s) - 1] = '\0';
        if(fork() == 0){
            char* arg[MAXARG] = {"0"};
            int i;
            for(i = 1;i < args;i++){
                arg[i-1] = argv[i];
            }
            arg[i - 1] = s;
            if(exec(argv[1],arg) < -1) printf("error");
            close(0);
            exit(0);
        }
        wait(&status);
    }
    exit(0);
}
