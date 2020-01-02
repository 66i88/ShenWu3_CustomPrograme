#pragma once

#define DINFO		1
#define DWARNING	2
#define DERROR		3

#define LOGLIMITLEVEL DINFO
//__RELEASE		or		__DEBUG
#define __DEBUG
//__LOG,open log
#define __LOG

#ifdef __LOG
#define LOG_MSG log_msg
#else
#define OUT_LOG(level,fmt,...)
#endif

#define LOGFILEPATH Unitl::GetAppPathS().append("Log.txt").c_str()

void log_msg(int level, const char* fmt, ...);
