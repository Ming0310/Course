#include "kernel/types.h"
#include "user.h"
void isPrime(int fd);
int main(int argc,char* argv[]){
    int pid,status;
    int p[2];
    pipe(p);
    if((pid = fork()) > 0){ //父进程
	close(p[0]);
	for(int i = 2;i < 36;i++)
            write(p[1],&i,sizeof(int));
	close(p[1]);
    }
    else if(pid == 0){
	close(p[1]);
	isPrime(p[0]);
        close(p[0]);
    }
    else{
	printf("Fork error\n");
	exit(1);
    }
    wait(&status);
    exit(0);
}

void isPrime(int fd){
    int p[2];
    int pid,status;
    pipe(p);
    int prime;
    int n = read(fd,&prime,sizeof(int));
    if(n == 0) return ;
    if((pid = fork()) > 0){
        close(p[0]);
       printf("prime %d\n",prime);
        while(n){
            int x;
            n = read(fd,&x,sizeof(int));
	    if(x % prime != 0)
                write(p[1],&x,sizeof(int));
            }
        close(p[1]);
    }
    else if(pid == 0){
        close(p[1]);
        isPrime(p[0]);
        close(p[0]);
    }
    else{
	printf("Fork error!\n");
    }
    wait(&status);
    return ;
}
