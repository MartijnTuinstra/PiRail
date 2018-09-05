#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include "logger.h"

#define LOG_TO_STDOUT
#ifdef LOG_TO_STDOUT
#define log_print(fp, ...) printf(__VA_ARGS__);\
                          fprintf(fp, __VA_ARGS__);
#define vlog_print(fp, ...) vprintf(__VA_ARGS__);\
                           vfprintf(fp, __VA_ARGS__);
#else
#define vlog_print(fp, ...) vfprintf(fp, __VA_ARGS__);
#endif

char * logger_file;

void init_logger(char * file_location){
  // Clear log
  FILE * fp = fopen(file_location,"w");
  fclose(fp);

  logger_file = (char *)malloc(sizeof(char)*strlen(file_location));
  strcpy(logger_file,file_location);
}

enum logging_levels logger_set_lvl;

pthread_mutex_t logger_mutex;

void set_level(enum logging_levels level){
  logger_set_lvl = level;
}

void floggerf(enum logging_levels level, char * file, int line, char * text, ...){
  if(level > logger_set_lvl)
    return;

  pthread_mutex_lock(&logger_mutex);

  va_list arglist;
  va_start( arglist, text );

  time_t current_time;
  struct tm * time_info;
  char c_time[9];  // space for "HH:MM:SS\0"

  time(&current_time);
  time_info = localtime(&current_time);

  strftime(c_time, sizeof(c_time), "%H:%M", time_info);

  FILE * fp = fopen(logger_file,"a");

  printf("%s - ",c_time);
  fprintf(fp,"%s - ",c_time);

  if(level == CRITICAL){
    log_print(fp,"CRITICAL");
  }
  else if(level == ERROR){
    log_print(fp,"..ERROR.");
  }
  else if(level == WARNING){
    log_print(fp,".WARNING");
  }
  else if(level == INFO){
    log_print(fp,"..INFO..");
  }
  else if(level == DEBUG){
    log_print(fp,"..DEBUG.");
  }
  else if(level == TRACE){
    log_print(fp,"..TRACE.");
  }
  else if(level == MEMORY){
    log_print(fp,".MEMORY.");
  }

  log_print(fp," - %20s:%4i - ", file, line);

  vlog_print(fp, text, arglist);

  log_print(fp,"\n");

  fclose(fp);
  va_end( arglist );

  pthread_mutex_unlock(&logger_mutex);
}
