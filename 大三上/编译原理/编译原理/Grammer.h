#pragma once
#include "Production.h"
#include <vector>
#include <map>
#include "global.h"
typedef struct action_table {   //�洢LR������Ķ���
	char action; // ���� ��Լ �ƽ�
	int state; // Ҫ�ƽ�����״̬
	int PNO; // ����ʽ��
}ACT;

typedef struct state_and_word { //LR����ջ
	int state; // ��ǰ������״̬
	int type_code; // ��ǰ����
	string name;
	int TPNO; //��ʶ����ս�����ڴʷ��������еı��
}SC;

// ����ջ����������
typedef struct ana_table {
	string name;
	int value;
	int type;
}ANA;

typedef struct abc {
	int state1, state2; // ��ǰ״̬
	struct abc* next;
};

typedef struct reg { // �Ĵ���
	string name;// ��ǰ�Ĵ������ӵı�����
	int number;
	int state; // ��Ϊ0
};


class Grammer
{
private:
	vector<Production> proList;
	vector<string> proFullList;
public:
	vector<Production> getProList();
	vector<string> getProFullList();
	void addToProList(Production pro);
	void addToFullList(string pro);
	void processCSV(); // old.csv -> new.csv
	void readProduction(); // ��txt�ļ��ж�ȡ����ʽ��Ϊ����ʽ��Ų��洢������list����
	void loadACT(); // ���ط��������ݽṹaction����
	void work(); // ����ƽ���Լ�Ŀ��Ƴ���
	vector<string> P; // ���ս����
	ACT action[100][40];// LR����������ݽṹ
	map<int,int> m; // ʵ��һ��ʵ����ֱ����ӳ���
	int find(string s); // Ѱ�ҷ��ս�����ڴ洢���ս���������е��±꣬��Ӧ+PNUM��Ϊ�����ķ��ս������LR�������е��к�
	map<string, struct reg*> m3;
	struct reg REG[20];// 20���Ĵ��� 
	string getTemp();
	void initial();
	void generate();
	struct reg* getReg(string name);
};

