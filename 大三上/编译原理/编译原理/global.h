#pragma once
#include <iostream>
using namespace std;
typedef struct token {
	int type;
	char* name;
}TOKEN;

typedef struct tuple {
	char* name;
	int type; // ����
	int value; // ����ֵ
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
extern TUPLE* ftable[M]; // ���ű�
extern KEY* table[G]; // �ؼ��ֱ�
extern int tempNumber;
// ����Ҫ�������
extern NODE* head;