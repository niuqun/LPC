#include "utils.h"

int CalculateExtremaNumber(vector<int> &heights)
{
	int number = 0;
	bool isExtremum = false;
	int difference;
	vector<int> smoothedHeights;

	smoothedHeights.push_back(0);
	for (unsigned int i = 1; i < heights.size() - 1; i++)
	{
		smoothedHeights.push_back(
			heights[i - 1] / 5 + heights[i] * 3 / 5 + heights[i] / 5);
	}
	smoothedHeights.push_back(0);

	for (unsigned int i = 1; i < smoothedHeights.size(); i++)
	{
		difference = smoothedHeights[i] - smoothedHeights[i - 1];

		if (difference > 50)
		{
			isExtremum = true;
		}
		else if (difference < -50)
		{
			if (isExtremum)
			{
				number++;
				isExtremum = false;
			}
		}
		else
		{
			// do nothing
		}
	}
	return number;
}