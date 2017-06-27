#include "stdafx.h"
#include "Adaptation.h"

int Adaptation::num = 0;
vector<float> Adaptation::Requests;
map<string, Adaptation::initialState> Adaptation::State;

Adaptation::Adaptation()
{
	last = 0;
}


Adaptation::~Adaptation()
{
}

void Adaptation::init(vector<int> bit)
{
	bitrates = bit;
}

vector<float> Adaptation::getUtility(vector<int> my)
{
	vector<int>::iterator itor;

	vector<float> s;

	itor = my.begin();
	while (itor != my.end())
	{
		s.push_back(log(*itor));
		itor++;
	}

	return s;
}


Adaptation::params Adaptation::calculateParameters(int minimumBufferS, int bufferTargetS, vector<int> bitrates) {
	int highestUtilityIndex = -1;
	params temp;

	vector<float> utilities = getUtility(bitrates);

	highestUtilityIndex = utilities.size() - 1;

	float gp = 1 - utilities[0] + (utilities[highestUtilityIndex] - utilities[0]) / (bufferTargetS / minimumBufferS - 1);
	float Vp = minimumBufferS / (utilities[0] + gp - 1);

	temp.Vp = Vp;
	temp.gp = gp;
	temp.utilities = utilities;

	return temp;
}



Adaptation::initialState Adaptation::calculateInitialState(vector<int> bitrates) {
	//vector<int> bitrates = getMediaAvalibleBandwidth();
	params k_params = calculateParameters(MINIMUM_BUFFER_S, BUFFER_TARGET_S, bitrates);

	initialState m_initialState;
	m_initialState.state = BOLA_STATE_STARTUP;
	m_initialState.bitrates = bitrates;
	m_initialState.utilities = k_params.utilities;
	m_initialState.Vp = k_params.Vp;
	m_initialState.gp = k_params.gp;
	m_initialState.lastQuality = 0;
	m_initialState.placeholderBuffer = 0;

	return m_initialState;
}


int Adaptation::getQualityFromBufferLevel(bolaState m_bolaState, int bufferLevel)
{
	const int bitrateCount = m_bolaState.bitrates.size();
	int quality = -1;
	float score = -1;
	for (int i = 0; i < bitrateCount; ++i)
	{
		float s = (m_bolaState.Vp * (m_bolaState.utilities[i] + m_bolaState.gp) - bufferLevel) / m_bolaState.bitrates[i];
		if (s > score)
		{
			score = s;
			quality = i;
		}
	}
	return quality;
}


void Adaptation::setLastHttpRequests(float throught)
{
	if (num < 3)
		Requests.push_back(throught);
	else
		Requests[num%AVERAGE_THROUGHPUT_SAMPLE_AMOUNT_VOD] = throught;
	num++;
}


float Adaptation::getRecentThroughput() {

	float sum = 0;
	if (Requests.size() == 0)
		return 0;
	for (int i = 0; i < Requests.size(); ++i) {
		sum += Requests[i];
	}
	return sum / Requests.size();
}



int Adaptation::getQualityFromThroughput(bolaState k_bolaState, float throughput)
{
	int q = 0;

	const int bitrateCount = k_bolaState.bitrates.size();

	for (int i = 0; i < bitrateCount; ++i)
	{
		if (k_bolaState.bitrates[i] > throughput)
		{
			break;
		}

		q = i;
	}

	return q;
}


Adaptation::switchRequest Adaptation::getMaxIndex(vector<int> bitrates, int CurrentBufferLevel) {

	switchRequest k_switchRequest;
	k_switchRequest.value = BOLA_NO_CHANGE;
	//vector<initialState> State;

	if (State.size() == 0)
	{
		//initialization
		initialState initState = calculateInitialState(bitrates);
		State.insert(make_pair("BOLA", initState));
		//State.push_back(initState);

		int q = 0;
		if (initState.state == BOLA_STATE_STARTUP)
		{
			// initState.state === BOLA_STATE_STARTUP
			//float  initThroughput = getRecentThroughput();//downloadBits/downloadSeconds;
			float  initThroughput = getRecentThroughput();
			//// We don't have information about any download yet
			if (initThroughput == 0)
			{
				return k_switchRequest;
			}

			q = getQualityFromThroughput(initState, initThroughput*ThroughOut_SAFETY_FACTOR);
			initState.lastQuality = q;
			k_switchRequest.value = q;
		}
		return k_switchRequest;
	}// initialization


	initialState bola = State["BOLA"];

	float bufferLevel = CurrentBufferLevel;
	float  recentThroughput = getRecentThroughput();

	if (bufferLevel <= 0.1) {
		// rebuffering occurred, reset placeholder buffer
		bola.placeholderBuffer = 0;
	}

	float effectiveBufferLevel = bufferLevel + bola.placeholderBuffer;

	int bolaQuality = getQualityFromBufferLevel(bola, effectiveBufferLevel);

	if (bola.state == BOLA_STATE_STARTUP)
	{
		// in startup phase, use some throughput estimation

		int q = getQualityFromThroughput(bola, recentThroughput*ThroughOut_SAFETY_FACTOR);

		if (bufferLevel > REBUFFER_SAFETY_FACTOR)
		{
			bola.state = BOLA_STATE_STEADY;

			float wantEffectiveBuffer = 0;

			for (int i = 0; i < q; ++i) {

				float b = bola.Vp * (bola.gp + (bola.bitrates[q] * bola.utilities[i] - bola.bitrates[i] * bola.utilities[q]) / (bola.bitrates[q] - bola.bitrates[i]));
				if (b > wantEffectiveBuffer) {
					wantEffectiveBuffer = b;
				}
			}
			if (wantEffectiveBuffer > bufferLevel) {
				bola.placeholderBuffer = wantEffectiveBuffer - bufferLevel;
			}
		}

		bola.lastQuality = q;
		State["BOLA"] = bola;
		//State.push_back(bola);
		k_switchRequest.value = q;

		return k_switchRequest;
	}

	// steady state
	// we want to avoid oscillations
	// We implement the "BOLA-O" variant: when network bandwidth lies between two encoded bitrate levels, stick to the lowest level.

	if (bola.state == BOLA_STATE_STEADY)
	{
		if (bufferLevel <= REBUFFER_SAFETY_FACTOR)
		{
			bola.state = BOLA_STATE_STARTUP;
			bola.placeholderBuffer = 0;
			State["BOLA"] = bola;
			k_switchRequest.value = BOLA_NO_CHANGE;
			return k_switchRequest;
		}

	}

	if (bolaQuality > bola.lastQuality)
	{
		int q = getQualityFromThroughput(bola, recentThroughput*ThroughOut_SAFETY_FACTOR);
		if (bolaQuality > q)
		{
			// only intervene if we are trying to *increase* quality to an *unsustainable* level

			if (q < bola.lastQuality)
			{
				// we are only avoid oscillations - do not drop below last quality
				q = bola.lastQuality;
			}
			// We are dropping to an encoding bitrate which is a little less than the network bandwidth because bitrate levels are discrete. Quality q might lead to buffer inflation, so we deflate buffer to the level that q gives postive utility. This delay will be added below.
			bolaQuality = q;
		}
	}

	int delaySeconds = 0;

	float wantBufferLevel = bola.Vp * (bola.utilities[bolaQuality] + bola.gp);
	delaySeconds = effectiveBufferLevel - wantBufferLevel;

	if (delaySeconds > 0)
	{
		// First reduce placeholder buffer.
		// Note that this "delay" is the main mechanism of depleting placeholderBuffer - the real buffer is depleted by playback.
		if (delaySeconds > bola.placeholderBuffer)
		{
			bola.placeholderBuffer = 0;
		}
		else
		{
			bola.placeholderBuffer -= delaySeconds;
		}
	}

	bola.lastQuality = bolaQuality;
	State["BOLA"] = bola;
	//metricsModel.updateBolaState(mediaType, bola);

	k_switchRequest.value = bolaQuality;
	return k_switchRequest;
}

int Adaptation::test(float CurrentBufferLevel, float RecentThroughput, bool is)
{
	switchRequest s_switchRequest;
	if (is)
	{
		setLastHttpRequests(RecentThroughput);

		if (CurrentBufferLevel > 0)
			s_switchRequest = getMaxIndex(bitrates, CurrentBufferLevel);
		else
			s_switchRequest.value = 0;

		last = s_switchRequest.value;
	}

	return is ? s_switchRequest.value : last;
}