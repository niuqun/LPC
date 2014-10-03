#include "Counter.h"

Counter::Counter(int gradient, int laserHeight, int pHeight, int pWidth,
	int eGap, int leastTime, int mostTime)
{
	this->gradient = gradient;
	this->laserHeight = laserHeight;
	this->prominentHeight = pHeight;
	this->prominentWidth = pWidth;

	Extremum::eGap = eGap;

	tracker.SetLeastDetectTime(leastTime);
	tracker.SetMostMissingTime(mostTime);
}

void Counter::CountPeople(int height[], int start, int end, int frame,
	int &peopleIn, int &peopleOut, int &peopleUndefined)
{
	prominentsInFrame.clear();
	extremaInFrame.clear();

	PreprocessHeight(height, 274);
	CalculateProminents(height, start, end, frame);

	for (unsigned int i = 0; i < prominentsInFrame.size(); i++)
	{
		prominentsInFrame[i].GetExtrema(extremaInFrame);
	}

	tracker.AddExtrema(extremaInFrame);
	tracker.SetCurrentFrame(frame);
	peopleIn += tracker.GetPeopleIn();
	peopleOut += tracker.GetPeopleOut();
	peopleUndefined += tracker.GetPeopleUndefined();
}

void Counter::PreprocessHeight(int height[], int length)
{
	for (int i = 0; i < 274; i++)
	{
		height[i] = laserHeight - height[i]/* * 19 / 25*/;
		if (height[i] < prominentHeight)
		{
			height[i] = 0;
		}
	}
}

int Counter::CalculateProminents(int height[], int start, int end, int frame)
{
	bool detectFrame;
	Prominent tmpProminent;

	detectFrame = false;
	tmpProminent.start = -1;
	tmpProminent.finish = -1;
	tmpProminent.topHeight = -1;
	tmpProminent.topIndex = -1;
	tmpProminent.frame = -1;
	tmpProminent.heightList.clear();

	for (int i = start; i <= end; i++)
	{
		if (height[i] - height[i - 1] > 800)
		{
			if (detectFrame)
			{
				detectFrame = true;
				tmpProminent.start = i;
				tmpProminent.frame = frame;
				tmpProminent.topHeight = height[i];
				tmpProminent.topIndex = i;
				tmpProminent.heightList.clear();
				tmpProminent.heightList.push_back(height[i]);
			}
			else
			{
				detectFrame = true;
				tmpProminent.start = i;
				tmpProminent.frame = frame;
				tmpProminent.topHeight = height[i];
				tmpProminent.topIndex = i;
				tmpProminent.heightList.clear();
				tmpProminent.heightList.push_back(height[i]);
			}
		}
		else if (height[i] - height[i - 1] < -800)
		{
			if (detectFrame && i - 1 - tmpProminent.start >= prominentWidth)
			{
				tmpProminent.finish = i - 1;
				prominentsInFrame.push_back(tmpProminent);
			}
			detectFrame = false;
		}
		else
		{
			if (detectFrame)
			{
				if (height[i] > tmpProminent.topHeight)
				{
					tmpProminent.topIndex = i;
					tmpProminent.topHeight = height[i];
				}
				tmpProminent.heightList.push_back(height[i]);
			}
		}
	}

	return prominentsInFrame.size();
}