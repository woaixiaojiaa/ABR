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

const int BOLA_NO_CHANGE = 0;
const int BOLA_STATE_STARTUP = 1;
const int BOLA_STATE_STEADY = 2;
const int AVERAGE_THROUGHPUT_SAMPLE_AMOUNT_VOD = 3;

const int MINIMUM_BUFFER_S = 2;
const int BUFFER_TARGET_S = 10;
const int REBUFFER_SAFETY_FACTOR = 5;
const float ThroughOut_SAFETY_FACTOR = 1;
const int Thereshold_Buffer_Time = 10;

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
	vector<float> getUtility(vector<int>);
	params calculateParameters(int, int, vector<int>);
	initialState calculateInitialState(vector<int>);
	int getQualityFromBufferLevel(bolaState, int);
	void setLastHttpRequests(float);
	float getRecentThroughput();
	int getQualityFromThroughput(bolaState, float);
	switchRequest getMaxIndex(vector<int>, int);
	int test(float CurrentBufferLevel, float RecentThroughput, bool is);
	void init(vector<int>);

	static map<string, initialState> State;//只能定义为全局的变量
	static vector<float> Requests;
	static int num;
	int last;
	vector<int> bitrates;

};