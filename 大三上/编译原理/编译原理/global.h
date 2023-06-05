#pragma once
#include <iostream>
using namespace std;
typedef struct token {
	int type;
	char* name;
}TOKEN;

typedef struct tuple {
	char* name;
	int type; // 类型
	int value; // 属性值
	struct tuple* next;
}TUPLE;

typedef struct key {
	char* name;
	int type;
	struct key* next;
}KEY;

typedef struct node {
	char* name;
	struct node* next;
}NODE;
#define N 2048
#define M 1000
#define G 50
int getHashKey(const char* s, int seed, int prime);
ostream& operator<<(ostream& out, TUPLE t);
extern TUPLE* ftable[M]; // 符号表
extern KEY* table[G]; // 关键字表
extern int tempNumber;
// 满足要求的冗余
extern NODE* head;