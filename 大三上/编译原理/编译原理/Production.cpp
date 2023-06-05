#include "Production.h"

string Production::getLeft()
{
    return left;
}

string Production::getRight()
{
    return right;
}

void Production::setLeft(string l)
{
    left = l;
    return;
}

void Production::setRight(string r)
{
    right = r;
    return;
}

int Production::getLength() {
    string s = getRight();
    int l = 0;
    for (int i = 0; i < s.size(); i++) {
        if (s[i] == ' ') l++;
    }
    return l;
}