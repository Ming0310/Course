#pragma once
#include "Production.h"
#include <vector>
#include <map>
#include "global.h"
typedef struct action_table {   //存储LR分析表的动作
	char action; // 动作 规约 移进
	int state; // 要移进到的状态
	int PNO; // 产生式号
}ACT;

typedef struct state_and_word { //LR分析栈
	int state; // 当前所处的状态
	int type_code; // 当前符号
	string name;
	int TPNO; //所识别的终结符号在词法分析器中的编号
}SC;

// 语义栈，保留属性
typedef struct ana_table {
	string name;
	int value;
	int type;
}ANA;

typedef struct abc {
	int state1, state2; // 当前状态
	struct abc* next;
};

typedef struct reg { // 寄存器
	string name;// 当前寄存器链接的变量名
	int number;
	int state; // 空为0
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
	void readProduction(); // 从txt文件中读取产生式，为产生式编号并存储到两个list当中
	void loadACT(); // 加载分析表到数据结构action表中
	void work(); // 完成移进规约的控制程序
	vector<string> P; // 非终结符号
	ACT action[100][40];// LR分析表的数据结构
	map<int,int> m; // 实验一与实验二种别码的映射表
	int find(string s); // 寻找非终结符号在存储非终结符号向量中的下标，对应+PNUM后即为真正的非终结符号在LR分析表中的列号
	map<string, struct reg*> m3;
	struct reg REG[20];// 20个寄存器 
	string getTemp();
	void initial();
	void generate();
	struct reg* getReg(string name);
};

