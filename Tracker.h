#ifndef LPC_TRACKER_H_
#define LPC_TRACKER_H_

#include <vector>
#include "Extremum.h"
using std::vector;

// keeps the track of extremum
class Tracker
{
public:
	Tracker();
	Tracker(int leastDetect, int mostMissing);

	// finds the most matching group of extrema to add, returns the position
	// if no matching found, returns the first empty group to fit
	// if no empty group, returns -1
	// extremum: extremum to be matched
	int FindExtremaId(const Extremum &extremum);

	// adds an extremum to tracker, returns true if successful, false otherwise
	// extremum: the Extremum to be added
	bool AddExtremum(const Extremum &extremum);

	// adds several extrema to tracker, returns true if successful, false otherwise
	// extrema: extrema to be added
	bool AddExtrema(vector<Extremum> &extremaInFrame);

	// returns people in count after adding extremum or extrema
	int GetPeopleIn() { return peopleIn; }

	// returns people out count after adding extremum or extrema
	int GetPeopleOut() { return peopleOut; }

	// returns the number of people whose direction is undefined
	int GetPeopleUndefined() { return peopleUndefined; }

	// sets the current frame number
	void SetCurrentFrame(int currentFrame);


	// sets the least detect time
	void SetLeastDetectTime(int least) { leastDetectTime = least; }

	// sets the most missing time
	void SetMostMissingTime(int most) { mostMissingTime = most; }
private:
	// counts the new detected people number
	void CalculatePeopleInOut();

	vector<Extremum> extrema[30];
	int similarity[30];
	int peopleIn;
	int peopleOut;
	int peopleUndefined;
	int currentFrame;


	int leastDetectTime;
	int mostMissingTime;
};

#endif // LPC_TRACKER_H_