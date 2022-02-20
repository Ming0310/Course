#include "kernel/types.h"
#include "user.h"
int main(int argc,char* argv[]){
    int p1[2];
    int p2[2];
    int pid,status;
    int size1,size2;
    char *buffer1 = (char*) malloc(128*sizeof(char));
    char *buffer2 = (char*) malloc(128*sizeof(char));
    pipe(p1);
    pipe(p2);
    if((pid = fork()) ==  0) // 子进程
    {
	close(p1[1]);
        size1 = read(p1[0],buffer2,5);
        if(size1 < 0)
	{
	    printf("child read error\n");
	    exit(1);
        }
	printf("%d: received %s\n",getpid(),buffer2);
	close(p1[0]);
	close(p2[0]);
	buffer2 = "pong";
	size1 = write(p2[1],buffer2,5);
	if(size1 < 0)
	{
	    printf("child write error\n");
	    exit(1);
	}
	close(p2[1]);
    }
    else if(pid > 0){                    //父进程
        close(p1[0]);
	buffer1 = "ping";
        size2 = write(p1[1],buffer1,5);
	if(size2 < 0)
	{
	    printf("parent write error\n");
	    exit(1);
	}
	close(p1[1]);
	close(p2[1]);
	size2 = read(p2[0],buffer1,5);
	if(size2 < 0)
	{
	    printf("parent read error\n");
	    exit(1);
	}
	printf("%d: received %s\n",getpid(),buffer1);
	close(p2[0]);
	wait(&status);
    }
    else {
	printf("fork error");
    }
    exit(0);
}
