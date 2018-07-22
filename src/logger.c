#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include "logger.h"

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
    printf("CRITICAL");
    fprintf(fp,"CRITICAL");
  }
  else if(level == ERROR){
    printf("..ERROR.");
    fprintf(fp,"..ERROR.");
  }
  else if(level == WARNING){
    printf(".WARNING");
    fprintf(fp,".WARNING");
  }
  else if(level == INFO){
    printf("..INFO..");
    fprintf(fp,"..INFO..");
  }
  else if(level == DEBUG){
    printf("..DEBUG.");
    fprintf(fp,"..DEBUG.");
  }
  else if(level == MEMORY){
    printf(".MEMORY.");
    fprintf(fp,".MEMORY.");
  }

  printf(" - %20s:%4i - ", file, line);
  fprintf(fp," - %15s:%4i - ", file, line);

  vprintf(text, arglist);
  vfprintf(fp, text, arglist);

  printf("\n");
  fprintf(fp,"\n");

  fclose(fp);
  va_end( arglist );

  pthread_mutex_unlock(&logger_mutex);
}
