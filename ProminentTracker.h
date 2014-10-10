#ifndef PROMINENT_TRACKER_H_
#define PROMINENT_TRACKER_H_

#include "Prominent.h"

class ProminentTracker
{
public:
	ProminentTracker();
	ProminentTracker(int leastDetect, int mostMissing);

	// finds the most matching prominent id, returns position
	// if not found, returns the first empty position
	// if no empty position, returns -1
	int FindProminentId(const Prominent &prominent);

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

private:

	// adds a prominent to tracker, returns true if successful, false otherwise
	// prominent: prominent to be added
	bool AddProminent(const Prominent &prominent);

	// counts the newly detected number of people
	// i: the index to which group of prominents to be calculated
	int CalculatePeopleInOut(int i);

private:
	vector<Prominent> prominents[30];

	int peopleIn;
	int peopleOut;
	int peopleUndefined;

	int leastDetectTime;
	int mostMissingTime;
};

#endif // PROMINENT_TRACKER_H_
