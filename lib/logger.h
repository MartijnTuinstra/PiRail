#ifndef INCLUDE_LOGGER_H
#define INCLUDE_LOGGER_H

#include <stdarg.h>
#include <string.h>

enum logging_levels {
  CRITICAL,
  ERROR,
  WARNING,
  INFO,
  DEBUG,
  TRACE,
  MEMORY
};

#define STR(x) #x
#define STRH(x) STR(x)
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define loggerf(level, ...) floggerf(level, __FILENAME__, __LINE__, __VA_ARGS__)
#define logger(text, level) floggerf(level, __FILENAME__, __LINE__, text)

void init_logger(char * file_location);
void init_logger_file_only(char * file_location);
void exit_logger();

void set_level(enum logging_levels);
enum logging_levels read_level();

void floggerf(enum logging_levels level, char * file, int line, char * text, ...);

#endif

