#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#define N 50
int match(char* path,char* object){
    char *q,*p;
    // printf("path:%s\n",path);
    q = object;
    p = path + strlen(path);
    for(;p >= path;p--){
        if(*p == '/') break;
        else continue;
    }
    p++;
    if(path + strlen(path) - p != strlen(object)) return 0;
    while((p <= path + strlen(path))){
      if(*p != *q) return 0;
      p++;
      q++;
    }
    return 1;   
}


void find(char* path,char* object){
    int fd;
    char buffer[512], *p;
    struct dirent de;
    struct stat st;
    if((fd = open(path,0)) < 0){
       //  printf("open file error!\n");
        return ;
    }
    if(fstat(fd,&st) < 0){
       //  printf("read status error!\n");
        close(fd);
        return ;
    }
    switch(st.type){
        case T_FILE:
           // printf("filename:%s\n",path);
            if(match(path,object) == 1)
              printf("%s\n",path);
            break;
        case T_DIR:
             strcpy(buffer,path);
             p = buffer + strlen(buffer);
             *p++ = '/';
             while(read(fd,&de,sizeof(de)) == sizeof(de)){
                 if(de.inum == 0) continue;
                 if(strcmp(de.name,".") == 0 || strcmp(de.name,"..") == 0) continue;
                 memmove(p,de.name,N);
                 p[N] = 0;
                 find(buffer,object);
             }
             break;
    }
    close(fd);
    return ;
    
}


int main(int argv,char* args[]){
    // printf("%s,%s",args[0],args[1]);
    if(argv < 3) printf("parameter lost\n");
    else find(args[1],args[2]);
    exit(0);
}
