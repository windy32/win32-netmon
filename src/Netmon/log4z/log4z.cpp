﻿/*
 * Log4z License
 * -----------
 * 
 * Log4z is licensed under the terms of the MIT license reproduced below.
 * This means that Log4z is free software and can be used for both academic
 * and commercial purposes at absolutely no cost.
 * 
 * 
 * ===============================================================================
 * 
 * Copyright (C) 2010-2016 YaweiZhang <yawei.zhang@foxmail.com>.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 * ===============================================================================
 * 
 * (end of COPYRIGHT)
 */

#include "log4z.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <string.h>

#include <string>
#include <vector>
#include <map>
#include <list>
#include <algorithm>


#ifdef WIN32
#include <io.h>
#include <shlwapi.h>
#include <process.h>
#pragma comment(lib, "shlwapi")
#pragma comment(lib, "User32.lib")
#pragma warning(disable:4996)

#else
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include<pthread.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <semaphore.h>
#endif


#ifdef __APPLE__
#include "TargetConditionals.h"
#include <dispatch/dispatch.h>
#if !TARGET_OS_IPHONE
#define LOG4Z_HAVE_LIBPROC
#include <libproc.h>
#endif
#endif



_ZSUMMER_BEGIN
_ZSUMMER_LOG4Z_BEGIN

static const char *const LOG_STRING[]=
{
    "LOG_TRACE",
    "LOG_DEBUG",
    "LOG_INFO ",
    "LOG_WARN ",
    "LOG_ERROR",
    "LOG_ALARM",
    "LOG_FATAL",
};

#ifdef WIN32
const static WORD LOG_COLOR[LOG_LEVEL_FATAL + 1] = {
    0,
    0,
    FOREGROUND_BLUE | FOREGROUND_GREEN,
    FOREGROUND_GREEN | FOREGROUND_RED,
    FOREGROUND_RED,
    FOREGROUND_GREEN,
    FOREGROUND_RED | FOREGROUND_BLUE };
#else

const static char LOG_COLOR[LOG_LEVEL_FATAL + 1][50] = {
    "\e[0m",
    "\e[0m",
    "\e[34m\e[1m",//hight blue
    "\e[33m", //yellow
    "\e[31m", //red
    "\e[32m", //green
    "\e[35m" };
#endif

//////////////////////////////////////////////////////////////////////////
//! Log4zFileHandler
//////////////////////////////////////////////////////////////////////////
class Log4zFileHandler
{
public:
    Log4zFileHandler(){ _file = NULL; }
    ~Log4zFileHandler(){ close(); }
    inline bool isOpen(){ return _file != NULL; }
    inline bool open(const char *path, const char * mod)
    {
        if (_file != NULL){fclose(_file);_file = NULL;}
        _file = fopen(path, mod);
        return _file != NULL;
    }
    inline void close()
    {
        if (_file != NULL){fclose(_file);_file = NULL;}
    }
    inline void write(const char * data, size_t len)
    {
        if (_file && len > 0)
        {
            if (fwrite(data, 1, len, _file) != len)
            {
                close();
            }
        }
    }
    inline void flush(){ if (_file) fflush(_file); }

    inline std::string readLine()
    {
        char buf[500] = { 0 };
        if (_file && fgets(buf, 500, _file) != NULL)
        {
            return std::string(buf);
        }
        return std::string();
    }
    inline const std::string readContent();
	inline bool removeFile(const std::string & path) { return ::remove(path.c_str()) == 0; }
public:
    FILE *_file;
};


//////////////////////////////////////////////////////////////////////////
//! UTILITY
//////////////////////////////////////////////////////////////////////////
static void sleepMillisecond(unsigned int ms);
static tm timeToTm(time_t t);
static bool isSameDay(time_t t1, time_t t2);

static void fixPath(std::string &path);
static void trimLogConfig(std::string &str, std::string extIgnore = std::string());
static std::pair<std::string, std::string> splitPairString(const std::string & str, const std::string & delimiter);


static bool isDirectory(std::string path);
static bool createRecursionDir(std::string path);
static std::string getProcessID();
static std::string getProcessName();



//////////////////////////////////////////////////////////////////////////
//! LockHelper
//////////////////////////////////////////////////////////////////////////
class LockHelper
{
public:
    LockHelper();
    virtual ~LockHelper();

public:
    void lock();
    void unLock();
private:
#ifdef WIN32
    CRITICAL_SECTION _crit;
#else
    pthread_mutex_t  _crit;
#endif
};

//////////////////////////////////////////////////////////////////////////
//! AutoLock
//////////////////////////////////////////////////////////////////////////
class AutoLock
{
public:
    explicit AutoLock(LockHelper & lk):_lock(lk){_lock.lock();}
    ~AutoLock(){_lock.unLock();}
private:
    LockHelper & _lock;
};

//////////////////////////////////////////////////////////////////////////
//! SemHelper
//////////////////////////////////////////////////////////////////////////
class SemHelper
{
public:
    SemHelper();
    virtual ~SemHelper();
public:
    bool create(int initcount);
    bool wait(int timeout = 0);
    bool post();
private:
#ifdef WIN32
    HANDLE _hSem;
#elif defined(__APPLE__)
    dispatch_semaphore_t _semid;
#else
    sem_t _semid;
    bool  _isCreate;
#endif

};



//////////////////////////////////////////////////////////////////////////
//! ThreadHelper
//////////////////////////////////////////////////////////////////////////
#ifdef WIN32
static unsigned int WINAPI  threadProc(LPVOID lpParam);
#else
static void * threadProc(void * pParam);
#endif

class ThreadHelper
{
public:
    ThreadHelper(){_hThreadID = 0;}
    virtual ~ThreadHelper(){}
public:
    bool start();
    bool wait();
    virtual void run() = 0;
private:
    unsigned long long _hThreadID;
#ifndef WIN32
    pthread_t _phtreadID;
#endif
};

#ifdef WIN32
unsigned int WINAPI  threadProc(LPVOID lpParam)
{
    ThreadHelper * p = (ThreadHelper *) lpParam;
    p->run();
    return 0;
}
#else
void * threadProc(void * pParam)
{
    ThreadHelper * p = (ThreadHelper *) pParam;
    p->run();
    return NULL;
}
#endif


//////////////////////////////////////////////////////////////////////////
//! LogData
//////////////////////////////////////////////////////////////////////////
enum LogDataType
{
    LDT_GENERAL,
    LDT_ENABLE_LOGGER,
    LDT_SET_LOGGER_NAME,
    LDT_SET_LOGGER_PATH,
    LDT_SET_LOGGER_LEVEL,
    LDT_SET_LOGGER_FILELINE,
    LDT_SET_LOGGER_DISPLAY,
    LDT_SET_LOGGER_OUTFILE,
    LDT_SET_LOGGER_LIMITSIZE,
	LDT_SET_LOGGER_MONTHDIR,
	LDT_SET_LOGGER_RESERVETIME,
	//    LDT_SET_LOGGER_,
};


//////////////////////////////////////////////////////////////////////////
//! LoggerInfo
//////////////////////////////////////////////////////////////////////////
struct LoggerInfo 
{
    //! attribute
    std::string _key;   //logger key
    std::string _name;    // one logger one name.
    std::string _path;    //path for log file.
    int  _level;        //filter level
    bool _display;        //display to screen 
    bool _outfile;        //output to file
    bool _monthdir;        //create directory per month 
    unsigned int _limitsize; //limit file's size, unit Million byte.
    bool _enable;        //logger is enable 
    bool _fileLine;        //enable/disable the log's suffix.(file name:line number)
	time_t _logReserveTime; //log file reserve time. unit is time second.
    //! runtime info
    time_t _curFileCreateTime;    //file create time
    unsigned int _curFileIndex; //rolling file index
    unsigned int _curWriteLen;  //current file length
    Log4zFileHandler    _handle;        //file handle.
	//!history
	std::list<std::pair<time_t, std::string> > _historyLogs;

    
    LoggerInfo()
    {
        _enable = false; 
        _path = LOG4Z_DEFAULT_PATH; 
        _level = LOG4Z_DEFAULT_LEVEL; 
        _display = LOG4Z_DEFAULT_DISPLAY; 
        _outfile = LOG4Z_DEFAULT_OUTFILE;

        _monthdir = LOG4Z_DEFAULT_MONTHDIR; 
        _limitsize = LOG4Z_DEFAULT_LIMITSIZE;
        _fileLine = LOG4Z_DEFAULT_SHOWSUFFIX;

        _curFileCreateTime = 0;
        _curFileIndex = 0;
        _curWriteLen = 0;
		_logReserveTime = 0;
    }
};


//////////////////////////////////////////////////////////////////////////
//! LogerManager
//////////////////////////////////////////////////////////////////////////
class LogerManager : public ThreadHelper, public ILog4zManager
{
public:
    LogerManager();
    virtual ~LogerManager();
    
    bool configFromStringImpl(std::string content, bool isUpdate);
    //! 读取配置文件并覆写
    virtual bool config(const char* configPath);
    virtual bool configFromString(const char* configContent);

    //! 覆写式创建
    virtual LoggerId createLogger(const char* key);
    virtual bool start();
    virtual bool stop();
    virtual bool prePushLog(LoggerId id, int level);
    virtual bool pushLog(LogData * pLog, const char * file, int line);
    //! 查找ID
    virtual LoggerId findLogger(const char*  key);
    bool hotChange(LoggerId id, LogDataType ldt, int num, const std::string & text);
    virtual bool enableLogger(LoggerId id, bool enable);
    virtual bool setLoggerName(LoggerId id, const char * name);
    virtual bool setLoggerPath(LoggerId id, const char * path);
    virtual bool setLoggerLevel(LoggerId id, int nLevel);
    virtual bool setLoggerFileLine(LoggerId id, bool enable);
    virtual bool setLoggerDisplay(LoggerId id, bool enable);
    virtual bool setLoggerOutFile(LoggerId id, bool enable);
    virtual bool setLoggerLimitsize(LoggerId id, unsigned int limitsize);
    virtual bool setLoggerMonthdir(LoggerId id, bool enable);
	virtual bool setLoggerReserveTime(LoggerId id, time_t sec);
    virtual bool setAutoUpdate(int interval);
    virtual bool updateConfig();
    virtual bool isLoggerEnable(LoggerId id);
    virtual unsigned long long getStatusTotalWriteCount(){return _ullStatusTotalWriteFileCount;}
    virtual unsigned long long getStatusTotalWriteBytes() { return _ullStatusTotalWriteFileBytes; }
    virtual unsigned long long getStatusTotalPushQueue() { return _ullStatusTotalPushLog; }
    virtual unsigned long long getStatusTotalPopQueue() { return _ullStatusTotalPopLog; }
    virtual unsigned int getStatusActiveLoggers();
protected:
    virtual LogData * makeLogData(LoggerId id, int level);
    virtual void freeLogData(LogData * log);
    void showColorText(const char *text, int level = LOG_LEVEL_DEBUG);
    bool onHotChange(LoggerId id, LogDataType ldt, int num, const std::string & text);
    bool openLogger(LogData * log);
    bool closeLogger(LoggerId id);
    bool popLog(LogData *& log);
    virtual void run();
private:

    //! thread status.
    bool        _runing;
    //! wait thread started.
    SemHelper        _semaphore;

    //! hot change name or path for one logger
    int _hotUpdateInterval;
    unsigned int _checksum;

    //! the process info.
    std::string _pid;
    std::string _proName;

    //! config file name
    std::string _configFile;

    //! logger id manager, [logger name]:[logger id].
    std::map<std::string, LoggerId> _ids; 
    // the last used id of _loggers
    LoggerId    _lastId; 
    LoggerInfo _loggers[LOG4Z_LOGGER_MAX];

    //! log queue
    LockHelper    _logLock;
    std::queue<LogData *> _logs;
    std::vector<LogData*> _freeLogDatas;

    //show color lock
    LockHelper _scLock;
    //status statistics
    //write file
    unsigned long long _ullStatusTotalWriteFileCount;
    unsigned long long _ullStatusTotalWriteFileBytes;

    //Log queue statistics
    unsigned long long _ullStatusTotalPushLog;
    unsigned long long _ullStatusTotalPopLog;
    



};




//////////////////////////////////////////////////////////////////////////
//! Log4zFileHandler
//////////////////////////////////////////////////////////////////////////

const std::string Log4zFileHandler::readContent()
{
    std::string content;

    if (!_file)
    {
        return content;
    }
    char buf[BUFSIZ];
    size_t ret = 0;
    do  
    {
        ret = fread(buf, sizeof(char), BUFSIZ, _file);
        content.append(buf, ret);
    }
    while (ret == BUFSIZ);

    return content;
}




//////////////////////////////////////////////////////////////////////////
//! utility
//////////////////////////////////////////////////////////////////////////


void sleepMillisecond(unsigned int ms)
{
#ifdef WIN32
    ::Sleep(ms);
#else
    usleep(1000*ms);
#endif
}

struct tm timeToTm(time_t t)
{
#ifdef WIN32
#if _MSC_VER < 1400 //VS2003
    return * localtime(&t);
#else //vs2005->vs2013->
    struct tm tt = { 0 };
    localtime_s(&tt, &t);
    return tt;
#endif
#else //linux
    struct tm tt = { 0 };
    localtime_r(&t, &tt);
    return tt;
#endif
}

bool isSameDay(time_t t1, time_t t2)
{
    tm tm1 = timeToTm(t1);
    tm tm2 = timeToTm(t2);
    if ( tm1.tm_year == tm2.tm_year
        && tm1.tm_yday == tm2.tm_yday)
    {
        return true;
    }
    return false;
}


void fixPath(std::string &path)
{
    if (path.empty()){return;}
    for (std::string::iterator iter = path.begin(); iter != path.end(); ++iter)
    {
        if (*iter == '\\'){*iter = '/';}
    }
    if (path.at(path.length()-1) != '/'){path.append("/");}
}

static void trimLogConfig(std::string &str, std::string extIgnore)
{
    if (str.empty()){return;}
    extIgnore += "\r\n\t ";
    int length = (int)str.length();
    int posBegin = 0;
    int posEnd = 0;

    //trim utf8 file bom
    if (str.length() >= 3 
        && (unsigned char)str[0] == 0xef
        && (unsigned char)str[1] == 0xbb
        && (unsigned char)str[2] == 0xbf)
    {
        posBegin = 3;
    }

    //trim character 
    for (int i = posBegin; i<length; i++)
    {
        bool bCheck = false;
        for (int j = 0; j < (int)extIgnore.length(); j++)
        {
            if (str[i] == extIgnore[j])
            {
                bCheck = true;
            }
        }
        if (bCheck)
        {
            if (i == posBegin)
            {
                posBegin++;
            }
        }
        else
        {
            posEnd = i + 1;
        }
    }
    if (posBegin < posEnd)
    {
        str = str.substr(posBegin, posEnd-posBegin);
    }
    else
    {
        str.clear();
    }
}

//split
static std::pair<std::string, std::string> splitPairString(const std::string & str, const std::string & delimiter)
{
    std::string::size_type pos = str.find(delimiter.c_str());
    if (pos == std::string::npos)
    {
        return std::make_pair(str, "");
    }
    return std::make_pair(str.substr(0, pos), str.substr(pos+delimiter.length()));
}

static bool parseConfigLine(const std::string& line, int curLineNum, std::string & key, std::map<std::string, LoggerInfo> & outInfo)
{
    std::pair<std::string, std::string> kv = splitPairString(line, "=");
    if (kv.first.empty())
    {
        return false;
    }

    trimLogConfig(kv.first);
    trimLogConfig(kv.second);
    if (kv.first.empty() || kv.first.at(0) == '#')
    {
        return true;
    }

    if (kv.first.at(0) == '[')
    {
        trimLogConfig(kv.first, "[]");
        key = kv.first;
        {
            std::string tmpstr = kv.first;
            std::transform(tmpstr.begin(), tmpstr.end(), tmpstr.begin(), ::tolower);
            if (tmpstr == "main")
            {
                key = "Main";
            }
        }
        std::map<std::string, LoggerInfo>::iterator iter = outInfo.find(key);
        if (iter == outInfo.end())
        {
            LoggerInfo li;
            li._enable = true;
            li._key = key;
            li._name = key;
            outInfo.insert(std::make_pair(li._key, li));
        }
        else
        {
			printf("log4z configure warning: duplicate logger key:[%s] at line: %d \r\n", key.c_str(), curLineNum);
        }
        return true;
    }
    trimLogConfig(kv.first);
    trimLogConfig(kv.second);
    std::map<std::string, LoggerInfo>::iterator iter = outInfo.find(key);
    if (iter == outInfo.end())
    {
		printf("log4z configure warning: not found current logger name:[%s] at line:%d, key=%s, value=%s \r\n", 
			key.c_str(), curLineNum, kv.first.c_str(), kv.second.c_str());
        return true;
    }
    std::transform(kv.first.begin(), kv.first.end(), kv.first.begin(), ::tolower);
    //! path
    if (kv.first == "path")
    {
        iter->second._path = kv.second;
        return true;
    }
    else if (kv.first == "name")
    {
        iter->second._name = kv.second;
        return true;
    }
    std::transform(kv.second.begin(), kv.second.end(), kv.second.begin(), ::tolower);
    //! level
    if (kv.first == "level")
    {
        if (kv.second == "trace" || kv.second == "all")
        {
            iter->second._level = LOG_LEVEL_TRACE;
        }
        else if (kv.second == "debug")
        {
            iter->second._level = LOG_LEVEL_DEBUG;
        }
        else if (kv.second == "info")
        {
            iter->second._level = LOG_LEVEL_INFO;
        }
        else if (kv.second == "warn" || kv.second == "warning")
        {
            iter->second._level = LOG_LEVEL_WARN;
        }
        else if (kv.second == "error")
        {
            iter->second._level = LOG_LEVEL_ERROR;
        }
        else if (kv.second == "alarm")
        {
            iter->second._level = LOG_LEVEL_ALARM;
        }
        else if (kv.second == "fatal")
        {
            iter->second._level = LOG_LEVEL_FATAL;
        }
    }
    //! display
    else if (kv.first == "display")
    {
        if (kv.second == "false" || kv.second == "0")
        {
            iter->second._display = false;
        }
        else
        {
            iter->second._display = true;
        }
    }
    //! output to file
    else if (kv.first == "outfile")
    {
        if (kv.second == "false" || kv.second == "0")
        {
            iter->second._outfile = false;
        }
        else
        {
            iter->second._outfile = true;
        }
    }
    //! monthdir
    else if (kv.first == "monthdir")
    {
        if (kv.second == "false" || kv.second == "0")
        {
            iter->second._monthdir = false;
        }
        else
        {
            iter->second._monthdir = true;
        }
    }
    //! limit file size
    else if (kv.first == "limitsize")
    {
        iter->second._limitsize = atoi(kv.second.c_str());
    }
    //! display log in file line
    else if (kv.first == "fileline")
    {
        if (kv.second == "false" || kv.second == "0")
        {
            iter->second._fileLine = false;
        }
        else
        {
            iter->second._fileLine = true;
        }
    }
    //! enable/disable one logger
    else if (kv.first == "enable")
    {
        if (kv.second == "false" || kv.second == "0")
        {
            iter->second._enable = false;
        }
        else
        {
            iter->second._enable = true;
        }
    }
	//! set reserve time 
	else if (kv.first == "reserve")
	{
		iter->second._logReserveTime = atoi(kv.second.c_str());
	}
    return true;
}

static bool parseConfigFromString(std::string content, std::map<std::string, LoggerInfo> & outInfo)
{

    std::string key;
    int curLine = 1;
    std::string line;
    std::string::size_type curPos = 0;
    if (content.empty())
    {
        return true;
    }
    do
    {
        std::string::size_type pos = std::string::npos;
        for (std::string::size_type i = curPos; i < content.length(); ++i)
        {
            //support linux/unix/windows LRCF
            if (content[i] == '\r' || content[i] == '\n')
            {
                pos = i;
                break;
            }
        }
        line = content.substr(curPos, pos - curPos);
        parseConfigLine(line, curLine, key, outInfo);
        curLine++;

        if (pos == std::string::npos)
        {
            break;
        }
        else
        {
            curPos = pos+1;
        }
    } while (1);
    return true;
}



bool isDirectory(std::string path)
{
#ifdef WIN32
    return PathIsDirectoryA(path.c_str()) ? true : false;
#else
    DIR * pdir = opendir(path.c_str());
    if (pdir == NULL)
    {
        return false;
    }
    else
    {
        closedir(pdir);
        pdir = NULL;
        return true;
    }
#endif
}



bool createRecursionDir(std::string path)
{
    if (path.length() == 0) return true;
    std::string sub;
    fixPath(path);

    std::string::size_type pos = path.find('/');
    while (pos != std::string::npos)
    {
        std::string cur = path.substr(0, pos-0);
        if (cur.length() > 0 && !isDirectory(cur))
        {
            bool ret = false;
#ifdef WIN32
            ret = CreateDirectoryA(cur.c_str(), NULL) ? true : false;
#else
            ret = (mkdir(cur.c_str(), S_IRWXU|S_IRWXG|S_IRWXO) == 0);
#endif
            if (!ret)
            {
                return false;
            }
        }
        pos = path.find('/', pos+1);
    }

    return true;
}

std::string getProcessID()
{
    std::string pid = "0";
    char buf[260] = {0};
#ifdef WIN32
    DWORD winPID = GetCurrentProcessId();
    sprintf(buf, "%06u", winPID);
    pid = buf;
#else
    sprintf(buf, "%06d", getpid());
    pid = buf;
#endif
    return pid;
}


std::string getProcessName()
{
    std::string name = "process";
    char buf[260] = {0};
#ifdef WIN32
    if (GetModuleFileNameA(NULL, buf, 259) > 0)
    {
        name = buf;
    }
    std::string::size_type pos = name.rfind("\\");
    if (pos != std::string::npos)
    {
        name = name.substr(pos+1, std::string::npos);
    }
    pos = name.rfind(".");
    if (pos != std::string::npos)
    {
        name = name.substr(0, pos-0);
    }

#elif defined(LOG4Z_HAVE_LIBPROC)
    proc_name(getpid(), buf, 260);
    name = buf;
    return name;;
#else
    sprintf(buf, "/proc/%d/cmdline", (int)getpid());
    Log4zFileHandler i;
    i.open(buf, "rb");
    if (!i.isOpen())
    {
        return name;
    }
    name = i.readLine();
    i.close();

    std::string::size_type pos = name.rfind("/");
    if (pos != std::string::npos)
    {
        name = name.substr(pos+1, std::string::npos);
    }
#endif


    return name;
}






//////////////////////////////////////////////////////////////////////////
// LockHelper
//////////////////////////////////////////////////////////////////////////
LockHelper::LockHelper()
{
#ifdef WIN32
    InitializeCriticalSection(&_crit);
#else
    //_crit = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&_crit, &attr);
    pthread_mutexattr_destroy(&attr);
#endif
}
LockHelper::~LockHelper()
{
#ifdef WIN32
    DeleteCriticalSection(&_crit);
#else
    pthread_mutex_destroy(&_crit);
#endif
}

void LockHelper::lock()
{
#ifdef WIN32
    EnterCriticalSection(&_crit);
#else
    pthread_mutex_lock(&_crit);
#endif
}
void LockHelper::unLock()
{
#ifdef WIN32
    LeaveCriticalSection(&_crit);
#else
    pthread_mutex_unlock(&_crit);
#endif
}
//////////////////////////////////////////////////////////////////////////
// SemHelper
//////////////////////////////////////////////////////////////////////////
SemHelper::SemHelper()
{
#ifdef WIN32
    _hSem = NULL;
#elif defined(__APPLE__)
    _semid = NULL;
#else
    _isCreate = false;
#endif

}
SemHelper::~SemHelper()
{
#ifdef WIN32
    if (_hSem != NULL)
    {
        CloseHandle(_hSem);
        _hSem = NULL;
    }
#elif defined(__APPLE__)
    if (_semid)
    {
        dispatch_release(_semid);
        _semid = NULL;
    }
#else
    if (_isCreate)
    {
        _isCreate = false;
        sem_destroy(&_semid);
    }
#endif

}

bool SemHelper::create(int initcount)
{
    if (initcount < 0)
    {
        initcount = 0;
    }
#ifdef WIN32
    if (initcount > 64)
    {
        return false;
    }
    _hSem = CreateSemaphore(NULL, initcount, 64, NULL);
    if (_hSem == NULL)
    {
        return false;
    }
#elif defined(__APPLE__)
    _semid = dispatch_semaphore_create(initcount);
    if (!_semid)
    {
        return false;
    }
#else
    if (sem_init(&_semid, 0, initcount) != 0)
    {
        return false;
    }
    _isCreate = true;
#endif

    return true;
}
bool SemHelper::wait(int timeout)
{
#ifdef WIN32
    if (timeout <= 0)
    {
        timeout = INFINITE;
    }
    if (WaitForSingleObject(_hSem, timeout) != WAIT_OBJECT_0)
    {
        return false;
    }
#elif defined(__APPLE__)
    if (dispatch_semaphore_wait(_semid, dispatch_time(DISPATCH_TIME_NOW, timeout*1000)) != 0)
    {
        return false;
    }
#else
    if (timeout <= 0)
    {
        return (sem_wait(&_semid) == 0);
    }
    else
    {
        struct timeval tm;
        gettimeofday(&tm, NULL);
        long long endtime = tm.tv_sec *1000 + tm.tv_usec/1000 + timeout;
        do 
        {
            sleepMillisecond(50);
            int ret = sem_trywait(&_semid);
            if (ret == 0)
            {
                return true;
            }
            struct timeval tv_cur;
            gettimeofday(&tv_cur, NULL);
            if (tv_cur.tv_sec*1000 + tv_cur.tv_usec/1000 > endtime)
            {
                return false;
            }

            if (ret == -1 && errno == EAGAIN)
            {
                continue;
            }
            else
            {
                return false;
            }
        } while (true);
        return false;
    }
#endif
    return true;
}

bool SemHelper::post()
{
#ifdef WIN32
    return ReleaseSemaphore(_hSem, 1, NULL) ? true : false;
#elif defined(__APPLE__)
    return dispatch_semaphore_signal(_semid) == 0;
#else
    return (sem_post(&_semid) == 0);
#endif

}

//////////////////////////////////////////////////////////////////////////
//! ThreadHelper
//////////////////////////////////////////////////////////////////////////
bool ThreadHelper::start()
{
#ifdef WIN32
    unsigned long long ret = _beginthreadex(NULL, 0, threadProc, (void *) this, 0, NULL);

    if (ret == -1 || ret == 0)
    {
		printf("log4z: create log4z thread error! \r\n");
        return false;
    }
    _hThreadID = ret;
#else
    int ret = pthread_create(&_phtreadID, NULL, threadProc, (void*)this);
    if (ret != 0)
    {
		printf("log4z: create log4z thread error! \r\n");
        return false;
    }
#endif
    return true;
}

bool ThreadHelper::wait()
{
#ifdef WIN32
    if (WaitForSingleObject((HANDLE)_hThreadID, INFINITE) != WAIT_OBJECT_0)
    {
        return false;
    }
#else
    if (pthread_join(_phtreadID, NULL) != 0)
    {
        return false;
    }
#endif
    return true;
}

//////////////////////////////////////////////////////////////////////////
//! LogerManager
//////////////////////////////////////////////////////////////////////////
LogerManager::LogerManager()
{
    _runing = false;
    _lastId = LOG4Z_MAIN_LOGGER_ID;
    _hotUpdateInterval = 0;

    _ullStatusTotalPushLog = 0;
    _ullStatusTotalPopLog = 0;
    _ullStatusTotalWriteFileCount = 0;
    _ullStatusTotalWriteFileBytes = 0;
    
    _pid = getProcessID();
    _proName = getProcessName();
    _loggers[LOG4Z_MAIN_LOGGER_ID]._enable = true;
    _ids[LOG4Z_MAIN_LOGGER_KEY] = LOG4Z_MAIN_LOGGER_ID;
    _loggers[LOG4Z_MAIN_LOGGER_ID]._key = LOG4Z_MAIN_LOGGER_KEY;
    _loggers[LOG4Z_MAIN_LOGGER_ID]._name = LOG4Z_MAIN_LOGGER_KEY;

}
LogerManager::~LogerManager()
{
    stop();
}


LogData * LogerManager::makeLogData(LoggerId id, int level)
{
    LogData * pLog = NULL;
    if (true)
    {
        if (!_freeLogDatas.empty())
        {
            AutoLock l(_logLock);
            if (!_freeLogDatas.empty())
            {
                pLog = _freeLogDatas.back();
                _freeLogDatas.pop_back();
            }
        }
        if (pLog == NULL)
        {
            pLog = new LogData();
        }
    }
    //append precise time to log
    if (true)
    {
        pLog->_id = id;
        pLog->_level = level;
        pLog->_type = LDT_GENERAL;
        pLog->_typeval = 0;
        pLog->_contentLen = 0;
#ifdef WIN32
        FILETIME ft;
        GetSystemTimeAsFileTime(&ft);
        unsigned long long now = ft.dwHighDateTime;
        now <<= 32;
        now |= ft.dwLowDateTime;
        now /= 10;
        now -= 11644473600000000ULL;
        now /= 1000;
        pLog->_time = now / 1000;
        pLog->_precise = (unsigned int)(now % 1000);
#else
        struct timeval tm;
        gettimeofday(&tm, NULL);
        pLog->_time = tm.tv_sec;
        pLog->_precise = tm.tv_usec / 1000;
#endif
    }

    //format log
    if (true)
    {
        tm tt = timeToTm(pLog->_time);

        pLog->_contentLen = sprintf(pLog->_content, "%d-%02d-%02d %02d:%02d:%02d.%03u %s ",
            tt.tm_year + 1900, tt.tm_mon + 1, tt.tm_mday, tt.tm_hour, tt.tm_min, tt.tm_sec, pLog->_precise,
            LOG_STRING[pLog->_level]);
        if (pLog->_contentLen < 0)
        {
            pLog->_contentLen = 0;
        }
    }
    return pLog;
}
void LogerManager::freeLogData(LogData * log)
{
    
    if (_freeLogDatas.size() < 200)
    {
        AutoLock l(_logLock);
        _freeLogDatas.push_back(log);
    }
    else
    {
        delete log;
    }
}

void LogerManager::showColorText(const char *text, int level)
{

#if defined(WIN32) && defined(LOG4Z_OEM_CONSOLE)
    char oem[LOG4Z_LOG_BUF_SIZE] = { 0 };
    CharToOemBuffA(text, oem, LOG4Z_LOG_BUF_SIZE);
#endif

    if (level <= LOG_LEVEL_DEBUG || level > LOG_LEVEL_FATAL)
    {
#if defined(WIN32) && defined(LOG4Z_OEM_CONSOLE)
        printf("%s", oem);
#else
        printf("%s", text);
#endif
        return;
    }
#ifndef WIN32
    printf("%s%s\e[0m", LOG_COLOR[level], text);
#else
    AutoLock l(_scLock);
    HANDLE hStd = ::GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStd == INVALID_HANDLE_VALUE) return;
    CONSOLE_SCREEN_BUFFER_INFO oldInfo;
    if (!GetConsoleScreenBufferInfo(hStd, &oldInfo))
    {
        return;
    }
    else
    {
        SetConsoleTextAttribute(hStd, LOG_COLOR[level]);
#ifdef LOG4Z_OEM_CONSOLE
		printf("%s", oem);
#else
		printf("%s", text);
#endif
		SetConsoleTextAttribute(hStd, oldInfo.wAttributes);
    }
#endif
    return;
}

bool LogerManager::configFromStringImpl(std::string content, bool isUpdate)
{
    unsigned int sum = 0;
    for (std::string::iterator iter = content.begin(); iter != content.end(); ++iter)
    {
        sum += (unsigned char)*iter;
    }
    if (sum == _checksum)
    {
        return true;
    }
    _checksum = sum;
    

    std::map<std::string, LoggerInfo> loggerMap;
    if (!parseConfigFromString(content, loggerMap))
    {
        printf(" !!! !!! !!! !!!\r\n");
		printf(" !!! !!! log4z load config file error \r\n");
		printf(" !!! !!! !!! !!!\r\n");
        return false;
    }
    for (std::map<std::string, LoggerInfo>::iterator iter = loggerMap.begin(); iter != loggerMap.end(); ++iter)
    {
        LoggerId id = LOG4Z_INVALID_LOGGER_ID;
        id = findLogger(iter->second._key.c_str());
        if (id == LOG4Z_INVALID_LOGGER_ID)
        {
            if (isUpdate)
            {
                continue;
            }
            else
            {
                id = createLogger(iter->second._key.c_str());
                if (id == LOG4Z_INVALID_LOGGER_ID)
                {
                    continue;
                }
            }
        }
        enableLogger(id, iter->second._enable);
        setLoggerName(id, iter->second._name.c_str());
        setLoggerPath(id, iter->second._path.c_str());
        setLoggerLevel(id, iter->second._level);
        setLoggerFileLine(id, iter->second._fileLine);
        setLoggerDisplay(id, iter->second._display);
        setLoggerOutFile(id, iter->second._outfile);
        setLoggerLimitsize(id, iter->second._limitsize);
        setLoggerMonthdir(id, iter->second._monthdir);
    }
    return true;
}

//! read configure and create with overwriting
bool LogerManager::config(const char* configPath)
{
    if (!_configFile.empty())
    {
		printf(" !!! !!! !!! !!!\r\n");
		printf(" !!! !!! log4z configure error: too many calls to Config. the old config file=%s,  the new config file=%s !!! !!! \r\n"
		, _configFile.c_str(), configPath);
		printf(" !!! !!! !!! !!!\r\n");
        return false;
    }
    _configFile = configPath;

    Log4zFileHandler f;
    f.open(_configFile.c_str(), "rb");
    if (!f.isOpen())
    {
		printf(" !!! !!! !!! !!!\r\n");
		printf(" !!! !!! log4z load config file error. filename=%s  !!! !!! \r\n", configPath);
		printf(" !!! !!! !!! !!!\r\n");
        return false;
    }
    return configFromStringImpl(f.readContent().c_str(), false);
}

//! read configure and create with overwriting
bool LogerManager::configFromString(const char* configContent)
{
    return configFromStringImpl(configContent, false);
}

//! create with overwriting
LoggerId LogerManager::createLogger(const char* key)
{
    if (key == NULL)
    {
        return LOG4Z_INVALID_LOGGER_ID;
    }
    
    std::string copyKey = key;
    trimLogConfig(copyKey);

    LoggerId newID = LOG4Z_INVALID_LOGGER_ID;
    {
        std::map<std::string, LoggerId>::iterator iter = _ids.find(copyKey);
        if (iter != _ids.end())
        {
            newID = iter->second;
        }
    }
    if (newID == LOG4Z_INVALID_LOGGER_ID)
    {
        if (_lastId +1 >= LOG4Z_LOGGER_MAX)
        {
            showColorText("log4z: CreateLogger can not create|writeover, because loggerid need < LOGGER_MAX! \r\n", LOG_LEVEL_FATAL);
            return LOG4Z_INVALID_LOGGER_ID;
        }
        newID = ++ _lastId;
        _ids[copyKey] = newID;
        _loggers[newID]._enable = true;
        _loggers[newID]._key = copyKey;
        _loggers[newID]._name = copyKey;
    }

    return newID;
}


bool LogerManager::start()
{
    if (_runing)
    {
        showColorText("log4z already start \r\n", LOG_LEVEL_FATAL);
        return false;
    }
    _semaphore.create(0);
    bool ret = ThreadHelper::start();
    return ret && _semaphore.wait(3000);
}
bool LogerManager::stop()
{
    if (_runing)
    {
        showColorText("log4z stopping \r\n", LOG_LEVEL_FATAL);
        _runing = false;
        wait();
        return true;
    }
    return false;
}
bool LogerManager::prePushLog(LoggerId id, int level)
{
    if (id < 0 || id > _lastId || !_runing || !_loggers[id]._enable)
    {
        return false;
    }
    if (level < _loggers[id]._level)
    {
        return false;
    }
    if (_logs.size() > LOG4Z_LOG_QUEUE_LIMIT_SIZE)
    {
        return false;
    }
    return true;
}
bool LogerManager::pushLog(LogData * pLog, const char * file, int line)
{
    // discard log
    if (pLog->_id < 0 || pLog->_id > _lastId || !_runing || !_loggers[pLog->_id]._enable)
    {
        freeLogData(pLog);
        return false;
    }

    //filter log
    if (pLog->_level < _loggers[pLog->_id]._level)
    {
        freeLogData(pLog);
        return false;
    }
    if (_loggers[pLog->_id]._fileLine && file)
    {
        const char * pNameBegin = file + strlen(file);
        do
        {
            if (*pNameBegin == '\\' || *pNameBegin == '/') { pNameBegin++; break; }
            if (pNameBegin == file) { break; }
            pNameBegin--;
        } while (true);
        zsummer::log4z::Log4zStream ss(pLog->_content + pLog->_contentLen, LOG4Z_LOG_BUF_SIZE - pLog->_contentLen); 
        ss << " " << pNameBegin << ":" << line;
        pLog->_contentLen += ss.getCurrentLen();
    }
    if (pLog->_contentLen < 3) pLog->_contentLen = 3;
    if (pLog->_contentLen +3 <= LOG4Z_LOG_BUF_SIZE ) pLog->_contentLen += 3;

    pLog->_content[pLog->_contentLen - 1] = '\0';
    pLog->_content[pLog->_contentLen - 2] = '\n';
    pLog->_content[pLog->_contentLen - 3] = '\r';
    pLog->_contentLen--; //clean '\0'


    if (_loggers[pLog->_id]._display && LOG4Z_ALL_SYNCHRONOUS_OUTPUT)
    {
        showColorText(pLog->_content, pLog->_level);
    }

    if (LOG4Z_ALL_DEBUGOUTPUT_DISPLAY && LOG4Z_ALL_SYNCHRONOUS_OUTPUT)
    {
#ifdef WIN32
        OutputDebugStringA(pLog->_content);
#endif
    }

    if (_loggers[pLog->_id]._outfile && LOG4Z_ALL_SYNCHRONOUS_OUTPUT)
    {
        AutoLock l(_logLock);
        if (openLogger(pLog))
        {
            _loggers[pLog->_id]._handle.write(pLog->_content, pLog->_contentLen);
            closeLogger(pLog->_id);
            _ullStatusTotalWriteFileCount++;
            _ullStatusTotalWriteFileBytes += pLog->_contentLen;
        }
    }

    if (LOG4Z_ALL_SYNCHRONOUS_OUTPUT)
    {
        freeLogData(pLog);
        return true;
    }
    
    AutoLock l(_logLock);
    _logs.push(pLog);
    _ullStatusTotalPushLog ++;
    return true;
}

//! 查找ID
LoggerId LogerManager::findLogger(const char * key)
{
    std::map<std::string, LoggerId>::iterator iter;
    iter = _ids.find(key);
    if (iter != _ids.end())
    {
        return iter->second;
    }
    return LOG4Z_INVALID_LOGGER_ID;
}

bool LogerManager::hotChange(LoggerId id, LogDataType ldt, int num, const std::string & text)
{
    if (id <0 || id > _lastId) return false;
    if (text.length() >= LOG4Z_LOG_BUF_SIZE) return false;
    if (!_runing || LOG4Z_ALL_SYNCHRONOUS_OUTPUT)
    {
        return onHotChange(id, ldt, num, text);
    }
    LogData * pLog = makeLogData(id, LOG4Z_DEFAULT_LEVEL);
    pLog->_id = id;
    pLog->_type = ldt;
    pLog->_typeval = num;
    memcpy(pLog->_content, text.c_str(), text.length());
    pLog->_contentLen = (int)text.length();
    AutoLock l(_logLock);
    _logs.push(pLog);
    return true;
}

bool LogerManager::onHotChange(LoggerId id, LogDataType ldt, int num, const std::string & text)
{
    if (id < LOG4Z_MAIN_LOGGER_ID || id > _lastId)
    {
        return false;
    }
    LoggerInfo & logger = _loggers[id];
    if (ldt == LDT_ENABLE_LOGGER) logger._enable = num != 0;
    else if (ldt == LDT_SET_LOGGER_NAME) logger._name = text;
    else if (ldt == LDT_SET_LOGGER_PATH) logger._path = text;
    else if (ldt == LDT_SET_LOGGER_LEVEL) logger._level = num;
    else if (ldt == LDT_SET_LOGGER_FILELINE) logger._fileLine = num != 0;
    else if (ldt == LDT_SET_LOGGER_DISPLAY) logger._display = num != 0;
    else if (ldt == LDT_SET_LOGGER_OUTFILE) logger._outfile = num != 0;
    else if (ldt == LDT_SET_LOGGER_LIMITSIZE) logger._limitsize = num;
	else if (ldt == LDT_SET_LOGGER_MONTHDIR) logger._monthdir = num != 0;
	else if (ldt == LDT_SET_LOGGER_RESERVETIME) logger._logReserveTime = num >= 0 ? num : 0;
	return true;
}

bool LogerManager::enableLogger(LoggerId id, bool enable) 
{
    if (id < 0 || id > _lastId) return false;
    if (enable)
    {
        _loggers[id]._enable = true;
        return true;
    }
    return hotChange(id, LDT_ENABLE_LOGGER, false, ""); 
}
bool LogerManager::setLoggerLevel(LoggerId id, int level) 
{ 
    if (id < 0 || id > _lastId) return false;
    if (level <= _loggers[id]._level)
    {
        _loggers[id]._level = level;
        return true;
    }
    return hotChange(id, LDT_SET_LOGGER_LEVEL, level, ""); 
}
bool LogerManager::setLoggerDisplay(LoggerId id, bool enable) { return hotChange(id, LDT_SET_LOGGER_DISPLAY, enable, ""); }
bool LogerManager::setLoggerOutFile(LoggerId id, bool enable) { return hotChange(id, LDT_SET_LOGGER_OUTFILE, enable, ""); }
bool LogerManager::setLoggerMonthdir(LoggerId id, bool enable) { return hotChange(id, LDT_SET_LOGGER_MONTHDIR, enable, ""); }
bool LogerManager::setLoggerFileLine(LoggerId id, bool enable) { return hotChange(id, LDT_SET_LOGGER_FILELINE, enable, ""); }
bool LogerManager::setLoggerReserveTime(LoggerId id, time_t sec) { return hotChange(id, LDT_SET_LOGGER_RESERVETIME, (int)sec, ""); }
bool LogerManager::setLoggerLimitsize(LoggerId id, unsigned int limitsize)
{
    if (limitsize == 0 ) {limitsize = (unsigned int)-1;}
    return hotChange(id, LDT_SET_LOGGER_LIMITSIZE, limitsize, "");
}

bool LogerManager::setLoggerName(LoggerId id, const char * name)
{
    if (id <0 || id > _lastId) return false;
    //the name by main logger is the process name and it's can't change. 
//    if (id == LOG4Z_MAIN_LOGGER_ID) return false; 
    
    if (name == NULL || strlen(name) == 0) 
    {
        return false;
    }
    return hotChange(id, LDT_SET_LOGGER_NAME, 0, name);
}

bool LogerManager::setLoggerPath(LoggerId id, const char * path)
{
    if (id <0 || id > _lastId) return false;
    if (path == NULL || strlen(path) == 0)  return false;
    std::string copyPath = path;
    {
        char ch = copyPath.at(copyPath.length() - 1);
        if (ch != '\\' && ch != '/')
        {
            copyPath.append("/");
        }
    }
    return hotChange(id, LDT_SET_LOGGER_PATH, 0, copyPath);
}
bool LogerManager::setAutoUpdate(int interval)
{
    _hotUpdateInterval = interval;
    return true;
}
bool LogerManager::updateConfig()
{
    if (_configFile.empty())
    {
        //LOGW("log4z update config file error. filename is empty.");
        return false;
    }
    Log4zFileHandler f;
    f.open(_configFile.c_str(), "rb");
    if (!f.isOpen())
    {
		printf(" !!! !!! !!! !!!\r\n");
		printf(" !!! !!! log4z load config file error. filename=%s  !!! !!! \r\n", _configFile.c_str());
		printf(" !!! !!! !!! !!!\r\n");
        return false;
    }
    return configFromStringImpl(f.readContent().c_str(), true);
}

bool LogerManager::isLoggerEnable(LoggerId id)
{
    if (id <0 || id > _lastId) return false;
    return _loggers[id]._enable;
}

unsigned int LogerManager::getStatusActiveLoggers()
{
    unsigned int actives = 0;
    for (int i=0; i<= _lastId; i++)
    {
        if (_loggers[i]._enable)
        {
            actives ++;
        }
    }
    return actives;
}


bool LogerManager::openLogger(LogData * pLog)
{
    int id = pLog->_id;
    if (id < 0 || id >_lastId)
    {
        showColorText("log4z: openLogger can not open, invalide logger id! \r\n", LOG_LEVEL_FATAL);
        return false;
    }

    LoggerInfo * pLogger = &_loggers[id];
    if (!pLogger->_enable || !pLogger->_outfile || pLog->_level < pLogger->_level)
    {
        return false;
    }

    bool sameday = isSameDay(pLog->_time, pLogger->_curFileCreateTime);
    bool needChageFile = pLogger->_curWriteLen > pLogger->_limitsize * 1024 * 1024;
    if (!sameday || needChageFile)
    {
        if (!sameday)
        {
            pLogger->_curFileIndex = 0;
        }
        else
        {
            pLogger->_curFileIndex++;
        }
        if (pLogger->_handle.isOpen())
        {
            pLogger->_handle.close();
        }
    }
    if (!pLogger->_handle.isOpen())
    {
        pLogger->_curFileCreateTime = pLog->_time;
        pLogger->_curWriteLen = 0;

        tm t = timeToTm(pLogger->_curFileCreateTime);
        std::string name;
        std::string path;

        name = pLogger->_name;
        path = pLogger->_path;

        
        char buf[500] = { 0 };
        if (pLogger->_monthdir)
        {
            sprintf(buf, "%04d_%02d/", t.tm_year + 1900, t.tm_mon + 1);
            path += buf;
        }

        if (!isDirectory(path))
        {
            createRecursionDir(path);
        }

        sprintf(buf, "%s_%s_%04d%02d%02d%02d%02d_%s_%03u.log",
            _proName.c_str(), name.c_str(), t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
            t.tm_hour, t.tm_min, _pid.c_str(), pLogger->_curFileIndex);
        path += buf;
        pLogger->_handle.open(path.c_str(), "ab");
        if (!pLogger->_handle.isOpen())
        {
			sprintf(buf, "log4z: can not open log file %s. \r\n", path.c_str());
            showColorText("!!!!!!!!!!!!!!!!!!!!!!!!!! \r\n", LOG_LEVEL_FATAL);
            showColorText(buf, LOG_LEVEL_FATAL);
            showColorText("!!!!!!!!!!!!!!!!!!!!!!!!!! \r\n", LOG_LEVEL_FATAL);
            pLogger->_outfile = false;
            return false;
        }
		if (pLogger->_logReserveTime > 0 )
		{
			if (pLogger->_historyLogs.size() > LOG4Z_FORCE_RESERVE_FILE_COUNT)
			{
				while (!pLogger->_historyLogs.empty() && pLogger->_historyLogs.front().first < time(NULL) - pLogger->_logReserveTime)
				{
					pLogger->_handle.removeFile(pLogger->_historyLogs.front().second.c_str());
					pLogger->_historyLogs.pop_front();
				}
			}
			pLogger->_historyLogs.push_back(std::make_pair(time(NULL), path));
		}
        return true;
    }
    return true;
}
bool LogerManager::closeLogger(LoggerId id)
{
    if (id < 0 || id >_lastId)
    {
        showColorText("log4z: closeLogger can not close, invalide logger id! \r\n", LOG_LEVEL_FATAL);
        return false;
    }
    LoggerInfo * pLogger = &_loggers[id];
    if (pLogger->_handle.isOpen())
    {
        pLogger->_handle.close();
        return true;
    }
    return false;
}
bool LogerManager::popLog(LogData *& log)
{
    AutoLock l(_logLock);
    if (_logs.empty())
    {
        return false;
    }
    log = _logs.front();
    _logs.pop();
    return true;
}

void LogerManager::run()
{
    _runing = true;
    LOGA("-----------------  log4z thread started!   ----------------------------");
    for (int i = 0; i <= _lastId; i++)
    {
        if (_loggers[i]._enable)
        {
            LOGA("logger id=" << i
                << " key=" << _loggers[i]._key
                << " name=" << _loggers[i]._name
                << " path=" << _loggers[i]._path
                << " level=" << _loggers[i]._level
                << " display=" << _loggers[i]._display);
        }
    }

    _semaphore.post();


    LogData * pLog = NULL;
    int needFlush[LOG4Z_LOGGER_MAX] = {0};
    time_t lastCheckUpdate = time(NULL);
    while (true)
    {
        while(popLog(pLog))
        {
            if (pLog->_id <0 || pLog->_id > _lastId)
            {
                freeLogData(pLog);
                continue;
            }
            LoggerInfo & curLogger = _loggers[pLog->_id];

            if (pLog->_type != LDT_GENERAL)
            {
                onHotChange(pLog->_id, (LogDataType)pLog->_type, pLog->_typeval, std::string(pLog->_content, pLog->_contentLen));
                curLogger._handle.close();
                freeLogData(pLog);
                continue;
            }
            
            //
            _ullStatusTotalPopLog ++;
            //discard
            
            if (!curLogger._enable || pLog->_level <curLogger._level  )
            {
                freeLogData(pLog);
                continue;
            }


            if (curLogger._display && !LOG4Z_ALL_SYNCHRONOUS_OUTPUT)
            {
                showColorText(pLog->_content, pLog->_level);
            }
            if (LOG4Z_ALL_DEBUGOUTPUT_DISPLAY && !LOG4Z_ALL_SYNCHRONOUS_OUTPUT)
            {
#ifdef WIN32
                OutputDebugStringA(pLog->_content);
#endif
            }


            if (curLogger._outfile && !LOG4Z_ALL_SYNCHRONOUS_OUTPUT)
            {
                if (!openLogger(pLog))
                {
                    freeLogData(pLog);
                    continue;
                }

                curLogger._handle.write(pLog->_content, pLog->_contentLen);
                curLogger._curWriteLen += (unsigned int)pLog->_contentLen;
                needFlush[pLog->_id] ++;
                _ullStatusTotalWriteFileCount++;
                _ullStatusTotalWriteFileBytes += pLog->_contentLen;
            }
            else if (!LOG4Z_ALL_SYNCHRONOUS_OUTPUT)
            {
                _ullStatusTotalWriteFileCount++;
                _ullStatusTotalWriteFileBytes += pLog->_contentLen;
            }

            freeLogData(pLog);
        }

        for (int i=0; i<=_lastId; i++)
        {
            if (_loggers[i]._enable && needFlush[i] > 0)
            {
                _loggers[i]._handle.flush();
                needFlush[i] = 0;
            }
            if(!_loggers[i]._enable && _loggers[i]._handle.isOpen())
            {
                _loggers[i]._handle.close();
            }
        }

        //! delay. 
        sleepMillisecond(100);

        //! quit
        if (!_runing && _logs.empty())
        {
            break;
        }
        
        if (_hotUpdateInterval != 0 && time(NULL) - lastCheckUpdate > _hotUpdateInterval)
        {
            updateConfig();
            lastCheckUpdate = time(NULL);
        }
        


    }

    for (int i=0; i <= _lastId; i++)
    {
        if (_loggers[i]._enable)
        {
            _loggers[i]._enable = false;
            closeLogger(i);
        }
    }

}

//////////////////////////////////////////////////////////////////////////
//ILog4zManager::getInstance
//////////////////////////////////////////////////////////////////////////
ILog4zManager * ILog4zManager::getInstance()
{
    static LogerManager m;
    return &m;
}



_ZSUMMER_LOG4Z_END
_ZSUMMER_END
