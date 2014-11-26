#ifndef PROMINENT_TRACKER_H_
#define PROMINENT_TRACKER_H_

#include "Prominent.h"
#include <algorithm>

enum Direction
{
	In,
	Out,
	Undefined
};

struct segment
{
	int start;
	int finish;
	int average;
	double standard_deviation;
};

class ProminentTracker
{
public:
	ProminentTracker();
	ProminentTracker(int leastDetect, int mostMissing);

	// finds the most matching prominent ids, returns position
	// if not found, returns the first empty position
	// if no empty position, returns -1
	vector<int> FindProminentId(const Prominent &prominent);

	// tests if prominents are overlapped with each other
	bool AreProminentOverlapped(vector<Prominent> &ps);

	// adds prominents to tracker, returns true if successful, false otherwise
	// prominents: prominents to be added
	bool AddProminents(const vector<Prominent> &prominents);

	// returns the number of people coming in
	int GetPeopleIn() { return peopleIn; }

	// returns the number of people going out
	int GetPeopleOut() { return peopleOut; }

	// returns the number of people whose directions are not determined
	int GetPeopleUndefined() { return peopleUndefined; }

	// sets current frame number
	int SetCurrentFrame(int currentFrame);

	// sets least detect time
	int SetLeastDetectTime(int leastTime);

	// sets most missing time
	int SetMostMissingTime(int mostTime);

	// calculates the list of minimum points the height
	// data: the height array
	// length: the length of height array
	vector<int> CalculateMinimum(const int data[], int length);

	// returns the number of possible heads
	vector<segment> CalculateHeads(const int data[], int length);

	// returns the average height of a given array
	// data: height array
	// start: starting point
	// finish: end point
	double CalculateAverageHeight(const int data[], int start, int finish);

	// returns the standard deviation of a given array
	// data: height array
	// start: starting point
	// finish: end point
	// average: average value of height array
	double CalculateStandardDeviation(const int data[],
		double average, int start, int finish);

private:

	// adds a prominent to tracker, returns true if successful, false otherwise
	// prominent: prominent to be added
	bool AddProminent(const Prominent &prominent);

	// counts the newly detected number of people
	// i: the index to which group of prominents to be calculated
	void CalculatePeopleInOut(int i);


	// votes for the direction of the pedestrian
	// heights: the height matrix of the height matrix
	// row: row number of height matrix
	// returns the in/out/undefined direction of the pedestrian
	Direction CalculateDirection(int heights[][274], int row, int start, int finish);

private:
	vector<Prominent> prominents[30];

	int peopleIn;
	int peopleOut;
	int peopleUndefined;

	int leastDetectTime;
	int mostMissingTime;
};

#endif // PROMINENT_TRACKER_H_
