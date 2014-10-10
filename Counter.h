#ifndef LPC_COUNTER_H_
#define LPC_COUNTER_H_

#include <vector>
#include "Extremum.h"
#include "Prominent.h"
#include "ProminentTracker.h"

// counts the number of people in and out
class Counter
{
public:
	// constructor
	// gradient: height gradient
	// laserHeight: the height of laser reader
	// pHeight: the height threshold for prominent to be diminished
	// pWidth: the width threshold for prominent to be diminished
	// eGap: the gap between two extrema
	// leastTime: the least time for a extrema to be counted
	// mostTime: the longest time for a extrema missing
	Counter(int gradient, int laserHeight, int pHeight, int pWidth,
		int eGap, int leastTime, int mostTime);
	
	// counts the number of people
	// height: the height value in a frame
	// length: length of height value
	// frame: frame from which height value comes
	// peopleIn: number of people gets in
	// peopleOut: number of people gets out
	// peopleUndefined: number of people whose direction cannot be decided
	void CountPeople(int height[], int start, int end, int frame,
		int &peopleIn, int &peopleOut, int &peopleUndefined);
private:
	// pre-processes the height data to avoid
	// height: height array
	void PreprocessHeight(int height[], int length);

	// gets prominent from frame
	// height: height value array
	// length: length of height value array
	// frame: the number of frame where height value comes from
	int CalculateProminents(int height[], int start, int end, int frame);

	vector<Prominent> prominentsInFrame;
	ProminentTracker tracker;
	vector<Extremum> extremaInFrame;

	int gradient;
	int laserHeight;
	int prominentHeight;
	int prominentWidth;
};

#endif // LPC_COUNTER_H_