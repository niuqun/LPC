#ifndef LPC_PROMINENT_H_
#define LPC_PROMINENT_H_

#include <vector>
#include "Extremum.h"
using std::vector;

class Prominent
{
public:
	Prominent();
	Prominent(int start, int finish, int frame,
		int topIndex, int topHeight, int plane);

	// gets extrema from one prominent, returns the number of extrema found
	// extremum: extrema vector found in this prominent
	//int GetExtrema(const Extremum &extremum);

	// gets extrema from list of prominents, returns the number of extrema found
	// if no extrema found, return the middle of this prominent as extremum
	// extremaInProminent: extrema vector found in this prominent
	int GetExtrema(vector<Extremum> &extremaInProminent);

	// fits the height curve to polynomial curve using least square
	// degree: to which degree to fit
	void PolynomialFit(/*int degree*/);

	int start;
	int finish;
	int frame;
	int topIndex;
	int topHeight;
	int plane;
	vector<int> heightList;
};

#endif // LPC_PROMINENT_H_