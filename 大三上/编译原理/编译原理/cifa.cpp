#include <iostream>
#include <string>
#include<fstream>
#include "param.h"
#include "Grammer.h"
#include "global.h"
using namespace std;
ostream& operator<<(ostream& out, TOKEN t);
int isLetter(char ch);
int isDigit(char ch);
int getToken(ofstream& o);
TOKEN* back(char* buffer, int& start, int& end, int type);
int isKey(TOKEN* t, KEY* table[]);
int readTable();
int addToTable(TOKEN* t, ofstream& ofs);
/*
1.读表建立关键字表
2.建立符号表
*/

int main() {
	head->next = NULL;
	readTable();
	ofstream ofs;
	ofs.open("./symbolTable.txt", ios::out);
	if(getToken(ofs) == 0) cout << "file input null";
	ofs.close();
	cout << "词法分析完成" << endl;
	Grammer g;
	g.initial();
	g.work(); // 语法分析器开始工作，生成三地址码。
	g.generate(); // 生成汇编代码
	cout << "生成完毕,生成的汇编代码保存在汇编代码.txt中" << endl;
	return 0;
}

int addToTable(TOKEN* t, ofstream& ofs) {
	int x = getHashKey(t->name, 131, 997);
	TUPLE* tuple = (TUPLE*)malloc(sizeof(TUPLE));
	if (!tuple) return -1;
	tuple->name = t->name;
	tuple->next = NULL;
	tuple->type = -1;
	tuple->value = -1;
	if (ftable[x] == NULL) {
		ftable[x] = tuple;
	}
	else {
		TUPLE* p = ftable[x];
		while (p->next != NULL) {
			if (strcmp(p->name, t->name) == 0) return 0; //返回值为0，表示该标识符已经出现过
			else p = p->next;
		}
		if (strcmp(p->name, t->name) != 0)
			p->next = tuple;
		else return 0;
	}
	ofs << *tuple << endl;
	NODE* h = head;
	while (h->next != NULL) {
		h = h->next;
	}
	NODE* temp = (NODE*)malloc(sizeof(NODE));
	if (temp) {
		temp->next = NULL;
		temp->name = tuple->name;
		h->next = temp;
	}
	else {
		exit(0);
	}
	return 1;
}

int getToken(ofstream& o) {
	ifstream inFile;
	ofstream ofs;
	string path = "./input_code.txt";
	inFile.open(path, ios::in);
	ofs.open("./token.txt", ios::out);
	char* buffer = (char*)malloc(sizeof(char) * N);
	inFile.read(buffer, N);
	int size = inFile.gcount();
	if (size == 0) return 0;
	int start, end, flag = 1;
	end = -1;
	do {
		TOKEN* t;
		end++;
		if (end >= size) break;
		start = end;
		char ch = buffer[start];
		if ((ch == ' ') || (ch == '\n') || (ch == '	')) {
			continue;
		}
		else if (isLetter(ch)) {  //标识符
			do {
				end++;
				ch = buffer[end];
			} while (isLetter(ch) || isDigit(ch));
			t = back(buffer, start, end, ID); //后退一格
		}
		else if (isDigit(ch) == 2) { //整常数
			do {
				end++;
				ch = buffer[end];
			} while (isDigit(ch));
			t = back(buffer, start, end, INT_CONST);
		}
		else if (ch == '"') { //字符串常数
			do {
				end++;
				ch = buffer[end];
			} while (ch != '"');
			end++;
			t = back(buffer, start, end, STRING_CONST);
		}
		else if (ch == '\'') { //字符常数
			if (buffer[end + 2] != '\'') {
				cout << "char error" << endl;
				break;
			}
			end += 3;
			t = back(buffer, start, end, CHAR_CONST);
		}
		else {
			t = (TOKEN*)malloc(sizeof(TOKEN));
			switch (ch) {
			case '+': {
				t->type = PLUS;
				t->name = NULL;
				break;
			}
			case '-': {
				t->type = MINUS;
				t->name = NULL;
				break;
			}
			case '*': {
				t->type = MUL;
				t->name = NULL;
				break;
			}
			case '/': {
				t->type = DIV;
				t->name = NULL;
				break;
			}
			case '=': {
				if (buffer[start + 1] != '=') {
					t->type = ASSIGN;
					t->name = NULL;
				}
				else {
					t->type = EQ;
					t->name = NULL;
					end++;
				}
				break;
			}
			case '>': {
				if (buffer[start + 1] != '=') {
					t->type = GT;
					t->name = NULL;
				}
				else {
					t->type = GE;
					t->name = NULL;
					end++;
				}
				break;
			}
			case '<': {
				if (buffer[start + 1] != '=') {
					t->type = LT;
					t->name = NULL;
				}
				else {
					t->type = LE;
					t->name = NULL;
					end++;
				}
				break;
			}
			case '!': {
				if (buffer[start + 1] != '=') {
					std::cout << "! error" << endl;
					break;
				}
				t->type = NE;
				t->name = NULL;
				end++;
				break;
			}
			case '(': {
				t->type = LPA;
				t->name = NULL;
				break;
			}
			case ')': {
				t->type = RPA;
				t->name = NULL;
				break;
			}
			case '{': {
				t->type = LPAH;
				t->name = NULL;
				break;
			}
			case '}': {
				t->type = RPAH;
				t->name = NULL;
				break;
			}
			case ',': {
				t->type = DH;
				t->name = NULL;
				break;
			}
			case ';': {
				t->type = FH;
				t->name = NULL;
				break;
			}
			case ':': {
				t->type = MH;
				t->name = NULL;
				break;
			}
			case '%': {
				t->type = MOD;
				t->name = NULL;
				break;
			}
			case '0': {
				t->type = ZERO_CONST;
				t->name = NULL;
				break;
			}
			default:
				flag = -1;
				std::cout << "analyze error";
			}
		}
		if (t->type == ID) {
			int x = isKey(t, table);
			if (x > 1) t->type = x;
			else addToTable(t, o);
		}
		// std::cout << *t << endl;
		ofs << *t << endl;
	} while (1);
	inFile.close();
	ofs << "(45,$)";
	ofs.close();
	return flag;
}

/*
回退
*/
TOKEN* back(char* buffer, int& start, int& end, int type) {
	end--;
	TOKEN* t = (TOKEN*)malloc(sizeof(TOKEN));
	if (t == NULL) {
		return NULL;
	}
	else {
		t->type = type;
		t->name = (char*)malloc((end - start + 2) * sizeof(char));
		if (t->name == NULL) {
			return NULL;
		}
		for (int i = 0; i < (end - start + 1); i++) {
			t->name[i] = buffer[i + start];
		}
		t->name[end - start + 1] = '\0';
	}
	return t;
}

int isDigit(char ch) {   //非零数字返回2 零返回1 否则返回0
	if (ch >= '1' && ch <= '9') return 2;
	else if (ch == '0') return 1;
	else return 0;
}

int isLetter(char ch) {
	if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))
		return 1;
	else return 0;
}





/*
标识符若为保留字返回其对应的类型type
否则返回1（ID）
*/
int isKey(TOKEN* t, KEY* table[]) {
	if (t->name == NULL)return -1;
	else {
		int index = getHashKey(t->name, 31, 47);
		KEY* k = table[index];
		while (k != NULL) {
			if (strcmp(t->name, k->name) == 0) {
				return k->type;
			}
			k = k->next;
		}
	}
	return 1;
}

/*
插入成功返回1，标识符已存在返回0，插入失败返回-1
*/


/*
从保留字文件key.txt中读取保留字信息，并通过哈希函数映射保存到table表中
读取失败返回-1，读取成功返回1
*/
int readTable() {
	string str;
	ifstream inFile("./key.txt", ios::in);
	if (!inFile) {
		std::cout << "open file error!" << endl;
		return -1;
	}
	while (getline(inFile, str)) {
		int i, x = 0;
		for (i = 0; i < str.size() && str[i] != ' '; i++) {
			x = (x * 31 + str[i]) % 47;
		}
		char* name = (char*)malloc((i + 1) * sizeof(char));
		KEY* k = (KEY*)malloc(sizeof(KEY));
		if (!k || !name) {
			std::cout << "malloc error" << endl;
			return -1;
		}
		str.copy(name, i, 0);
		name[i] = '\0';
		k->name = name;
		k->type = atoi(str.substr(i + 1, str.size() - i).c_str());
		k->next = NULL;
		if (table[x] == NULL) table[x] = k;
		else {
			KEY* p = table[x];
			while (p->next != NULL) {
				p = p->next;
			}
			p->next = k;
		}
	}
	return 1;
}
ostream& operator<<(ostream& out, TOKEN t) {
	if (t.name == NULL) // 默认值为0
		out << "(" << t.type << "," << "0" << ")";
	else
		out << "(" << t.type << "," << t.name << ")";
	return out;
}