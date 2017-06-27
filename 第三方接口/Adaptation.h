#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <math.h>
#include <Windows.h>
#include <fileapi.h>
#include <stdio.h>
#include <time.h>
using namespace std;

class Adaptation
{
public:
	Adaptation();
	~Adaptation();
public:
	typedef struct
	{
		vector<float>   utilities;
		vector<int> bitrates;
		float gp;
		float Vp;
		int state;
		int lastQuality;
		float placeholderBuffer;
	} bolaState, initialState;


	typedef struct
	{
		vector<float>   utilities;
		float gp;
		float Vp;
	} params;


	typedef struct
	{
		int value;
	} switchRequest;

public:
	vector<float> getUtility(vector<int> );
	params calculateParameters(int, int, vector<int> );
	initialState calculateInitialState(vector<int> );
	int getQualityFromBufferLevel(bolaState, int );
	void setLastHttpRequests(float);
	float getRecentThroughput();
	int getQualityFromThroughput(bolaState, float );
	switchRequest getMaxIndex(vector<int>, int );
	int test(float CurrentBufferLevel, float RecentThroughput);
	void init(vector<int>);

	static map<string, initialState> State;//只能定义为全局的变量
	static vector<float> Requests;
	static int num ;
	int last ;
	vector<int> bitrates;

};

