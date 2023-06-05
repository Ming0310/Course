#pragma once
#include "Grammer.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include "Production.h"
#include <stack>
#include "param.h"
using namespace std;
const int PNUM = 17;
const int COLNUM = 30;
void Grammer::readProduction() {
	string path = "./production.txt";
	ifstream ifs(path, ios::in);
	string line;
	while (getline(ifs, line)) {
		int lend = line.find("->", 0);
		Production pro;
		pro.setLeft(line.substr(0, lend - 1));
		pro.setRight(line.substr(lend + 2, line.size() - 2 - lend));
		addToProList(pro);
		addToFullList(line);
	}
	ifs.close();
	m.insert(pair<int, int>(ID, 0));
	m.insert(pair<int, int>(INT, 2));
	m.insert(pair<int, int>(CHAR, 3));
	m.insert(pair<int, int>(PLUS, 10));
	m.insert(pair<int, int>(MINUS, 11));
	m.insert(pair<int, int>(MUL, 7));
	m.insert(pair<int, int>(DIV, 8));
	m.insert(pair<int, int>(MOD, 9));
	m.insert(pair<int, int>(LPA, 5));
	m.insert(pair<int, int>(RPA, 6));
	m.insert(pair<int, int>(ASSIGN, 12));
	m.insert(pair<int, int>(INT_CONST, 1));
	m.insert(pair<int, int>(LPAH, 14));
	m.insert(pair<int, int>(RPAH, 15));
	m.insert(pair<int, int>(MAIN, 13));
	m.insert(pair<int, int>(FH, 4));
	m.insert(pair<int, int>(JS, 16));


	// for (int i = 0; i < proList.size(); i++) cout << proList[i].getLeft().size() << endl;
}


vector<Production> Grammer::getProList()
{
	return proList;
}

vector<string> Grammer::getProFullList()
{
	return proFullList;
}

void Grammer::addToProList(Production pro)
{
	proList.push_back(pro);
}

void Grammer::addToFullList(string pro)
{
	proFullList.push_back(pro);
}

void Grammer::initial() {
	readProduction(); // 从文件中读取产生式，相当于编号
	processCSV(); // 将软件生成的分析表处理并保存到CSV文件中去。
	loadACT(); // LR分析表存储到ACT数据结构
	for (int i = 0; i < 20; i++) {
		REG[i].state = 0;
	}
}

void Grammer::processCSV() {
	string path1 = "./old.csv";
	string path2 = "./new.csv";
	ifstream ifs(path1, ios::in);
	ofstream ofs(path2, ios::out);
	if (!ifs || !ofs) {
		cout << "打开文件失败" << endl;
	}
	else { // fenxi.csv -> new.csv
		string line;
		string field;
		int row = 0;
		while (getline(ifs, line)) {
			row++;
			istringstream in(line);
			for (int i = 0; i < COLNUM; i++) {
				getline(in, field, ',');
				if (row == 2 && i > PNUM) { // 将非终结符号编码为数字，即数组下标 低效
					P.push_back(field);
				}
				if (field.size() > 0 && field[0] == 's') {
					string s = "s";
					s += field.substr(5, field.size() - 5);
					ofs << s << ",";
				}
				else if (field.size() > 0 && field[0] == 'r') {
					string str = "r ";
					string s = field.substr(7, field.size() - 7);
					// cout << s << endl;
					for (int i = 0; i < proFullList.size(); i++) {
						if (proFullList[i] == s) str += to_string(i);
					}
					ofs << str << ",";
				}
				else {
					ofs << field << ",";
				}
			}
			ofs << endl;
		}
		ifs.close();
		ofs.close();
	}
}

void Grammer::loadACT() {
	string path = "./new.csv";
	ifstream ifs(path, ios::in);
	if (!ifs) cout << "打开文件失败";
	string line,field;
	getline(ifs, line);
	getline(ifs, line);
	int row = 0;
	while (getline(ifs, line)) {
		istringstream in(line);
		for (int i = 0; i < COLNUM; i++) {
			getline(in, field, ',');
			if (i > 0 && i <= PNUM) {
				if (field.size() > 0 && field[0] == 's')
				{
					action[row][i - 1].action = 's';
					action[row][i - 1].state = atoi(field.substr(2, field.size() - 2).c_str());
					// cout << action[row][i - 1].action << action[row][i - 1].state << endl;
				}
				else if (field.size() > 0 && field[0] == 'r') {
					action[row][i - 1].action = 'r';
					action[row][i - 1].PNO = atoi(field.substr(2, field.size() - 2).c_str());
				}
				else if (field == "accept") {
					action[row][i - 1].action = 'a';
				}
				else if (field.size() == 0) action[row][i - 1].action = 'e';
			}
			else if (i > PNUM) {
				if (field.size() > 0) {
					action[row][i - 1].action = 'g';
					action[row][i - 1].state = atoi(field.c_str());
				}
				else {
					action[row][i - 1].action = 'e';
				}
			}
		}
		row++;
	}
	//for (int i = 0; i < row; i++) {
	//	for (int j = 0; j < 48; j++) {
	//		if (action[i][j].action == 's') cout << "s " << action[i][j].state << endl;
	//		else if (action[i][j].action == 'r') cout << "r " << action[i][j].PNO << endl;
	//		else if (action[i][j].action == 'a') cout << "accept" << endl;
	//		else if (action[i][j].action == 'g') cout << "goto " << action[i][j].state << endl;
	//	}
	//}
	ifs.close();
}

void Grammer::work() {
	int pcount = 0;
	ofstream ofs,ofs1,ofs2;
	ofs.open("./outProduction.txt", ios::out);
	ofs2.open("./outTable.txt", ios::out);
	int result = 0;
	stack<SC> s;
	stack<ANA> ana;
	SC initial;
	initial.state = 0;
	s.push(initial);
	ifstream ifs("./token.txt", ios::in);
	string line;
	bool flag = true;
	while (1) {
		if(flag) getline(ifs, line);
		istringstream in(line);
		string field;
		getline(in, field, ',');
		int tpno = atoi(field.substr(1, field.size() - 1).c_str());
		getline(in, field, ',');
		string x = field.substr(0, field.size() - 1);// 属性值
		ACT temp = action[s.top().state][m[tpno]];
		if (temp.action == 's') {
			SC sc;
			sc.state = temp.state;
			sc.type_code = m[tpno];
			sc.TPNO = tpno;
			sc.name = x;
			s.push(sc);
			flag = true;
		}
		else if (temp.action == 'r') {
			// cout << proFullList[temp.PNO] << endl;
			ofs << proFullList[temp.PNO] << endl; // 输出规约用的产生式号
			switch (temp.PNO) {
			case 0: {
				ANA a = ana.top();
				ana.pop();
				int x = getHashKey(a.name.c_str(),131,997);
				TUPLE* t = ftable[x];
				bool flag = false;
				while(t) {
					if (strcmp(t->name, a.name.c_str()) == 0) {
						flag = true;
						t->type = ana.top().type;
						ana.pop();
						break;
					}
					else {
						t = t->next;
					}
				}
				if (flag == false) {
					cout << "error" << endl;
					exit(0);
				}
				break;
			}
			case 1: { // B->INT
				ANA a;
				a.type = INT;
				ana.push(a);
				break;
			}
			case 2: { // B->CHAR
				ANA a;
				a.type = CHAR;
				ana.push(a);
				break;
			}
			case 3: { // C->ID
				ANA a;
				a.name = s.top().name;
				ana.push(a);
				break;
			}
			case 4: { // E -> E + T
				string temp = getTemp();
				ANA name1 = ana.top();
				ana.pop();
				ANA name2 = ana.top();
				ana.pop();
				cout << temp << " = " << name2.name << " + " << name1.name << endl;
				ofs2 << temp << " = " << name2.name << " + " << name1.name << endl;


				ANA a;
				a.name = temp;
				a.value = name2.value + name1.value;
				ana.push(a);
				break;
			}
			case 5: { // E-> E - T
				string temp = getTemp();
				ANA name1 = ana.top();
				ana.pop();
				ANA name2 = ana.top();
				ana.pop();
				cout << temp << " = " << name2.name << " - " << name1.name << endl;
				ofs2 << temp << " = " << name2.name << " - " << name1.name << endl;

				ANA a;
				a.name = temp;
				a.value = name2.value - name1.value;
				ana.push(a);
				break;
			}
			case 6: { // E -> T
				break;
			}
			case 7: { // E -> T * F
				string temp = getTemp();
				ANA name1 = ana.top();
				ana.pop();
				ANA name2 = ana.top();
				ana.pop();
				cout << temp << " = " << name2.name << " * " << name1.name << endl;
				ofs2 << temp << " = " << name2.name << " * " << name1.name << endl;
				ANA a;
				a.name = temp;
				a.value = name2.value * name1.value;
				ana.push(a);
				break;
			}
			case 8: { // E -> T / F
				string temp = getTemp();
				ANA name1 = ana.top();
				ana.pop();
				ANA name2 = ana.top();
				ana.pop();
				cout << temp << " = " << name2.name << " / " << name1.name << endl;
				ofs2 << temp << " = " << name2.name << " / " << name1.name << endl;
				ANA a;
				a.name = temp;
				a.value = name2.value / name1.value;
				ana.push(a);
				break;
			}
			case 9: { // E -> T % F
				ANA a;
				string temp = getTemp();
				ANA name1 = ana.top();
				ana.pop();
				ANA name2 = ana.top();
				ana.pop();
				cout << temp << " = " << name2.name << " % " << name1.name << endl;
				ofs2 << temp << " = " << name2.name << " % " << name1.name << endl;
				a.name = temp;
				a.value = name2.value % name1.value;
				ana.push(a);
				break;
			}
			case 10: {// T -> F
				break;
			}
			case 11: { // F -> (E)
				break;
			}
			case 12: { // F -> ID
				ANA a;
				a.name = s.top().name;
				int x = getHashKey(s.top().name.c_str(), 131, 997);
				TUPLE* t = ftable[x];
				bool flag = false;
				while (t) {
					if (strcmp(t->name, s.top().name.c_str()) == 0) {
						flag = true;
						a.value = t->value;
						a.type = t->type;
						break;
					}
					else {
						t = t->next;
					}
				}
				if (flag == false) {
					cout << "error" << endl;
					exit(0);
				}
				ana.push(a);
				break;
			}
			case 13: { //F -> INT_CONST
				string temp = getTemp();
				cout << temp << " = " << s.top().name << endl;
				ofs2 << temp << " = " << s.top().name << endl;
				ANA a;
				a.name = temp;
				a.value = atoi(s.top().name.c_str());
				a.type = INT;
				ana.push(a);
				break;
			}
			case 14: { // Assign -> ID = G
				string name;
				SC stop1 = s.top();
				s.pop();
				SC stop2 = s.top();
				s.pop();
				name = s.top().name;
				s.push(stop2);
				s.push(stop1);
				int x = getHashKey(name.c_str(), 131, 997);
				TUPLE* t = ftable[x];
				bool flag = false;
				while (t) {
					if (strcmp(t->name, name.c_str()) == 0) {
						flag = true;
						t->value = ana.top().value;
						break;
					}
					else {
						t = t->next;
					}
				}
				if (flag == false) {
					cout << 1 << endl;
					cout << "error" << endl;
					exit(0);
				}
				cout << name << " = " << ana.top().name << endl;
				ofs2 << name << " = " << ana.top().name << endl;
				ana.pop();
				break;
			}
			case 15: { // G -> E
				break;
			}
			}
			if (proList[temp.PNO].getLeft()[0] == 'Y') {
				result++;
				ofs << "Accept a sentence" << endl;
			}
			int x = proList[temp.PNO].getLength(); // 弹出栈的数量
			for (int i = 0; i < x; i++) s.pop();
			x = find(proList[temp.PNO].getLeft()); // 产生式的左部 要压栈
			if (action[s.top().state][x + PNUM].action == 'g')
			{
				flag = false;
				SC sc;
				sc.state = action[s.top().state][x + PNUM].state;
				sc.TPNO = 0;
				sc.type_code = x + PNUM;
				s.push(sc);
			}
			else {
				cout << "Rerror" << endl;
				exit(0);
			}
		}
		else if (temp.action == 'e') {
			cout << "error，不被接受" << endl;
			break;
		}
		else if (temp.action == 'a') {
			ofs << "共识别出" << result << "个句子";
			break;
		}

	}
	ifs.close();
	ofs.close();
	ofs2.close();
	/*
		满足实验三的要求，将符号表可视化··· 可以删
	*/
	ofs1.open("./symbolTable.txt",ios::out);
	NODE* node = head->next;
	while(node != NULL) {
		int k = getHashKey(node->name, 131, 997);
		TUPLE* t = ftable[k];
		while (t) {
			if (strcmp(t->name, node->name) == 0) {
				ofs1 << *t << endl;
				break;
			}
			t = t->next;
		}
		node = node->next;
	}
	ofs1.close();
	cout << "中间生成代码保存到outTable" << endl;
	cout << "更新的符号表见symbolTable" << endl;
}

int Grammer::find(string s) {
	for (int i = 0; i < P.size(); i++) {
		if (s == P[i]) return i;
	}
	return -1; // 未找到
}

string Grammer::getTemp() {
	string s = "t";
	s += to_string(tempNumber++);
	return s;
}

struct reg* Grammer::getReg(string name) {
	map<string, struct reg*>::iterator it;
	if (m3[name]) { // 有寄存器与之关联
		return m3[name];
	}
	for (int i = 0; i < 20; i++) {
		if (REG[i].state == 0) { // 没用过的寄存器
			REG[i].name = name;
			REG[i].state = 1;
			REG[i].number = i;
			m3[name] = &REG[i];
			return &REG[i];
		}
	}
	exit(-1);
	return NULL;
}


void Grammer::generate() {
	ifstream ifs;
	ifs.open("./outTable.txt");
	ofstream ofs;
	ofs.open("./汇编代码.txt");
	string line;
	while (getline(ifs, line)) {
		string dst, op1, op2, op;
		dst = op1 = op2 = "";
		istringstream in(line);
		string field;
		// A = B [+ C]
		getline(in, field, ' '); // A
		dst = field;
		getline(in, field, ' '); // =
		getline(in, field, ' '); // B
		op1 = field;
		getline(in, field, ' ');
		if (field == "+") { // 算术表达式
			op = field;
			getline(in, field, ' ');// C
			op2 = field;
			string name;
			if (op1[0] == 't') { // 第一个操作数有寄存器，用这个作为运算后保存结果的寄存器
				struct reg* reg2 = getReg(op1);
				if (op2[0] == 't') {// 第二个操作数也是寄存器变量
					ofs << "Add " << "R" << reg2->number << "," << "R" << getReg(op2)->number << endl;
				}
				else { // 第二个操作数是变量
					ofs << "Add " << "R" << reg2->number << "," << op2 << endl;
				}
				name = "R" + to_string(reg2->number);
			}
			else { // 第一个操作数是变量
				if (op2[0] == 't') { // 第二个操作数为寄存器
					struct reg* reg2 = getReg(op2);
					ofs << "Add " << "R" << reg2->number << "," << op1 << endl;
					name = "R" + to_string(reg2->number);
				}
				else { // 两个操作数都是变量
					struct reg* reg2 = getReg(op1);
					ofs << "Mov " << "R" << reg2->number << "," << op1 << endl;
					ofs << "Add " << "R" << reg2->number << "," << op2 << endl;
					name = "R" + to_string(reg2->number);
				}
			}
			if (dst[0] == 't') { // 中间变量，需要找一个寄存器
				struct reg* reg1 = getReg(dst);
				ofs << "Mov " << "R" << reg1->number << "," << name << endl;
			}
			else {// 是变量
				ofs << "Mov" << dst << "," << name << endl;
			}
		}
		else if (field == "-") {
			op = field;
			getline(in, field, ' ');// C
			op2 = field;
			string name;
			if (op1[0] == 't') { // 第一个操作数有寄存器，用这个作为运算后保存结果的寄存器
				struct reg* reg2 = getReg(op1);
				if (op2[0] == 't') {// 第二个操作数也是寄存器变量
					ofs << "Sub " << "R" << reg2->number << "," << "R" << getReg(op2)->number << endl;
				}
				else { // 第二个操作数是变量
					ofs << "Sub " << "R" << reg2->number << "," << op2 << endl;
				}
				name = "R" + to_string(reg2->number);
			}
			else { // 第一个操作数是变量
				if (op2[0] == 't') { // 第二个操作数为寄存器
					struct reg* reg2 = getReg(op2);
					ofs << "Sub " << "R" << reg2->number << "," << op1 << endl;
					ofs << "Mul " << "R" << reg2->number << "," << -1 << endl;
					name = "R" + to_string(reg2->number);
				}
				else { // 两个操作数都是变量
					struct reg* reg2 = getReg(op1);
					ofs << "Mov " << "R" << reg2->number << "," << op1 << endl;
					ofs << "Sub " << "R" << reg2->number << "," << op2 << endl;
					name = "R" + to_string(reg2->number);
				}
			}
			if (dst[0] == 't') { // 中间变量，需要找一个寄存器
				struct reg* reg1 = getReg(dst);
				ofs << "Mov " << "R" << reg1->number << "," << name << endl;
			}
			else {// 是变量
				ofs << "Mov" << dst << "," << name << endl;
			}
		}
		else if (field == "*") {
			op = field;
			getline(in, field, ' ');// C
			op2 = field;
			string name;
			if (op1[0] == 't') { // 第一个操作数有寄存器，用这个作为运算后保存结果的寄存器
				struct reg* reg2 = getReg(op1);
				if (op2[0] == 't') {// 第二个操作数也是寄存器变量
					ofs << "Mul " << "R" << reg2->number << "," << "R" << getReg(op2)->number << endl;
				}
				else { // 第二个操作数是变量
					ofs << "Mul " << "R" << reg2->number << "," << op2 << endl;
				}
				name = "R" + to_string(reg2->number);
			}
			else { // 第一个操作数是变量
				if (op2[0] == 't') { // 第二个操作数为寄存器
					struct reg* reg2 = getReg(op2);
					ofs << "Mul " << "R" << reg2->number << "," << op1 << endl;
					name = "R" + to_string(reg2->number);
				}
				else { // 两个操作数都是变量
					struct reg* reg2 = getReg(op1);
					ofs << "Mov " << "R" << reg2->number << "," << op1 << endl;
					ofs << "Mul " << "R" << reg2->number << "," << op2 << endl;
					name = "R" + to_string(reg2->number);
				}
			}
			if (dst[0] == 't') { // 中间变量，需要找一个寄存器
				struct reg* reg1 = getReg(dst);
				ofs << "Mov " << "R" << reg1->number << "," << name << endl;
			}
			else {// 是变量
				ofs << "Mov" << dst << "," << name << endl;
			}
		}
		else if (field == "/"){
			op = field;
			getline(in, field, ' ');// C
			op2 = field;
			string name;
			if (op1[0] == 't') { // 第一个操作数有寄存器，用这个作为运算后保存结果的寄存器
				struct reg* reg2 = getReg(op1);
				if (op2[0] == 't') {// 第二个操作数也是寄存器变量
					ofs << "DIV " << "R" << reg2->number << "," << "R" << getReg(op2)->number << endl;
				}
				else { // 第二个操作数是变量
					ofs << "DIV " << "R" << reg2->number << "," << op2 << endl;
				}
				name = "R" + to_string(reg2->number);
			}
			else { // 第一个操作数是变量
				if (op2[0] == 't') { // 第二个操作数为寄存器
					struct reg* reg = getReg(op1);
					struct reg* reg2 = getReg(op2);
					ofs << "Mov " << "R" << reg->number << "," << op1 << endl;
					ofs << "DIV " << "R" << reg->number << "," << "R" << reg2->number << endl;
					name = "R" + to_string(reg2->number);
				}
				else { // 两个操作数都是变量
					struct reg* reg2 = getReg(op1);
					ofs << "Mov " << "R" << reg2->number << "," << op1 << endl;
					ofs << "Div " << "R" << reg2->number << "," << op2 << endl;
					name = "R" + to_string(reg2->number);
				}
			}
			if (dst[0] == 't') { // 中间变量，需要找一个寄存器
				struct reg* reg1 = getReg(dst);
				ofs << "Mov " << "R" << reg1->number << "," << name << endl;
			}
			else {// 是变量
				ofs << "Mov" << dst << "," << name << endl;
			}
		}
		else { // 赋值语句
			if (dst[0] == 't') {
				struct reg* reg = getReg(dst);
				if (op1[0] == 't') {
					ofs << "Mov " << "R" << reg->number << "," << "R" << getReg(op1)->number << endl;
				}
				else
					ofs << "Mov " << "R" << reg->number << "," << op1 << endl;
			}
			else {
				if(op1[0] == 't')
					ofs << "Mov " << dst << "," << "R" << getReg(op1)->number << endl;
				else
					ofs << "Mov " << dst << "," << op1 << endl;
			}
		}
	}
	ifs.close();
	ofs.close();
}