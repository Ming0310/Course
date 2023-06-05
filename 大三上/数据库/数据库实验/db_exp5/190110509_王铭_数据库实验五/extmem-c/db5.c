#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "extmem.h"
int getValue(unsigned char *blk,int choose); // choose为0，获取当前的第一个属性，choose为1，获取第二个属性
int copyToBlock(unsigned char *sblk,unsigned char *dblk);// 将一个sblk的一条记录copy到另一个dblk
Buffer buff;// 模拟内存
int lab1();
int lab2();
int lab3();
int lab4();
int lab5();
/*
    其实是选择排序，参数为首地址，空闲块，record总数目
*/
void BBsort(unsigned char *blk1,unsigned char * blk,int total_record);
void swapRecord(unsigned char *blk1,unsigned char *blk2,unsigned char *blk); // blk为空闲块
/*
    第二轮六路归并排序，第一个参数为起始块，第二参数为每一组内有多少块，第三个参数为一共有多少个块
    第四个参数为写入的首磁盘块号
*/
void merge_sort(int ini_block,int group_size,int total_block,int dest_block);
/*
    获取下一个值，关系R的choose的传参为0，S为1
*/
int getNext(unsigned char *blk,int *counter,int choose);
int main(){
    if(!initBuffer(520,64,&buff)){
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    // test();
    printf("-------------------------lab1----------------------------------------\n");
    lab1();
    printf("-------------------------lab2----------------------------------------\n");
    // printf("freeNum:%d\n",buff.numFreeBlk);
    lab2();
    printf("-------------------------lab3----------------------------------------\n");
    // printf("freeNum:%d\n",buff.numFreeBlk);
    lab3();
    // printf("freeNum:%d\n",buff.numFreeBlk);
    printf("-------------------------lab4----------------------------------------\n");
    lab4();
    printf("-------------------------lab5----------------------------------------\n");
    // printf("freeNum:%d\n",buff.numFreeBlk);
    lab5();
}

int getValue(unsigned char *blk,int choose){
    char temp[5];
    if(choose == 0){ // 返回第一个值
        for(int i = 0;i < 4;i++){
            temp[i] = *(blk + i);
        }
    }
    else{ // 返回第二个值
        for(int i = 0;i < 4;i++){
            temp[i] = *(blk + i + 4);
        }
    }
    return atoi(temp);
}

int copyToBlock(unsigned char *sblk,unsigned char *dblk){
    for(int i = 0;i < 8;i++){
        *(dblk + i) = *(sblk + i);
    }
    return 1;
}

int getNext(unsigned char *blk,int *counter,int choose){
    int value;
    (*counter)++;
    if((*counter) % 8 != 7){
        value = getValue(blk + (*counter) % 8 * 8,0);
    }
    else{ // 换块
        (*counter)++;
        freeBlockInBuffer(blk,&buff);
        memset(blk,0,64);
        if(choose == 0){
            if((*counter) / 8 < 16)
                blk = readBlockFromDisk(221 + (*counter) / 8,&buff);
            else return -1;
        }
        else{
            if((*counter) / 8 < 32)
                blk = readBlockFromDisk(301 + (*counter) / 8,&buff);
            else{
                return -1;
            }
        }
        value = getValue(blk + (*counter) % 8 * 8,0);
    }
    return value;
}

int lab1(){
    // lab1
    int object = 130;
    int counter = 0;
    int io_counter = 0;
    printf("基于线性搜索的关系选择算法 S.C = %d\n",object);
    printf("---------------------------------------------------------------------\n");
    unsigned char * dblk = getNewBlockInBuffer(&buff);
    for(int i = 17;i < 49;i++){
        unsigned char* blk = readBlockFromDisk(i,&buff);
        io_counter++;
        if(!blk){
            perror("load block error!\n");
            return -1;
        }
        printf("读入数据块%d\n",i);
        for(int j = 0;j < 6;j++){
            if(getValue(blk + j * 8,0) == object){
                copyToBlock(blk + j * 8,dblk + 8 * (counter % 7));
                printf("(C=%d,D=%d)\n",object,getValue(blk + j*8,1));
                counter++;
                // 当前block已经装满，可以写回磁盘了
                if(counter % 7 == 0){
                    // writeBlockToDisk(dblk,100 + counter % 7,&buff);
                    io_counter++;
                    printf("注：结果写入磁盘:%d\n",100 + counter / 7);
                    if (writeBlockToDisk(dblk,100 + counter / 7,&buff) != 0)
                    {
                        perror("write error\n");
                        return -1;
                    }
                }

            }
        }
        freeBlockInBuffer(blk,&buff);
    }
    if(counter % 7 != 0){
            io_counter++;
            printf("注：结果写入磁盘:%d\n",101 + counter / 7);
        if (writeBlockToDisk(dblk,100 + counter / 7,&buff) != 0)
            {
                perror("write error\n");
                return -1;
            }
    }
    printf("满足要求的元组共%d个\n",counter);
    printf("IO读写一共%d次\n",io_counter);
    return 0;
}

int lab2(){
    // 第二轮采用六路归并排序
    // 处理R 16个blk 第一轮3个块一组
    unsigned char *blk1,*blk2,*blk3,*blk4,*blk5,*blk6;
    unsigned char * blk = getNewBlockInBuffer(&buff);
    for(int i = 0; i < 5;i++){
        blk1 = readBlockFromDisk((i * 3 + 1),&buff);
        blk2 = readBlockFromDisk((i * 3 + 2),&buff);
        blk3 = readBlockFromDisk((i * 3 + 3),&buff);
        BBsort(blk1,blk,21);
        writeBlockToDisk(blk1,200 + i * 3 + 1,&buff);
        writeBlockToDisk(blk2,200 + i * 3 + 2,&buff);
        writeBlockToDisk(blk3,200 + i * 3 + 3,&buff);
    }
    blk1 = readBlockFromDisk(16,&buff);
    BBsort(blk1,blk,7);
    writeBlockToDisk(blk1,216,&buff);
    freeBlockInBuffer(blk,&buff);
    printf("第一趟：将R化分为3个块一组，组内排序结束，结果写入磁盘块201~216中\n");
    // 第二轮 6路归并
    merge_sort(201,3,16,220);
    printf("第二趟：R采用六路归并排序结束，结果写入磁盘块221~236中\n");
    //S块内排序，每组6块，最后一组2块
    blk = getNewBlockInBuffer(&buff);
    for(int i = 0;i < 5;i++){
        blk1 = readBlockFromDisk((i * 6 + 17),&buff);
        blk2 = readBlockFromDisk((i * 6 + 18),&buff);
        blk3 = readBlockFromDisk((i * 6 + 19),&buff);
        blk4 = readBlockFromDisk((i * 6 + 20),&buff);
        blk5 = readBlockFromDisk((i * 6 + 21),&buff);
        blk6 = readBlockFromDisk((i * 6 + 22),&buff);
        BBsort(blk1,blk,42);
        writeBlockToDisk(blk1,250 + i * 6 + 1,&buff);
        writeBlockToDisk(blk2,250 + i * 6 + 2,&buff);
        writeBlockToDisk(blk3,250 + i * 6 + 3,&buff);
        writeBlockToDisk(blk4,250 + i * 6 + 4,&buff);
        writeBlockToDisk(blk5,250 + i * 6 + 5,&buff);
        writeBlockToDisk(blk6,250 + i * 6 + 6,&buff);
    }
    blk1 = readBlockFromDisk(47,&buff);
    blk2 = readBlockFromDisk(48,&buff);
    BBsort(blk1,blk,14);
    writeBlockToDisk(blk1,281,&buff);
    writeBlockToDisk(blk2,282,&buff);
    freeBlockInBuffer(blk,&buff);
    printf("第一趟：将S化分为6个块一组，组内排序结束，结果写入磁盘块251~282中\n");
    // S6路归并
    merge_sort(251,6,32,300);
    printf("第二趟：S采用六路归并排序结束，结果写入磁盘块301~332中\n");
    return 0;
}
void swapRecord(unsigned char *blk1,unsigned char *blk2,unsigned char *blk)
{
    copyToBlock(blk1,blk);
    copyToBlock(blk2,blk1);
    copyToBlock(blk,blk2);

}

void BBsort(unsigned char *blk1,unsigned char * blk,int total_record){
        int k = 0;
        while(k < total_record){
            int min = getValue(blk1 + k * 8 + 9 * (k / 7),0);
            int mini = k;
            for(int j = k + 1;j < total_record;j++){
                if(getValue(blk1 + j * 8 + 9 * (j / 7),0) < min){
                    mini = j;
                    min = getValue(blk1 + j * 8 + 9 * (j / 7),0);
                }
            }
            if(k != mini){
                // 交换两个block的数据
                swapRecord(blk1 + k * 8 + 9 * (k / 7),blk1 + mini * 8 + 9 * (mini / 7),blk);
                // printf("k:%d,min:%d\n",k,min);
            }
            k++;
        }
}
void merge_sort(int ini_block,int group_size,int total_block,int dest_block){
    unsigned char *blk1,*blk2,*blk3,*blk4,*blk5,*blk6;
    int counter1,counter2,counter3,counter4,counter5,counter6,counter;
    counter1 = counter2 = counter3 = counter4 = counter5 = counter6 = counter = 0;
    blk1 = readBlockFromDisk((ini_block),&buff);
    blk2 = readBlockFromDisk((ini_block + group_size),&buff);
    blk3 = readBlockFromDisk((ini_block + group_size * 2),&buff);
    blk4 = readBlockFromDisk((ini_block + group_size * 3),&buff);
    blk5 = readBlockFromDisk((ini_block + group_size * 4),&buff);
    blk6 = readBlockFromDisk((ini_block + group_size * 5),&buff);
    unsigned char *blk = getNewBlockInBuffer(&buff);
    while(1){
            // 先初始化
        int min,minBlock;
        min = 300;
        minBlock = -1;
        if(counter1 / 8 < group_size && counter1 % 8 != 7){
            if(getValue(blk1 + (counter1 % 8) * 8,0) < min){
                min = getValue(blk1 + (counter1 % 8) * 8,0);
                minBlock = 1;
            }
        }
        if(counter2 / 8 < group_size && counter2 % 8 != 7){
            if(getValue(blk2 + (counter2 % 8) * 8,0) < min){
                min = getValue(blk2 + (counter2 % 8) * 8,0);
                minBlock = 2;
            }
        }
        if(counter3 / 8 < group_size && counter3 % 8 != 7){
            if(getValue(blk3 + (counter3 % 8) * 8,0) < min){
                min = getValue(blk3 + (counter3 % 8) * 8,0);
                minBlock = 3;
            }
        }
        if(counter4 / 8 < group_size && counter4 % 8 != 7){
            if(getValue(blk4 + (counter4 % 8) * 8,0) < min){
                min = getValue(blk4 + (counter4 % 8) * 8,0);
                minBlock = 4;
            }
        }
        if(counter5 / 8 < group_size && counter5 % 8 != 7){
            if(getValue(blk5 + (counter5 % 8) * 8,0) < min){
                min = getValue(blk5 + (counter5 % 8) * 8,0);
                minBlock = 5;
            }
        }
        if(counter6 / 8 < total_block - 5 * group_size && counter6 % 8 != 7){
            if(getValue(blk6 + (counter6 % 8) * 8,0) < min){
                min = getValue(blk6 + (counter6 % 8) * 8,0);
                minBlock = 6;
            }
        }
        // printf("counter: %d min: %d,minBlock:%d\n",counter,min,minBlock);
       // printf("counter1~6:%d,%d,%d,%d,%d,%d\n",counter1,counter2,counter3,counter4,counter5,counter6);
        switch(minBlock){
                case 1:{
                    copyToBlock(blk1 + (counter1 % 8) * 8,blk + (counter % 7) * 8);
                    counter1++;
                    break;
                    }
                case 2:{
                    copyToBlock(blk2 + (counter2 % 8) * 8,blk + (counter % 7) * 8);
                    counter2++;
                    break;
                    }
                case 3:{
                    copyToBlock(blk3 + (counter3 % 8) * 8,blk + (counter % 7) * 8);
                    counter3++;
                    break;
                }
                case 4:{
                    copyToBlock(blk4 + (counter4 % 8) * 8,blk + (counter % 7) * 8);
                    counter4++;
                    break;
                    }
                case 5:{
                    copyToBlock(blk5 + (counter5 % 8) * 8,blk + (counter % 7) * 8);
                    counter5++;
                    break;
                }
                case 6:{
                    copyToBlock(blk6 + (counter6 % 8) * 8,blk + (counter % 7) * 8);
                    counter6++;
                    break;
                }
        }
        ++counter;
        if(counter / 7 != (counter - 1) / 7){
            writeBlockToDisk(blk,dest_block + counter / 7,&buff);
            memset(blk,0,64);
            blk = getNewBlockInBuffer(&buff);
        }
        if(counter1 % 8 == 7 && (counter1 + 1) / 8 < group_size) // 换块
        {
            counter1++;
            // printf("before next block: %d\n",buff.numFreeBlk);
            freeBlockInBuffer(blk1,&buff);
            blk1 = readBlockFromDisk((ini_block) + counter1 / 8,&buff);
            // printf("after next block: %d\n",buff.numFreeBlk);
        }
        if(counter2 % 8 == 7 && (counter2 + 1) / 8 < group_size) // 换块
        {
            counter2++;
            freeBlockInBuffer(blk2,&buff);
            blk2 = readBlockFromDisk((ini_block + group_size) + counter2 / 8,&buff);
        }
        if(counter3 % 8 == 7 && (counter3 + 1) / 8 < group_size) // 换块
        {
            counter3++;
            freeBlockInBuffer(blk3,&buff);
            blk3 = readBlockFromDisk((ini_block + group_size * 2) + counter3 / 8,&buff);
        }
        if(counter4 % 8 == 7 && (counter4 + 1) / 8 < group_size) // 换块
        {
            counter4++;
            freeBlockInBuffer(blk4,&buff);
            blk4 = readBlockFromDisk((ini_block + group_size * 3) + counter4 / 8,&buff);
        }
        if(counter5 % 8 == 7 && (counter5 + 1) / 8 < group_size) // 换块
        {
            counter5++;
            freeBlockInBuffer(blk5,&buff);
            blk5 = readBlockFromDisk((ini_block + group_size * 4) + counter5 / 8,&buff);
        }
        if(counter6 % 8 == 7 && (counter6 + 1) / 8 < total_block - 5 * group_size) // 换块
        {
            counter6++;
            freeBlockInBuffer(blk6,&buff);
            blk6 = readBlockFromDisk((ini_block + group_size * 5) + counter6 / 8,&buff);
        }
        if(counter == total_block * 7){
            freeBlockInBuffer(blk,&buff);
            freeBlockInBuffer(blk1,&buff);
            freeBlockInBuffer(blk2,&buff);
            freeBlockInBuffer(blk3,&buff);
            freeBlockInBuffer(blk4,&buff);
            freeBlockInBuffer(blk5,&buff);
            freeBlockInBuffer(blk6,&buff);
            break;
        }

    }
}
int lab3(){
    // 为S的C属性建立索引，并进行查询，与实验1比较
    // 1.建立索引
    int object = 130;
    printf("基于索引的选择算法S.C = 130\n");
    printf("---------------------------------------------------------------------\n");
    unsigned char* blk;
    unsigned char* blk1 = getNewBlockInBuffer(&buff);
    int i;
    printf("开始为S.C建立索引...\n");
    for(i = 0;i < 32;i++){
        blk = readBlockFromDisk(301 + i,&buff);
        // copyToBlock(blk,blk1 + (i % 7) * 8);
        // 把记录对应的块号记录下来
        for(int j = 0;j < 4;j++){
            *(blk1 + (i % 7) * 8 + j) = *(blk + j);
        }
        *(blk1 + (i % 7) * 8 + 4) = (301 + i) / 100 + '0';
        *(blk1 + (i % 7) * 8 + 5) = (301 + i) / 10 - 10 * ((301 + i) /100) + '0';
        *(blk1 + (i % 7) * 8 + 6) = (301 + i) % 10 + '0';
        if((i + 1) / 7 != i / 7){
            writeBlockToDisk(blk1,350 + ((i + 1) / 7),&buff);
            blk1 = getNewBlockInBuffer(&buff);
            memset(blk1,0,64);
        }
        freeBlockInBuffer(blk,&buff);
    }
    writeBlockToDisk(blk1,350 + (i / 7) + 1,&buff);
    printf("建立索引结束,");
    printf("索引写入磁盘块351~355中\n");
    // printf("freeNum:%d\n",buff.numFreeBlk);
    // 根据索引查询
    int io_counter,counter;
    counter = io_counter = 0;
    int block_num = -1;
    int flag = 0;
    for(int i = 0;i < 5;i++){
        blk = readBlockFromDisk(351 + i,&buff);
        printf("读入索引块%d\n",351+i);
        io_counter++;
        for(int j = 0;j < 7;j++){
            int x = getValue(blk + j * 8,0);
            if(x > 0 && x < object){
                block_num = getValue(blk + j * 8,1);
                continue;
            }
            else if(x >= object){
                flag = 1;
                break;
            }
        }
        freeBlockInBuffer(blk,&buff);
        if(flag == 1) break;
        printf("该索引块未查询到满足要求的元组\n");
    }
    if(flag && block_num > 0){
        printf("查询到的起始块号为:%d\n",block_num);
    }
    else{
        printf("查询结束，未查询到所给条件的元组\n");
        return 0;
    }
    // 3.从块中读取
    flag = 1;
    blk1 = getNewBlockInBuffer(&buff);
    while(flag){
        printf("读入数据块%d\n",block_num);
        blk = readBlockFromDisk(block_num++,&buff);
        io_counter++;
        for(int i = 0;i < 7;i++){
            if(getValue(blk + i * 8,0) == object){
                printf("(C=%d,D=%d)\n",object,getValue(blk + i * 8,1));
                copyToBlock(blk + i * 8,blk1 + (counter % 7) * 8);
                counter++;
                if(counter % 7 == 0){
                    writeBlockToDisk(blk1,360 + counter / 7,&buff);
                    printf("注：结果写入磁盘块%d\n",360 + counter / 7);
                    blk1 = getNewBlockInBuffer(&buff);
                    io_counter++;
                    memset(blk1,0,64);
                }
            }
            else if(getValue(blk + i * 8,0) > object){
                flag = 0;
                break;
            }
        }
        freeBlockInBuffer(blk,&buff);
    }
    if(counter % 7 != 0){
        writeBlockToDisk(blk1,361 + counter / 7,&buff);
        printf("注：结果写入磁盘块%d\n",361 + counter / 7);
        io_counter++;
    }
    printf("满足要求的元组共%d个\n",counter);
    printf("IO读写一共%d次\n",io_counter);
    return 1;
}

int lab4(){
    printf("基于排序的连接算法\n");
    printf("---------------------------------------------------------------------\n");
    // 基于排序的连接
    // R保存在221到236里 S保存在301到332里
    // 保存到351~398里
    unsigned char *blk1,*blk2,*blk3,*blk4;
    blk2 = readBlockFromDisk(301,&buff);
    blk1 = readBlockFromDisk(221,&buff);
    blk3 = getNewBlockInBuffer(&buff);
    int counter1,counter2,counter,result;
    counter1 = counter2 = counter = result =0;
    int offset1;
    offset1 = 0;
    /*
        1.每次比较counter1和counter2所指的数据，相等跳到步骤2，
        否则当前较小的值继续走，若一方走完，跳到步骤3
        2.进行连接，一方指针不动，一方移动,直至不能匹配，写到输出块
        3.读入新块
        4.循环至全部读完
    */
    int value1,value2;
    value1 = getValue(blk1,0);
    value2 = getValue(blk2,0);
    while(value1 > 0 && value2 > 0){
        if(value1 < value2){
            value1 = getNext(blk1,&counter1,0);
        }
        else if(value1 > value2){
            value2 = getNext(blk2,&counter2,1);
        }
        else{ // value1 = value2 比较麻烦，让S(value2)的关系不动
            offset1 = counter1;
            blk4 = readBlockFromDisk(221 + counter1 / 8,&buff);
            while(value1 == value2){
                // 连接并写入
                result++;
                copyToBlock(blk2 + counter2 % 8 * 8,blk3 + counter % 8 * 8);
                counter++;
                if(counter % 8 == 7){
                    writeBlockToDisk(blk3,400 + counter / 8,&buff);
                    printf("注：结果写入磁盘块%d\n",400 + counter / 8);
                    memset(blk3,0,64);
                    blk3 = getNewBlockInBuffer(&buff);
                    counter += 1;
                }
                copyToBlock(blk4 + offset1 % 8 * 8,blk3 + counter % 8 * 8);
                counter++;
                if(counter % 8 == 7){
                    writeBlockToDisk(blk3,400 + counter / 8,&buff);
                    printf("注：结果写入磁盘块%d\n",400 + counter / 8);
                    memset(blk3,0,64);
                    blk3 = getNewBlockInBuffer(&buff);
                    counter += 1;
                }
                // 移动R
                offset1++;
                if((offset1) % 8 == 7){
                    offset1++;
                    if((offset1) / 8 < 16){
                        freeBlockInBuffer(blk4,&buff);
                        memset(blk4,0,64);
                        blk4 = readBlockFromDisk(221 + (offset1) / 8,&buff);
                        value1 = getValue(blk4 + offset1 % 8 * 8,0);
                    }
                    else // 读到了尽头
                        value1 = -1;
                }
                else
                    value1 = getValue(blk4 + offset1 % 8 * 8,0);
            }
            value1 = getValue(blk1 + counter1 % 8 * 8,0);
            value2 = getNext(blk2,&counter2,1);
            if(value2 != value1){ // 因为此时R中的关系与S的下一个值不相等，可以直接利用offset跳转
                if(offset1 / 8 == 16){ // R已经读完了 结束
                    value1 = -1;
                    break;
                }
                freeBlockInBuffer(blk4,&buff);
                memset(blk4,0,64);
                freeBlockInBuffer(blk1,&buff);
                memset(blk1,0,64);
                counter1 = offset1;
                blk1 = readBlockFromDisk(221 + counter1 / 8,&buff);
                value1 = getValue(blk1 + counter1 % 8 * 8,0);
            }
            else{
                freeBlockInBuffer(blk4,&buff);
                memset(blk4,0,64);
            }
        }
    }
    if(*(blk1 - 1) == 1){
        freeBlockInBuffer(blk1,&buff);
        memset(blk1,0,64);
    }
    if(*(blk2 - 1) == 1){
        freeBlockInBuffer(blk2,&buff);
        memset(blk2,0,64);
    }
    if(counter % 8 != 0){
        writeBlockToDisk(blk3,400 + counter / 8,&buff);
        printf("注：结果写入磁盘块%d\n",400 + counter / 8);
        memset(blk3,0,64);
    }
    if(*(blk3 - 1 ) == 1){
        freeBlockInBuffer(blk3,&buff);
        memset(blk3,0,64);
    }
    if(*(blk4 - 1 ) == 1){
        freeBlockInBuffer(blk4,&buff);
        memset(blk4,0,64);
    }
    printf("总共连接%d次\n",result);
    return 1;
}
int lab5(){
    // 交
    printf("基于排序的集合的交运算\n");
    printf("---------------------------------------------------------------------\n");
    unsigned char *blk1,*blk2,*blk3,*blk4;
    blk1 = readBlockFromDisk(221,&buff);
    blk2 = readBlockFromDisk(301,&buff);
    blk3 = getNewBlockInBuffer(&buff);
    memset(blk3,0,64);
    int counter1,counter2,counter,result;
    int offset1 = 0;
    int value1,value2;
    counter1 = counter2 = counter = result =0;
    value1 = getValue(blk1,0);
    value2 = getValue(blk2,0);
    /*
        较小的值的指针进行移动，直至两者相等
    */
    while(value1 > 0 && value2 > 0){
        if(value1 < value2){
            value1 = getNext(blk1,&counter1,0);
        }
        else if(value1 > value2){
            value2 = getNext(blk2,&counter2,1);
        }
        else{ // value1 = value2 比较麻烦，让S的关系不动
            offset1 = counter1;
            blk4 = readBlockFromDisk(221 + counter1 / 8,&buff);
            while(value1 == value2){
                if(getValue(blk4 + offset1 % 8 * 8,1) == getValue(blk2 + counter2 % 8 * 8,1)){
                    result ++;
                    printf("(X=%d,Y=%d)\n",value1,getValue(blk4 + offset1 % 8 * 8,1));
                    copyToBlock(blk4 + offset1 % 8 * 8,blk3 + counter % 8 * 8);
                    counter++;
                    if(counter % 8 == 7){
                        counter++;
                        writeBlockToDisk(blk3,500 + counter / 8,&buff);
                        printf("注：结果写入磁盘块%d\n",500 + counter / 8);
                        memset(blk3,0,64);
                        blk3 = getNewBlockInBuffer(&buff);
                    }
                    break;
                }
                // 移动R
                offset1++;
                if((offset1) % 8 == 7){
                    offset1++;
                    if((offset1) / 8 < 16){
                        freeBlockInBuffer(blk4,&buff);
                        memset(blk4,0,64);
                        blk4 = readBlockFromDisk(221 + (offset1) / 8,&buff);
                        value1 = getValue(blk4 + offset1 % 8 * 8,0);
                    }
                    else // 读到了尽头
                        value1 = -1;
                }
                else
                    value1 = getValue(blk4 + offset1 % 8 * 8,0);
            }
            value2 = getNext(blk2,&counter2,1);
            value1 = getValue(blk1 + counter1 % 8 * 8,0);
            if(value2 != value1){
                if(offset1 / 8 == 16){ // R已经读完了，结束
                    value1 = -1;
                    break;
                }
                // 直接跳转
                counter1 = offset1;
                freeBlockInBuffer(blk4,&buff);
                freeBlockInBuffer(blk1,&buff);
                memset(blk1,0,64);
                memset(blk4,0,64);
                blk1 = readBlockFromDisk(221 + counter1 / 8,&buff);
                value1 = getValue(blk1 + counter1 % 8 * 8,0);
            }
            else{
                freeBlockInBuffer(blk4,&buff);
                memset(blk4,0,64);
            }
        }
    }
    if(*(blk1 - 1) == 1){
        freeBlockInBuffer(blk1,&buff);
        memset(blk1,0,64);
    }
    if(*(blk2 - 1) == 1){
        freeBlockInBuffer(blk2,&buff);
        memset(blk2,0,64);
    }
    if(counter % 8 != 0){
        writeBlockToDisk(blk3,500 + counter / 8 + 1,&buff);
        printf("注：结果写入磁盘块%d\n",500 + counter / 8 + 1);
        memset(blk3,0,64);
    }
    if(*(blk3 - 1 ) == 1){
        freeBlockInBuffer(blk3,&buff);
        memset(blk3,0,64);
    }
    if(*(blk4 - 1 ) == 1){
        freeBlockInBuffer(blk4,&buff);
        memset(blk4,0,64);
    }
    printf("S和R的交集共有%d个元组\n",result);
    return 1;
}
