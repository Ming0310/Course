#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define mydev_SIZE  0x1000
int main()
{
    int fp;
    int i;
    char buf[mydev_SIZE];
    fp = open("/dev/mydev", O_RDWR);
    // TODO：往mydev设备文件写入数据
    strcpy(buf,"hello world");
    write(fp,buf,sizeof(buf));
    memset(buf,0,mydev_SIZE);
    lseek(fp,0,SEEK_SET);
    // TODO: 从mydev设备读出数据
    i = 0;
    while(read(fp,&buf[i++],1)){
    }
    printf("first read:%s\n",buf);
    memset(buf,0,mydev_SIZE);
    lseek(fp,0,SEEK_SET);
    // TODO: 使用IOCTL的MEM_CLEAR命令清空内存
    ioctl(fp,0x1);
    lseek(fp,0,SEEK_SET);
    // TODO: 再次从mydev设备读出数据
    i = 0;
    while(read(fp,&buf[i++],1)){
    }
    printf("second read:%s\n",buf);
    close(fp);
    return 0;
}