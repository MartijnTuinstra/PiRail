#ifndef INCLUDE_LOGGER_H
#define INCLUDE_LOGGER_H

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

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
#define loggerf(level, ...) logger.f(level, __FILENAME__, __LINE__, __VA_ARGS__)
#define log(text, level) logger.f(level, __FILENAME__, __LINE__, text)
#define log_hex(header, data, length) logger.hexdump(__FILENAME__, __LINE__, header, data, length);

class Logger {
  private:
    bool clear();
    bool open();
    void close();
    uint16_t write();


    const char * levels_str[9] = {
      "NONE", "CRITICAL", "  ERROR ", " WARNING", "  INFO  ", "  DEBUG ", "  TRACE ", " MEMORY ", "        "
    };
    const char * levels_colour[9] = {
      "", "\x1b[31m", "\x1b[31m", "\x1b[33m", "\x1b[32m", "", "", "", "\x1b[0m"
    };

    char * filename = 0;
    FILE * file;

    bool enabled = false;
    bool stdout = false;
    bool fileout = false;

    enum logging_levels stdout_lvl = INFO;
    enum logging_levels file_lvl = INFO;

    pthread_mutex_t mutex;
  
  public:
    Logger();
    ~Logger();

    void setfilename(const char * file_location);

    void setlevel(enum logging_levels);
    void setlevel_stdout(enum logging_levels);

    void f(enum logging_levels level, const char * file, const int line, const char * text, ...);
    void hexdump(const char * file, const int line, const char * header, void * data, int length);
};

extern Logger logger;

// void init_logger(const char * file_location);
// void init_logger_file_only(const char * file_location);
// void exit_logger();

// void set_level(enum logging_levels level);
// void set_logger_print_level(enum logging_levels level);
// enum logging_levels read_level();

// void floggerf(enum logging_levels level, const char * file, const int line, const char * text, ...);

#endif

