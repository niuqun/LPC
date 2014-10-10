#include "Prominent.h"

#include "alglib/src/ap.h"
#include "alglib/src/interpolation.h"
using namespace alglib;


const int kMaxSingleLength = 50;


Prominent::Prominent()
{
	start = -1;
	finish = -1;
	frame = 0;
	topIndex = -1;
	topHeight = -1;
	plane = -1;
	heightList.clear();
}

Prominent::Prominent(int start, int finish, int frame,
	int topIndex, int topHeight, int plane)
{
	this->start = start;
	this->finish = finish;
	this->frame = frame;
	this->topIndex = topIndex;
	this->topHeight = topHeight;
	this->plane = plane;
	this->heightList.clear();
}


void Prominent::PolynomialFit(/*int degree*/)
{
	real_1d_array x;
	real_1d_array y;
	ae_int_t length;
	ae_int_t degree;
	ae_int_t info;
	barycentricinterpolant bary;
	polynomialfitreport report;
	double index[100];
	double heights[100];

	length = heightList.size();
	degree = 4;
	for (int i = 0; i < length; i++)
	{
		index[i] = i;
		heights[i] = heightList[i];
	}
	x.setcontent(length, index);
	y.setcontent(length, heights);
	polynomialfit(x, y, length, degree, info, bary, report);

	for (int i = 0; i < length; i++)
	{
		heightList[i] = static_cast<int>(barycentriccalc(bary, i));
	}
}

int Prominent::GetExtrema(vector<Extremum> &extremaInProminent)
{
	bool isExtremum;
	int difference;
	Extremum tmpExtremum;
	int beforeSize;
	int afterSize;

	// polynomial fitted height list
	PolynomialFit();

	beforeSize = extremaInProminent.size();
	isExtremum = false;
	for (unsigned int i = 1; i < heightList.size(); i++)
	{
		difference = heightList[i] - heightList[i - 1];
		if (difference > 0)
		{
			isExtremum = true;
		}
		else if (difference < 0)
		{
			if (isExtremum)
			{
				tmpExtremum.SetFrame(frame);
				tmpExtremum.SetExtremumIndex(i - 1 + start);
				tmpExtremum.SetExtremumHeight(heightList[i - 1]);

				// the extremum cannot be too close to prominent edge
				if (i - 1 > 3 && finish - (i - 1 + start) > 3)
				{
					if (extremaInProminent.size() > 0)
					{
						int size = extremaInProminent.size();
						int t = tmpExtremum.GetExtremumIndex() -
							extremaInProminent[size - 1].GetExtremumIndex();

						if (t < Extremum::eGap)
						{
							// too close the two extrema
						}
						else
						{
							extremaInProminent.push_back(tmpExtremum);
						}
					}
					else
					{
						extremaInProminent.push_back(tmpExtremum);
					}
				}

				isExtremum = false;
			}
		}
		else
		{
			// do nothing
		}
	}

	afterSize = extremaInProminent.size();
	if (beforeSize == afterSize)
	{
		tmpExtremum.SetFrame(frame);
		tmpExtremum.SetExtremumIndex((start + finish) / 2);
		tmpExtremum.SetExtremumHeight(
			heightList[tmpExtremum.GetExtremumIndex() - start]
		);
		extremaInProminent.push_back(tmpExtremum);
	}

	return extremaInProminent.size();
/*
	if (extremaInProminent.size() > 0)
	{
		return extremaInProminent.size();
	}
	else
	{
		tmpExtremum.SetFrame(frame);
		tmpExtremum.SetExtremumIndex((start + finish) / 2);
		tmpExtremum.SetExtremumHeight(
			heightList[tmpExtremum.GetExtremumIndex() - start]
		);
		extremaInProminent.push_back(tmpExtremum);

		return extremaInProminent.size();
	}
	*/
}

vector<Prominent> Prominent::Splits()
{
	vector<Prominent> ps;

	if (heightList.size() > kMaxSingleLength)
	{
		bool down = false;
		int tmp_start = 0;

		for (int i = 1; i < heightList.size(); ++i)
		{
			if (heightList[i] <= heightList[i - 1])
			{
				down = true;
			}
			else
			{
				if (down = true && i - tmp_start > 20)
				{
					Prominent p;

					p.start = tmp_start + start;
					p.finish = i + start - 1;
					p.frame = frame;
					p.plane = plane;
					p.topIndex = (tmp_start + i - 1) / 2;
					p.topHeight = heightList[p.topIndex];

					for (int j = tmp_start; j <= i; ++j)
					{
						p.heightList.push_back(heightList[j]);
					}
					
					down = false;

					tmp_start = i;
					ps.push_back(p);
				}
			}
		}

		// if no turning point(s) found, make this prominent
		if (ps.empty())
		{
			ps.push_back(*this);
		}
	}
	else
	{
		ps.push_back(*this);
	}

	return ps;
}

int Prominent::operator-(const Prominent &r) const
{
	return (abs(start - r.start) + abs(finish - r.finish));
}
