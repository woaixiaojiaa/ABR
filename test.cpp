// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Adaptation.h"

int main()
{	
	Adaptation ad;
	int a[9] = { 250000,500000,700000,1000000,1200000,2000000,4000000,8000000,10000000 };
	vector<int> bitrates(a, a + sizeof(a)/sizeof(int));
	
	float CurrentBufferLevel = 0;
	float RecentThroughput = 0;	

	ad.init(bitrates);
	int k = ad.test(CurrentBufferLevel, RecentThroughput);
	cout << k << endl;
	return 0;
}





