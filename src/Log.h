#include <cstdio>
#include <string>

#ifndef LOG_H_
#define LOG_H_

class Log
{
public:
	static void Init(char* file)
	{
		Log::instance = new Log(file);
	}

	static Log* Instance()
	{
		return Log::instance;
	}

	static void Msg(std::string msg)
	{
		Log::instance->write(msg);
	}

	void write(std::string& msg)
	{
		fprintf(logFile, msg.c_str());
	}

	~Log()
	{
		fclose(logFile);
		delete Log::instance;
	}
private:
	static Log* instance;
	FILE* logFile;

	Log(char* file)
	{
		logFile = fopen(file, "w");
	}

};

#endif /* LOG_H_ */
