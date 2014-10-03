#ifndef LPC_EXTREMUM_H_
#define LPC_EXTREMUM_H_

class Extremum
{
public:
	Extremum();
	Extremum(int frame, int index, int height);

	// gets the difference between this extremum and rhs, returns differences
	// ranges from 0 ~ eGap
	// rhs: the compared extremum
	int GetDifference(const Extremum &rhs);

	// gets the frame
	int GetFrame() const
	{
		return frame;
	}

	// sets the frame
	void SetFrame(int frameNumber)
	{
		this->frame = frameNumber;
	}

	// gets the extremumIndex
	int GetExtremumIndex() const
	{
		return extremumIndex;
	}

	// sets extremumIndex
	void SetExtremumIndex(int extremumIndex)
	{
		this->extremumIndex = extremumIndex;
	}

	// gets extremumHeight
	int GetExtremumHeight() const
	{
		return extremumHeight;
	}

	// sets extremumHeight
	void SetExtremumHeight(int extremumHeight) 
	{
		this->extremumHeight = extremumHeight;
	}

	// minimum range between two extrema
	static int eGap;
private:
	int frame;
	int extremumIndex;
	int extremumHeight;
};

#endif // LPC_EXTREMUM_H_