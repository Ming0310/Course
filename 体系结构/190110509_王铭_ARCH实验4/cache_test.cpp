#include <iostream>
#include <time.h>
#include <Windows.h>

using namespace std;

#define ARRAY_SIZE (1 << 28)                                    // test array size is 2^28
#define SIZE_TEST_TIMES 123456789
#define WAY_TEST_TIMES 99999 
typedef unsigned char BYTE;										// define BYTE as one-byte type

BYTE array[ARRAY_SIZE];											// test array
int L1_cache_size = 1 << 15;
int L2_cache_size = 1 << 18;
int L1_cache_block = 64;
int L2_cache_block = 64;
int L1_way_count = 8;
int L2_way_count = 4;
int write_policy = 0;											// 0 for write back ; 1 for write through

// have an access to arrays with L2 Data Cache'size to clear the L1 cache
void Clear_L1_Cache() {
	memset(array, 0, L2_cache_size);
}

// have an access to arrays with ARRAY_SIZE to clear the L2 cache
void Clear_L2_Cache() {
	memset(&array[L2_cache_size + 1], 0, ARRAY_SIZE - L2_cache_size);
}

int test(int size){
	int size_ret;
	char c;
	clock_t begin, end;
	int max = 0;
	int time1;
	for (int i = 0; i < 5; i++)
	{
		int n = (1 << (size + 10 + i));
		cout << "test cache size:" << (1 << (size + i)) << "KB" << endl;
		Clear_L1_Cache();
		Clear_L2_Cache();
		begin = clock();
		for (int j = 0; j < SIZE_TEST_TIMES; ++j)
		{
			c = array[(rand() * rand()) % n];
		}
		end = clock();
		cout << "average time:" << end - begin << "ms" << endl;
		if(i == 0){
			time1 = end - begin;
			size_ret = size + i;
		}
		else{
			int temp = end - begin;
			temp -= time1;
			time1 = end - begin;
			if(temp > max){
				max = temp;
				size_ret = size + i - 1;
			}
		}
	}

	return (1 << size_ret) ;
} 

int test2(int size){
	int time1,size_ret;
	char c;
	clock_t start, end;
	int max = 0;
	for (int i = 0; i < 8; i++){
		Clear_L1_Cache();
		Clear_L2_Cache();
		int x = 1 << (size + i);
		cout << "test block size:" << (1 << (size + i)) << "B" << endl;
		start = clock();
		for (int j = 0; j < x; j++)
			for (int k = 0; k < ARRAY_SIZE; k += x) c = array[k];
		end = clock();
		int temp = end - start;
		cout << "total time :" << temp << "ms" << endl;
		if(i == 0){
			time1 = temp;
		}
		else{
			if(temp - time1 > max){
				max = temp - time1;
				size_ret = size + i - 1;
			}
			time1 = (end - start);
		}
	}
	return (1 << size_ret);
}

int test3(int num,int size){
	int temp,time1,max = 0;
	int num_ret;
	int interval;
	clock_t start,end;
	int group = num;
	for (int i = 0; i < 5; ++i){
		Clear_L1_Cache();
		Clear_L2_Cache();
		interval = size / group;
		cout << "test assosination num is " << group / 2 << endl;
		start = clock();
		for (int j = 0; j < WAY_TEST_TIMES; ++j) for (int k = 0; k < group; k+= 2) memset(&array[k * interval], 0, interval);
		end = clock();
		if(i == 0)
			time1 = end - start;
		else{
			temp = end - start;
			if(temp - time1 > max){
				num_ret = group / 4;
				max = temp - time1;
			}
			time1 = end - start;
		}
		cout << "total time is " << end - start << "ms" << endl; 
		group = group << 1;
	}
	return num_ret;
}


int L1_DCache_Size() {
	cout << "L1_Data_Cache_Test" << endl;
	//add your own code
	test(3);
}

int L2_Cache_Size() {
	cout << "L2_Data_Cache_Test" << endl;
	//add your own code
	test(6);
}

int L1_DCache_Block() {
	cout << "L1_DCache_Block_Test" << endl;
	//add your own code
	test2(1);
}

int L2_Cache_Block() {
	cout << "L2_Cache_Block_Test" << endl;
	//add your own code
	test2(1);
}

int L1_DCache_Way_Count() {
	cout << "L1_DCache_Way_Count" << endl;
	//add your own code
	test3(4,L1_cache_size << 11);
}

int L2_Cache_Way_Count() {
	cout << "L2_Cache_Way_Count" << endl;
	//add your own code
	test3(4,L2_cache_size << 11);
	
}


int main() {
	SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
	L1_cache_size = L1_DCache_Size();
	L2_cache_size = L2_Cache_Size();
	L1_cache_block = L1_DCache_Block();
	L2_cache_block = L2_Cache_Block();
	L1_way_count = L1_DCache_Way_Count();
	L2_way_count = L2_Cache_Way_Count();
	
	cout << "****************************************************************************" << endl;
	cout << "L1_cache_size:"<< (L1_cache_size)<< "KB" << endl;
	cout << "L2_cache_size:" << L2_cache_size << "KB" << endl;
	cout << "L1_cache_block:" << L1_cache_block << "B" << endl;
	cout << "L2_cache_block:" << L2_cache_block << "B" << endl;
	cout << "L1_way_count:" << L1_way_count << endl;
	cout << "L2_way_count:" << L2_way_count << endl;

	system("pause");
	return 0;
 }


