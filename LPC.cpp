#include <iostream>
#include <fstream>
#include <string>
#include "ParamLoader.h"
#include "Counter.h"
using namespace std;

int Extremum::eGap = 20;

int main()
{
	string fileName;
	fstream fileReader;
	
	//// from which point in frame to start counting
	//int startIndex = 10;
	//// from which point in frame to end counting
	//int endIndex = 220;

	//// prominent width threshold
	//int prominentWidthThre = 16;
	//// prominent height threshold
	//int prominentHeightThre = 1000;
	//// laser reader height
	//int laserHeight = 3900;
	//// distance gradient
	//int gradient = 1;
	//
	//// least gap between tow extrema
	//int extremaGapThre = 20;
	//// least extrema detection time
	//int leastExtremaDetectTime = 2;
	//// longest time in which an extremum is missing
	//int mostExtremaMissingTime = 10;

	int startIndex;
	int endIndex;
	int prominentWidthThre;
	int prominentHeightThre;
	int laserHeight;
	int gradient;
	int extremaGapThre;
	int leastExtremaDetectTime;
	int mostExtremaMissingTime;

	int peopleInNumber;
	int peopleOutNumber;
	int peopleUndefinedNumber;
	int heights[274];
	int frameId;

	cout << "Enter parameter file name: ";
	cin >> fileName;
	ParamLoader loader(fileName);
	if (!loader.Load(startIndex, endIndex, prominentWidthThre, prominentHeightThre,
		laserHeight, gradient, extremaGapThre, leastExtremaDetectTime,
		mostExtremaMissingTime))
	{
		cout << "Load parameters failed" << endl;
		return 1;
	}

	cout << "Enter data file name: ";
	cin >> fileName;

	fileReader.open(fileName, std::ios_base::in);
	if (fileReader.good())
	{
		peopleInNumber = 0;
		peopleOutNumber = 0;
		peopleUndefinedNumber = 0;
		frameId = 1;

		Counter counter(gradient, laserHeight, prominentHeightThre,
			prominentWidthThre, extremaGapThre,
			leastExtremaDetectTime, mostExtremaMissingTime);
		char plane[10];
		int planeNumber;
		char colon;

		while (fileReader.good())
		{
			fileReader >> plane >> planeNumber >> colon;
			for (int i = 0; i < 274; i++)
			{
				fileReader >> heights[i];
			}

			cout << "Processing frame: " << frameId << endl;

			
			if (planeNumber == 0)
			{
				frameId++;
				counter.CountPeople(heights, startIndex, endIndex, frameId,
					peopleInNumber, peopleOutNumber, peopleUndefinedNumber);
			}

			cout << "In: " << peopleInNumber << " Out: "<< peopleOutNumber
				<< " Undefined: " << peopleUndefinedNumber << endl;
		}
	}
	else
	{
		cout << "Open file error" << endl;
	}

	return 0;
}