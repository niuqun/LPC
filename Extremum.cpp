#include "Extremum.h"

Extremum::Extremum()
{
	frame = 0;
	extremumIndex = 0;
	extremumHeight = 0;
}

Extremum::Extremum(int frame, int index, int height)
{
	this->frame = frame;
	this->extremumIndex = index;
	this->extremumHeight = height;
}

int Extremum::GetDifference(const Extremum &rhs)
{
	int deviation;

	deviation = 100;
	if (extremumHeight - rhs.GetExtremumHeight() < 500
		&& extremumHeight - rhs.GetExtremumHeight() > -500)
	{
		deviation = extremumIndex - rhs.GetExtremumIndex();
		if (deviation < 0)
		{
			deviation = -deviation;
		}
	}

	return deviation;
}