#ifndef INCLUDE_LOGGER_H
#define INCLUDE_LOGGER_H

#include <stdarg.h>
#include <string.h>

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
#define loggerf(level, ...) floggerf(level, __FILENAME__, __LINE__, __VA_ARGS__)
#define logger(text, level) floggerf(level, __FILENAME__, __LINE__, text)

void init_logger(const char * file_location);
void init_logger_file_only(const char * file_location);
void exit_logger();

void set_level(enum logging_levels level);
void set_logger_print_level(enum logging_levels level);
enum logging_levels read_level();

void floggerf(enum logging_levels level, const char * file, const int line, const char * text, ...);

#endif

