#include "ParamLoader.h"

ParamLoader::ParamLoader()
{
	fileName.clear();
}

ParamLoader::ParamLoader(std::string fileName)
{
	this->fileName = fileName;
}

bool ParamLoader::Load(int &param1, int &param2, int &param3, int &param4,
	int &param5, int &param6, int &param7, int &param8, int &param9)
{
	std::map<std::string, int> parameters;
	bool loadSucceeded = false;
	std::list<std::string> paraNames;

	paraNames.push_back("startIndex");
	paraNames.push_back("endIndex");
	paraNames.push_back("prominentWidthThre");
	paraNames.push_back("prominentHeightThre");
	paraNames.push_back("laserHeight");
	paraNames.push_back("gradient");
	paraNames.push_back("extremaGapThre");
	paraNames.push_back("leastExtremaDetectTime");
	paraNames.push_back("mostExtremaMissingTime");

	if ((loadSucceeded = Load(parameters)) == true)
	{
		for (std::list<std::string>::iterator it = paraNames.begin();
			it != paraNames.end();
			++it)
		{
			if (parameters.find(*it) == parameters.end())
			{
				loadSucceeded = false;
				break;
			}
		}

		if (loadSucceeded)
		{
			param1 = parameters["startIndex"];
			param2 = parameters["endIndex"];
			param3 = parameters["prominentWidthThre"];
			param4 = parameters["prominentHeightThre"];
			param5 = parameters["laserHeight"];
			param6 = parameters["gradient"];
			param7 = parameters["extremaGapThre"];
			param8 = parameters["leastExtremaDetectTime"];
			param9 = parameters["mostExtremaMissingTime"];
		}
	}

	return loadSucceeded;
}

bool ParamLoader::Load(std::map<std::string, int> &nameValue)
{
	bool loadSucceeded = false;
	std::fstream fReader;
	std::string paramName;
	int paramValue;

	fReader.open(fileName, std::ios_base::in);
	if (fReader.good())
	{
		while (fReader.good())
		{
			fReader >> paramName >> paramValue;
			nameValue.insert(std::make_pair(paramName, paramValue));
		}
		fReader.close();
		loadSucceeded = true;
	}
	
	return loadSucceeded;
}