#include "dfulogger.h"
#include <fstream>
#include <iostream>
#include  <ctime>

using namespace std;

string Logger::filename = "log.txt";

const void Logger::log(const string& msg)
{
	ofstream file(filename.c_str(), ios::app);
	if (file.is_open())
	{
		time_t now = time(0);
		char* strTime = ctime(&now);
		file << strTime << msg << endl;
		file.close();
	}
}

const void Logger::console(const std::string& msg)
{
	cout << msg << endl;
}