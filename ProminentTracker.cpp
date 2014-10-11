#include "ProminentTracker.h"

#define MAX_DIFFERENCE 1000

ProminentTracker::ProminentTracker()
	: leastDetectTime(1), mostMissingTime(1)
{
	peopleIn = 0;
	peopleOut = 0;
	peopleUndefined = 0;
}

ProminentTracker::ProminentTracker(int leastDetect, int mostMissing)
	: leastDetectTime(leastDetect), mostMissingTime(mostMissing)
{
	peopleIn = 0;
	peopleOut = 0;
	peopleUndefined = 0;
}

int ProminentTracker::FindProminentId(const Prominent &prominent)
{
	int matching_rate[30];

	for (int i = 0; i < 30; ++i)
	{
		matching_rate[i] = MAX_DIFFERENCE;

		if (prominents[i].size() > 0)
		{
			Prominent tmp = prominents[i].back();

			matching_rate[i] = tmp - prominent;
		}
	}

	int min_diff_id = 0;

	for (int i = 1; i < 30; ++i)
	{
		if (matching_rate[i] < matching_rate[min_diff_id])
		{
			min_diff_id = i;
		}
	}

	if (matching_rate[min_diff_id] > 30)
	{
		for (int i = 0; i < 30; ++i)
		{
			if (prominents[i].size() == 0)
			{
				return i;
			}
		}
	}

	return min_diff_id;
}

bool ProminentTracker::AddProminents(const vector<Prominent> &prominents)
{
	for (int i = 0; i < prominents.size(); ++i)
	{
		int id = FindProminentId(prominents[i]);

		if (id != -1)
		{
			this->prominents[id].push_back(prominents[i]);
		}
		else
		{
			return false;
		}
	}

	return true;
}

int ProminentTracker::SetCurrentFrame(int currentFrame)
{
	peopleIn = 0;
	peopleOut = 0;
	peopleUndefined = 0;

	for (int i = 0; i < 30; ++i)
	{
		if (prominents[i].size() > 0)
		{
			if (currentFrame - prominents[i].back().frame > mostMissingTime)
			{
				// TODO: determine the direction of this prominent
				CalculatePeopleInOut(i);
			}
		}
	}
	
	return 0;
}

int ProminentTracker::CalculatePeopleInOut(int i)
{
	// minimum start point
	int min = 300;
	// maximum end point
	int max = 0;

	int length = prominents[i].size();

	// stores height value of prominents
	int heights[20][100] = {0};

	for (int j = 0; j < length; ++j)
	{
		if (prominents[i][j].start < min)
		{
			min = prominents[i][j].start;
		}

		if (prominents[i][j].finish > max)
		{
			max = prominents[i][j].finish;
		}
	}

	for (int j = 0; j < length; ++j)
	{
		for (int k = 0; k < prominents[i][j].heightList.size(); ++k)
		{
			heights[j][k + prominents[i][j].start - min] =
				prominents[i][j].heightList[k];
		}
	}

	// votes for the direction of the prominent
	int in = 0;
	int out = 0;

	for (int j = 0; j < max - min + 1; ++j)
	{
		int max_height = 0;
		int first_non_zero = 0;
		int k;

		while (heights[first_non_zero][j] == 0)
		{
			++first_non_zero;
		}

		for (k = first_non_zero; heights[k][j] != 0; ++k)
		{
			if (heights[k][j] > heights[max_height][j])
			{
				max_height = k;
			}
		}

		if (max_height - first_non_zero + 1 > (k - first_non_zero) / 2)
		{
			++in;
		}
		else
		{
			++out;
		}
	}

	if (in > out)
	{
		++peopleIn;
	}
	else if (in < out)
	{
		++peopleOut;
	}
	else
	{
		++peopleUndefined;
	}

	prominents[i].clear();

	return 0;
}

int ProminentTracker::SetLeastDetectTime(int leastTime)
{
	leastDetectTime = leastTime;

	return 0;
}

int ProminentTracker::SetMostMissingTime(int mostTime)
{
	mostMissingTime = mostTime;

	return 0;
}