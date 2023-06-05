#include <stdlib.h>
#include <stdio.h>
#include "extmem.h"
int getValue(char *blk,bool choose);

int main(){
    Buffer buff;
    if(!initBuffer(520,64,&buff)){
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    // lab1
    int object = 130;
    for(int i = 17;i < 49;i++){
        char* blk = readBlockFromDisk(i,buff);
        if(!blk){
            perror("load block error!\n");
            return -1;
        }
        printf("�������ݿ�%d\n",i);
        for(int j = 0;j < 6;j++){
            if(getValue(blk + j*8,0) == object){
                printf("(%d,%d)\n",object,getValue(blk + j*8,1));
            }
        }
    }


}
int getValue(char *blk,bool choose){
    char temp[5];
    if(!choose){ // ���ص�һ��ֵ
        for(int i = 0;i < 4;i++){
            temp[i] = *(blk + i);
        }
    }
    else{ // ���صڶ���ֵ
        for(int i = 0;i < 4;i++){
            temp[i] = *(blk + i + 4);
        }
    }
    return atoi(temp);
}
