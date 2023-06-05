#pragma once
#include <iostream>
using namespace std;
class Production
{
private:
	string left, right;
public:
	int getLength();
	string getLeft();
	string getRight();
	void setLeft(string l);
	void setRight(string r);
	friend ostream& operator<<(ostream& os, Production pro) {
		os << pro.getLeft() << "->" << pro.getRight();
		return os;
	}
};

