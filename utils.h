#ifndef LPC_UTIL_H_
#define LPC_UTIL_H_

#include <vector>
using std::vector;

// calculates the number of extremum found in discrete height curve
// returns the number
// heights: height array
int CalculateExtremaNumber(vector<int> &heights);

#endif // LPC_UTIL_H_