#ifndef INCLUDE_LOGGER_H
#define INCLUDE_LOGGER_H

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <iostream>
#include <map>
#include <string>
#include <string_view>

enum logging_levels {
  NONE,
  CRITICAL,
  ERROR,
  WARNING,
  INFO,
  DEBUG,
  TRACE,
  MEMORY,
  ALL
};

#define STR(x) #x
#define STRH(x) STR(x)
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define log(_LogName_, _LogLvL_, ...) getLogger(_LogName_)->f(_LogLvL_, __FILENAME__, __LINE__, __VA_ARGS__)
#define log_hex(_LogName_, _LogLvL_, _header_, _data_, _length_) getLogger(_LogName_)->hexdump(_LogLvL_, __FILENAME__, __LINE__, _header_, _data_, _length_)

#define loggerf(level, ...) log("root", level, __VA_ARGS__)
#define logger_hex(_LogLvL_, _header_, _data_, _length_) log_hex("root", _LogLvL_, _header_, _data_, _length_)

// #define log(text, level) BaseLogging.f(level, __FILENAME__, __LINE__, text)


class Logging;

class Logger {
  bool enabled = true;

  public:
  enum logging_levels stdout_lvl = INFO;
  enum logging_levels file_lvl = INFO;
  private:

  Logging * Base;
  const char * name;

  public:
  Logger(Logging * Base, const char * name);

  void setlevel(enum logging_levels);
  void setlevel_stdout(enum logging_levels);
  void setEnabled(bool enabled);

  void f(enum logging_levels level, const char * file, const int line, const char * text, ...);
  void hexdump(enum logging_levels level, const char * file, const int line, const char * header, void * data, int length);

};

class Logging {
  private:
    bool clear();
    bool open();
    void close();
    uint16_t write();


    const char * levels_str[9] = {
      "NONE", "CRITICAL", "  ERROR ", " WARNING", "  INFO  ", "  DEBUG ", "  TRACE ", " MEMORY ", "        "
    };
    const char levels_short_str[10] = "NCEWIDTM ";
    const char * levels_colour[9] = {
      "", "\x1b[31m", "\x1b[31m", "\x1b[33m", "\x1b[32m", "", "", "", "\x1b[0m"
    };
    const char * S_detail_level[6] = {
      "ALL", "SHORT", "T+L+M", "T+M", "M"
    }; // T: Time, L: Location, M: Message, ALL has level as well, SHORT minimizes space
    const char * detail_level_offset[6] = {
      "\n             -          -                         - ",
      "\n          -   -                         - ",
      "\n             -                         - ",
      "\n             - ",
      "\n"
    };

    char * filename = 0;
    FILE * file;

    std::map<const char *, Logger *> Loggers;

    bool stdout = true;
    bool fileout = false;
    uint8_t detail_level;
    uint8_t nameLength = 4;

    pthread_mutex_t mutex;
  
  public:
    Logging();
    ~Logging();

    void setfilename(const char * file_location);

    void setDetailLevel(uint8_t);

    Logger * getLogger(const char * name);
    void printLoggers();

    void f(Logger * L, enum logging_levels lvl, const char * fn, const int fl, char * t);
    void f(enum logging_levels level, const char * file, const int line, const char * text, ...);
    void hexdump(enum logging_levels level, const char * file, const int line, const char * header, void * data, int length);
};

// extern Logger logger;

extern Logging BaseLogging;

Logger * getLogger();
Logger * getLogger(const char * name);

// void init_logger(const char * file_location);
// void init_logger_file_only(const char * file_location);
// void exit_logger();

// void set_level(enum logging_levels level);
// void set_logger_print_level(enum logging_levels level);
// enum logging_levels read_level();

// void floggerf(enum logging_levels level, const char * file, const int line, const char * text, ...);

#endif

