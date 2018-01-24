/**
** @file : logger.h

** @brief : log error messages in a text file
*/

#ifndef __LOGGER__
#define __LOGGER__

#include <string>

class Logger
{
public:
	static const void log(const std::string& msg);
	static const void console(const std::string& msg);
private:
	static std::string filename; // the name of the file to write the logs
};

#endif