#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include "logger.h"
#include "mem.h"

#define LOG_TO_STDOUT
#ifdef LOG_TO_STDOUT
#define log_print(fp, ...) printf(__VA_ARGS__);\
                          fprintf(fp, __VA_ARGS__);
#define vlog_print(fp, ...) vprintf(__VA_ARGS__);\
                           vfprintf(fp, __VA_ARGS__);
#else
#define vlog_print(fp, ...) vfprintf(fp, __VA_ARGS__);
#endif

#define LOGGER_RED    "\x1b[31m"
#define LOGGER_RESET  "\x1b[0m"
#define LOGGER_YELLOW "\x1b[33m"
#define LOGGER_GREEN  "\x1b[32m"

char * logger_file = 0;

const char * logging_levels_str[7] = {
  "CRITICAL", "  ERROR ", " WARNING", "  INFO  ", "  DEBUG ", "  TRACE ", " MEMORY "
};
const char * logging_levels_colour[8] = {
  "\x1b[31m", "\x1b[31m", "\x1b[33m", "\x1b[32m", "", "", "", "\x1b[0m"
};

_Bool stdoutPrint = 1;

void init_logger(char * file_location){
  // Clear log / create log
  FILE * fp = fopen(file_location,"w");
  
  fclose(fp);

  logger_file = _calloc(strlen(file_location) + 1, char);
  strcpy(logger_file,file_location);

  stdoutPrint = 1;
}

void init_logger_file_only(char * file_location){
  init_logger(file_location);
  stdoutPrint = 0;
}

void exit_logger(){
  _free(logger_file);
}

enum logging_levels logger_set_lvl;
enum logging_levels logger_set_print_lvl;

pthread_mutex_t logger_mutex;

void set_level(enum logging_levels level){
  logger_set_lvl = level;
}

void set_logger_print_level(enum logging_levels level){
  logger_set_print_lvl = level;
}

enum logging_levels read_level(){
  return logger_set_lvl;
}

void floggerf(enum logging_levels level, char * file, int line, char * text, ...){
  if(level > logger_set_lvl && level > logger_set_print_lvl)
    return;

  pthread_mutex_lock(&logger_mutex);

  va_list arglist;
  va_start( arglist, text );

  // time_t current_time;
  struct tm * time_info;
  struct timespec clock;
  char c_time[9];  // space for "HH:MM:SS\0"

  // time(&clock);m
  clock_gettime(CLOCK_REALTIME_COARSE, &clock);
  time_info = localtime(&clock.tv_sec);


  strftime(c_time, sizeof(c_time), "%H:%M:%S", time_info);

  char msg[300];
  char arg[200];

  vsprintf(arg, text, arglist);

  sprintf(msg, "%s.%ld - %s%s - %20s:%4i - %s%s\n",c_time,clock.tv_nsec,logging_levels_colour[level], logging_levels_str[level],file,line, arg, logging_levels_colour[7]);


  if(stdoutPrint && level <= logger_set_print_lvl)
    printf("%s", msg);

  if(level <= logger_set_lvl){
    FILE * fp = fopen(logger_file,"a");
    if(fp){
      fprintf(fp, "%s", msg);
      fclose(fp);
    }
    else
      printf("%s%s - Failed to open logger %s\n", logging_levels_colour[0], logging_levels_str[0], logging_levels_colour[7]);
  }

  va_end( arglist );

  pthread_mutex_unlock(&logger_mutex);
}
