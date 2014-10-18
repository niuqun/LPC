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
	Direction CalculateDirection(int heights[][100], int row, int start, int finish);

private:
	vector<Prominent> prominents[30];

	int peopleIn;
	int peopleOut;
	int peopleUndefined;

	int leastDetectTime;
	int mostMissingTime;
};

#endif // PROMINENT_TRACKER_H_
