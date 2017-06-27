// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Adaptation.h"

int main()
{
	Adaptation ad;
	int a[] = { 250000,500000,700000,1000000,1200000,1800000,3000000,8000000,9000000 };//200kb,9Mb
	//int a[9] = { 250000,500000,700000,1000000,1200000,2000000,4000000,8000000,10000000 };
	vector<int> bitrates(a, a + sizeof(a)/sizeof(int));

	DWORD start, last = 0, last0 = 0, end = 0, start1 = 0;
	DWORD last1 = 0;
	bool is = 0;
	int k = 0;
	//tp.bitrates = bitrates;
	end = GetTickCount();

	srand((unsigned)time(NULL));

	//模拟启动,在buffer中已经达到开播的时间			
	/*tp.CurrentBufferLevel = 2;
	tp.RecentThroughput = 400000;	*/

	float CurrentBufferLevel = 2;
	float RecentThroughput = 400000;

	while (1)
	{
		Sleep(500);
		start = GetTickCount();

		if (CurrentBufferLevel < Thereshold_Buffer_Time)
			is = 1;
		else
			is = 0;

		start1 = GetTickCount();

		ad.init(bitrates);
		k = ad.test(CurrentBufferLevel, RecentThroughput, is);

		if (start - last >= 2000) //每2s进行输出操作、
		{
			cout << CurrentBufferLevel << "HJ" << endl;
			cout << a[k] / 1000000.0 << "    " << RecentThroughput / 1000000.0 << endl;
			//fprintf(fp, "%d\t %.1f\n", a[s_switchRequest.value] / 1000000, tp.RecentThroughput / 1000000);
			//fprintf(fp1, "%.1f\n", tp.CurrentBufferLevel);
			last = start;
		}

		//在时间轴上模拟一种稳定的网络环境。。。
		last0 = GetTickCount() - end;
		if (last0 <= 25000)
		{
			RecentThroughput = 500000;
		}

		if (last0 > 25000 && last0 <= 50000)
		{
			RecentThroughput = 1000000;
		}

		if (last0 > 50000 && last0 <= 75000)
		{
			RecentThroughput = 2000000;
		}

		if (last0 > 75000 && last0 <= 100000)
		{
			RecentThroughput = 4000000;
		}

		if (last0 > 100000 && last0 <= 150000)
		{
			RecentThroughput = 8000000;
		}

		if (last0 > 150000 && last0 <= 200000)
		{
			RecentThroughput = 10000000;
		}

		if (last0 > 200000)
		{
			break;
		}

		//模拟周期变化的网络环境
		/*last0 = GetTickCount() - end;
		if (last0 <= 50000)
		{
		tp.RecentThroughput = 500000;
		}

		if (last0 > 50000 && last0 <= 150000)
		{
		tp.RecentThroughput = 2000000;
		}

		if (last0 > 150000 && last0 <= 200000)
		{
		tp.RecentThroughput = 4000000;
		}

		if (last0 > 200000)
		{
		break;
		}*/


		//模拟短时间周期变化的网络
		/*last0 = GetTickCount() - end;
		if (last0 <= 50000)
		{
		tp.RecentThroughput = 500000;
		}

		if (last0 >= 50000 && last0 <= 200000)
		{
		for(int i = 0; i < 15;i++)
		{
		if (last0 >= 50000 + 10000 * i&&last0 <= 55000 + 10000 * i)
		{
		tp.RecentThroughput = 1000000;
		break;
		}
		else
		tp.RecentThroughput = 500000;
		}

		}

		if (last0 > 200000)
		{
		break;
		}*/


		//模拟随机的网络变化
		/*	last0 = GetTickCount() - end;
		if (last0 <= 50000)
		{
		tp.RecentThroughput = (rand() % 201 + 400)*1000;
		}

		if (last0 >= 50000 && last0 <= 75000)
		{
		tp.RecentThroughput = (rand() % 501 + 700) * 1000;
		}

		if (last0 >= 75000 && last0 <= 120000)
		{
		tp.RecentThroughput = (rand() % 100 + 200) * 1000;
		}

		if (last0 >= 120000 && last0 <= 200000)
		{
		tp.RecentThroughput = (rand() % 301 + 500) * 1000;
		}

		if (last0 > 200000)
		{
		break;
		}*/

		//模拟buffer的情况,单位是s
		if (start1 - last1 >= 500)
		{
			if (is)
			{
				CurrentBufferLevel += (RecentThroughput / a[k] - 1) * 500 / 1000.0;
			}
			else
			{
				CurrentBufferLevel -= 500 / 1000.0;
			}

			last1 = start1;
		}

	}

	return 0;
}





