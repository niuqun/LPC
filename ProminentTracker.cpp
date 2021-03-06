#include "ProminentTracker.h"

#define MAX_DIFFERENCE 1000

ProminentTracker::ProminentTracker()
	: leastDetectTime(3), mostMissingTime(1)
{
	peopleIn = 0;
	peopleOut = 0;
	peopleUndefined = 0;
}

ProminentTracker::ProminentTracker(int leastDetect, int mostMissing)
	: leastDetectTime(leastDetect), mostMissingTime(mostMissing)
{
	peopleIn = 0;
	peopleOut = 0;
	peopleUndefined = 0;
}

vector<int> ProminentTracker::FindProminentId(const Prominent &prominent)
{
	double matching_rate[30] = {0};
	vector<int> matched_ids;

	for (int i = 0; i < 30; ++i)
	{
		//matching_rate[i] = MAX_DIFFERENCE;

		if (prominents[i].size() > 0)
		{
			Prominent tmp = prominents[i].back();

			matching_rate[i] = tmp - prominent;
		}
	}

	int max_similarity_index = 0;

	for (int i = 1; i < 30; ++i)
	{
		if (matching_rate[i] > matching_rate[max_similarity_index])
		{
			max_similarity_index = i;
		}
	}


	if (matching_rate[max_similarity_index] < 0.30)
	{
		for (int i = 0; i < 30; ++i)
		{
			if (prominents[i].size() == 0)
			{
				matched_ids.push_back(i);
				break;
			}
		}
	}
	else
	{
		matched_ids.push_back(max_similarity_index);

		if (matching_rate[max_similarity_index] > 0.8)
		{

			for (int i = 0; i < 30; ++i)
			{
				if (i != max_similarity_index)
				{
					if (matching_rate[i] > 0.8)
					{
						matched_ids.push_back(i);
					}
				}
			}
		}
	}

	return matched_ids;
}

bool ProminentTracker::AddProminents(const vector<Prominent> &prominents)
{
	for (int i = 0; i < prominents.size(); ++i)
	{
		vector<int> id = FindProminentId(prominents[i]);


		if (id.size() > 1)
		{
			// several candidates, may need a merge
			vector<Prominent> ps;

			for (int j = 0; j < id.size(); ++j)
			{
				ps.push_back(this->prominents[id[j]].back());
			}
			ps.push_back(prominents[i]);

			// pop back the hollow prominent at the end of each prominents queue
			Prominent hollow = this->prominents[id[0]].back();

			this->prominents[id[0]].pop_back();

			if (AreProminentOverlapped(ps))
			{
				for (int j = 1; j < id.size(); ++j)
				{
					for (int k = 0; k < this->prominents[id[j]].size() - 1; ++k)
					{
						if (this->prominents[id[j]][k].start < hollow.start)
						{
							hollow.start = this->prominents[id[j]][k].start;
						}

						if (this->prominents[id[j]][k].finish > hollow.finish)
						{
							hollow.finish = this->prominents[id[j]][k].finish;
						}

						this->prominents[id[0]].push_back(this->prominents[id[j]][k]);
					}
					this->prominents[id[j]].clear();
				}
			}

			if (prominents[i].start < hollow.start)
			{
				hollow.start = prominents[i].start;
			}

			if (prominents[i].finish > hollow.finish)
			{
				hollow.finish = prominents[i].finish;
			}

			hollow.frame = prominents[i].frame;
			hollow.plane = prominents[i].plane;

			this->prominents[id[0]].push_back(prominents[i]);

			this->prominents[id[0]].push_back(hollow);
		}
		else if (id.size() == 1)
		{
			int t = id[0];
			Prominent hollow;

			if (this->prominents[t].size() == 0)
			{
				hollow.start = prominents[i].start;
				hollow.finish = prominents[i].finish;
				hollow.frame = prominents[i].frame;
				hollow.plane = prominents[i].plane;
			}
			else
			{
				hollow = this->prominents[t].back();
				
				this->prominents[t].pop_back();

				if (prominents[i].start < hollow.start)
				{
					hollow.start = prominents[i].start;
				}

				if (prominents[i].finish > hollow.finish)
				{
					hollow.finish = prominents[i].finish;
				}

				hollow.frame = prominents[i].frame;
				hollow.plane = prominents[i].plane;
			}

			this->prominents[t].push_back(prominents[i]);

			this->prominents[t].push_back(hollow);

		}
		else
		{
			return false;
		}
	}

	return true;
}

bool ProminentTracker::AreProminentOverlapped(vector<Prominent> &ps)
{
	int start_index;
	int finish_index;
	bool are_overlapped = true;

	sort(ps.begin(), ps.end());

	start_index = ps[0].start;
	finish_index = ps[0].finish;

	for (int i = 1; i < ps.size(); ++i)
	{
		if (ps[i].start > finish_index)
		{
			are_overlapped = false;
			break;
		}
		else if (ps[i].start >= start_index)
		{
			finish_index = (ps[i].finish > finish_index) ? ps[i].finish : finish_index;
		}
		else
		{
			start_index = ps[i].start;
			finish_index = (ps[i].finish > finish_index) ? ps[i].finish : finish_index;
		}
	}

	return are_overlapped;
}

int ProminentTracker::SetCurrentFrame(int currentFrame)
{
	peopleIn = 0;
	peopleOut = 0;
	peopleUndefined = 0;

	for (int i = 0; i < 30; ++i)
	{
		if (prominents[i].size() > 0)
		{
			if (currentFrame - prominents[i].back().frame > mostMissingTime)
			{
				// TODO: determine the direction of this prominent
				CalculatePeopleInOut(i);
			}
		}
	}
	
	return 0;
}

void ProminentTracker::CalculatePeopleInOut(int i)
{
	// minimum start point
	int min = 300;
	// maximum end point
	int max = 0;

	this->prominents[i].pop_back();

	int length = prominents[i].size();

	// stores height values of prominents
	int heights[20][274] = {0};

	// stores the height of highest point in each detection point
	int front_height[274] = {0};

	// stores the first starting frame
	int start_frame = this->prominents[i].front().frame;
	int current_frame;

	for (int j = 1; j < this->prominents[i].size(); ++j)
	{
		if (this->prominents[i][j].frame < start_frame)
		{
			start_frame = this->prominents[i][j].frame;
		}
	}


	for (int j = 0; j < length; ++j)
	{
		if (prominents[i][j].start < min)
		{
			min = prominents[i][j].start;
		}

		if (prominents[i][j].finish > max)
		{
			max = prominents[i][j].finish;
		}
	}

	for (int j = 0; j < length; ++j)
	{
		current_frame = this->prominents[i][j].frame;
		
		for (int k = 0; k < prominents[i][j].heightList.size(); ++k)
		{

			if (prominents[i][j].heightList[k] > front_height[k + prominents[i][j].start])
			{
				front_height[k + prominents[i][j].start] =
					prominents[i][j].heightList[k];
			}

			heights[current_frame - start_frame][k + prominents[i][j].start - min] =
				prominents[i][j].heightList[k];
		}
	}


	// detects the number of people walking in parallel
	//vector<int> minimum = CalculateMinimum(front_height, 274);
	vector<segment> heads = CalculateHeads(front_height, 274);

	/*
	if (minimum.size() > 0)
	{
		int start_index = 0;

		for (int i = 0; i < minimum.size(); ++i)
		{
			int end_index = minimum[i] - min;

			Direction d = CalculateDirection(heights, 20, start_index, end_index);

			if (d == In)
			{
				++peopleIn;
			}
			else if (d == Out)
			{
				++peopleOut;
			}
			else
			{
				++peopleUndefined;
			}

			start_index = end_index;
		}
	}
	*/


	if (heads.size() > 0)
	{
		for (int i = 0; i < heads.size(); ++i)
		{
			int start_index = heads[i].start - min;
			int finish_index = heads[i].finish - min;

			Direction d = CalculateDirection(heights, 20, start_index, finish_index);

			if (d == In)
			{
				++peopleIn;
			}
			else if (d == Out)
			{
				++peopleOut;
			}
			else
			{
				++peopleUndefined;
			}
		}
	}

	prominents[i].clear();
}

Direction ProminentTracker::CalculateDirection(int heights[][274], int row,
	int start, int finish)
{
	int in = 0;
	int out = 0;

	for (int i = start; i <= finish; ++i)
	{
		int first_non_zero = 0;
		int max_height = 0;
		int k;

		while (heights[first_non_zero][i] == 0)
		{
			first_non_zero++;
		}

		max_height = first_non_zero;
		for (k = first_non_zero + 1; k < row && heights[k][i] != 0; ++k)
		{
			if (heights[k][i] > heights[max_height][i])
			{
				max_height = k;
			}
		}

		if (max_height - first_non_zero + 1 > (k - first_non_zero) / 2)
		{
			++in;
		}
		else
		{
			++out;
		}
	}

	Direction d;

	if (in > out)
	{
		d = In;
	}
	else if (in < out)
	{
		d = Out;
	}
	else
	{
		d = Undefined;
	}

	return d;
}

vector<int> ProminentTracker::CalculateMinimum(const int data[], int length)
{
	int min_number = 0;

	//int smoothed_height[274] = {0};
	vector<int> smoothed_height;

	// stores the list of mimimum
	vector<int> minimum;

	int first_non_zero = 0;
	int last_non_zero = 0;

	for (int i = 0; i < 274; ++i)
	{
		if (data[i])
		{
			first_non_zero = i;
			break;
		}
	}

	for (int i = 273; i >= 0; --i)
	{
		if (data[i])
		{
			last_non_zero = i;
			break;
		}
	}

	smoothed_height.push_back(data[first_non_zero]);
	for (int i = first_non_zero + 1; i < last_non_zero - 1; ++ i)
	{
		smoothed_height.push_back((2 * data[i] + data[i - 1] + data[i + 1]) / 4);
	}
	smoothed_height.push_back(data[last_non_zero]);

	bool is_down = false;
	int last_minimum = 0;

	if (smoothed_height.size() > 20)
	{
		for (int i = 1; i < smoothed_height.size(); ++i)
		{
			if (smoothed_height[i] - smoothed_height[i - 1] < -8)
			{
				is_down = true;
			}
			else
			{
				if (is_down && smoothed_height[i] - smoothed_height[i - 1] > 8
					&& i - last_minimum > 20)
				{
					minimum.push_back(i - 1 + first_non_zero);
					last_minimum = i;
				}

				is_down = false;
			}
		}
	}

	// push back the end prominent as the last minimum
	minimum.push_back(last_non_zero);

	return minimum;
}

int ProminentTracker::SetLeastDetectTime(int leastTime)
{
	leastDetectTime = leastTime;

	return 0;
}

int ProminentTracker::SetMostMissingTime(int mostTime)
{
	mostMissingTime = mostTime;

	return 0;
}

vector<segment> ProminentTracker::CalculateHeads(const int data[], int length)
{
	vector<segment> heads;

	const int segment_length = 20;

	if (length < segment_length)
	{
		segment s;

		s.start = 0;
		s.finish = length - 1;
		s.average = CalculateAverageHeight(data, s.start, s.finish);
		s.standard_deviation =
			CalculateStandardDeviation(data, s.average, s.start, s.finish);

		heads.push_back(s);
	}
	else
	{
		vector<segment> segments;

		for (int i = 0; i < length - segment_length; ++i)
		{
			segment s;

			s.start = i;
			s.finish = i + segment_length - 1;
			s.average = (int)(CalculateAverageHeight(data, s.start, s.finish) / 20) * 20;
			s.standard_deviation =
				CalculateStandardDeviation(data, s.average, s.start, s.finish);

			segments.push_back(s);
		}

		if (segments.size() < 3)
		{
			if (segments[0].average > segments[1].average)
			{
				heads.push_back(segments[0]);
			}
			else
			{
				heads.push_back(segments[1]);
			}
		}
		else
		{
			bool is_increasing = false;

			for (int i = 1; i < segments.size(); ++i)
			{
				if (segments[i].average > segments[i - 1].average)
				{
					is_increasing = true;
				}
				else if (segments[i].average < segments[i - 1].average)
				{
					if (is_increasing)
					{
						/*if (heads.size() > 0)
						{
							segment s_tmp = heads.back();

							if (s_tmp.finish > segments[i - 1].start)
							{
								heads.pop_back();
								segments[i - 1].start = s_tmp.start;
							}
						}
						heads.push_back(segments[i - 1]);*/

						if (heads.size() > 0)
						{
							segment s_tmp = heads.back();

							int segment_gap;

							if (segments[i - 1].start > 200 || segments[i - 1].finish < 100)
							{
								segment_gap = 30;
							}
							else
							{
								segment_gap = 40;
							}

							if (segments[i - 1].start - s_tmp.finish >= segment_gap)
							{
								heads.push_back(segments[i - 1]);
							}
						}
						else
						{
							heads.push_back(segments[i - 1]);
						}
					}
					is_increasing = false;
				}
				else
				{
					// do nothing
				}
			}
		}
		
	}

	return heads;
}

double ProminentTracker::CalculateAverageHeight(const int data[], int start, int finish)
{
	double average = 0;

	for (int i = start; i <= finish; ++i)
	{
		average += data[i];
	}

	return average / ((double)(finish - start + 1));
}

double ProminentTracker::CalculateStandardDeviation(const int data[],
	double average, int start, int finish)
{
	double standard_deviation = 0;

	for (int i = 0; i <= finish; ++i)
	{
		standard_deviation += (data[i] - average) * (data[i] - average);
	}

	return sqrt(standard_deviation);
}