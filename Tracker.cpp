#include "Tracker.h"
#include "utils.h"


#include <iostream>

Tracker::Tracker()
{
	for (int i = 0; i < 30; i++)
	{
		similarity[i] = 500;
	}
	peopleIn = 0;
	peopleOut = 0;
	currentFrame = 0;

	leastDetectTime = 2;
	mostMissingTime = 10;
}

Tracker::Tracker(int leastDetect, int mostMissing)
{
	for (int i = 0; i < 30; i++)
	{
		similarity[i] = 500;
	}
	peopleIn = 0;
	peopleOut = 0;
	currentFrame = 0;

	leastDetectTime = leastDetect;
	mostMissingTime = mostMissing;
}

int Tracker::FindExtremaId(const Extremum &extremum)
{
	int minIndex = -1;
	int min = 1000;
	int length;

	for (int i = 0; i < 30; i++)
	{
		length = extrema[i].size();
		similarity[i] = 0;

		if (length > 0)
		{
			similarity[i] = extrema[i][length - 1].GetDifference(extremum);
			if (similarity[i] < min)
			{
				min = similarity[i];
				minIndex = i;
			}
		}
	}

	if (min < Extremum::eGap)
	{
		return minIndex;
	}
	else
	{
		for (int i = 0; i < 30; i++)
		{
			if (extrema[i].size() == 0)
			{
				return i;
			}
		}
	}

	return -1;
}

bool Tracker::AddExtremum(const Extremum &extremum)
{
	int index = FindExtremaId(extremum);

	if (index >= 0 && index < 30)
	{
		extrema[index].push_back(extremum);
		return true;
	}
	else
	{
		return false;
	}
}

bool Tracker::AddExtrema(vector<Extremum> &extremaInFrame)
{
	for (unsigned int i = 0; i < extremaInFrame.size(); i++)
	{
		if (!AddExtremum(extremaInFrame[i]))
		{
			return false;
		}
	}

	return true;
}

void Tracker::SetCurrentFrame(int currentFrame)
{
	this->currentFrame = currentFrame;
	CalculatePeopleInOut();
}

void Tracker::CalculatePeopleInOut()
{
	peopleIn = 0;
	peopleOut = 0;
	peopleUndefined = 0;

	for (int i = 0; i < 30; i++)
	{
		unsigned int length = extrema[i].size();

		if (length > 0
			&& currentFrame - extrema[i][length - 1].GetFrame() > mostMissingTime)
		{
			if ((int)length > leastDetectTime)
			{
				vector<int> heightArray;
				int peopleNumber;

				heightArray.push_back(0);
				for (unsigned int j = 0; j < length; j++)
				{
					heightArray.push_back(extrema[i][j].GetExtremumHeight());
				}
				heightArray.push_back(0);

				peopleNumber = CalculateExtremaNumber(heightArray);

				int size = heightArray.size();

				// determine the direction of people
				if (heightArray[1] > heightArray[size - 2])
				{
					peopleOut += peopleNumber;
				}
				else if (heightArray[1] < heightArray[size - 2])
				{
					peopleIn += peopleNumber;
				}
				else
				{
					peopleUndefined += peopleNumber;
				}

				std::cout << "Extremum at " <<
					extrema[i][length - 1].GetExtremumIndex() << std::endl;

			}
			extrema[i].clear();
		}
	}
}