#ifndef PARAM_LOADER_H_
#define PARAM_LOADER_H_

#include <fstream>
#include <string>
#include <list>
#include <map>

class ParamLoader
{
public:
	ParamLoader();
	ParamLoader(std::string fileName);

	bool Load(int &param1, int &param2, int &param3, int &param4, int &param5,
		int &param6, int &param7, int &param8, int &param9);
private:
	bool Load(std::map<std::string, int> &nameValue);
	std::string fileName;
};

#endif