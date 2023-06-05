#include "global.h"
TUPLE* ftable[M];
KEY* table[G];
int tempNumber = 0;
NODE* head = (NODE*)malloc(sizeof(NODE));
int getHashKey(const char* s, int seed, int prime) {
	int key = 0;
	for (int i = 0; i < strlen(s); i++) {
		key = (key * seed + s[i]) % prime;
	}
	return key;
}
ostream& operator<<(ostream& out, TUPLE t) {
	// out << "(" << t.name << "," << t.type << "," << t.value << ")";
	out << "name: " << t.name << endl;
	out << "type: " << t.type << endl;
	out << "value: " << t.value;
	return out;
}
