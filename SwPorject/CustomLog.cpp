#include "stdafx.h"
#include <ctime>
#include <iostream>
#include "windows.h"
#include <string>
#include <stdarg.h>
#include <debugapi.h>
#include "CustomLog.h"
#include "unitl.h"

#pragma warning(disable:4996)

char *MakeLogStringHead(char *logStr, int level)
{
	time_t u8_time;
	tm *nowTime;
	time(&u8_time);
	nowTime = localtime(&u8_time);
	auto getStr = [&](int x)->std::string {
		std::string strTemp;
		if (x < 10) {
			strTemp.append("0");
		}
		return strTemp.append(std::to_string(x));
	};
	switch (level) {
	case DINFO:
		strcpy(logStr, "[INFO]");
		break;
	case DWARNING:
		strcpy(logStr, "[WARNING]");
		break;
	case DERROR:
		strcpy(logStr, "[ERROR]");
		break;
	}
	sprintf(logStr, "%s[%s-%s %s:%s:%s] ", logStr, getStr(nowTime->tm_mon + 1).c_str(), getStr(nowTime->tm_mday).c_str(),
		getStr(nowTime->tm_hour).c_str(), getStr(nowTime->tm_min).c_str(), getStr(nowTime->tm_sec).c_str());
	return logStr;
}
//
//void log_msg(int level, const char* str)
//{
//	char *dStr = new char[1024];
//	MakeLogStringHead(dStr, level);
//	strcat(dStr, str);
//#ifdef __DEBUG
//	OutputDebugStringA(str);
//#else
//	FILE* logFile;
//	logFile = fopen("E:\\log.txt", "a+");
//	if (logFile)
//	{
//		fseek(logFile, 0, SEEK_END);
//		fwrite(dStr, 1, strlen(dStr), logFile);
//		vfprintf(logFile, fmt, argPtr);
//		fwrite("\r\n", 1, 1, logFile);
//	}
//	fclose(logFile);
//#endif
//}
void log_msg(int level, const char* fmt, ...)
{
	char *dStr = NULL;
	if (level < LOGLIMITLEVEL)
	{
		return;
	}
	dStr = new char[1024];
	memset(dStr, 0, 1024);
	va_list argPtr;
	MakeLogStringHead(dStr, level);
	va_start(argPtr, fmt);

#ifdef __DEBUG
	vsprintf(dStr, fmt, argPtr);
	OutputDebugStringA(dStr);
	OutputDebugStringA("\n");
#else

#ifdef __RELEASE
	FILE* logFile;
	logFile = fopen(LOGFILEPATH, "a+");
	if (logFile)
	{
		fseek(logFile, 0, SEEK_END);
		fwrite(dStr, 1, strlen(dStr), logFile);
		vfprintf(logFile, fmt, argPtr);
		fwrite("\r\n", 1, 1, logFile);
	}
	fclose(logFile);
#endif

#endif // DEBUG
	if (dStr)
	{
		delete dStr;
	}
}

