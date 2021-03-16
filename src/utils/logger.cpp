#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <time.h>
#include <stdint.h>

#include "utils/logger.h"
#include "utils/mem.h"

Logger logger = Logger();

bool Logger::clear(){
  FILE * fp = fopen(filename,"w");

  if(!fp){
    return false;
  }

  fclose(fp);
  return true;
}

bool Logger::open(){
  file = fopen(filename,"w");

  if(!file){
    return false;
  }

  enabled = true;
  fileout = true;
  return true;
}

void Logger::close(){
  if(!enabled)
    return;

  enabled = stdout;
  fileout = false;

  if(!file)
    return;

  fclose(file);
}
uint16_t Logger::write(){
  return 0;
}

Logger::Logger(){
  enabled = false;
  stdout = false;
  fileout = false;
  pthread_mutex_init(&mutex, NULL);
}
Logger::~Logger(){
  _free(filename);
  close();
  enabled = false;
  pthread_mutex_destroy(&mutex);
}

void Logger::setfilename(const char * file_location){
  filename = (char *)_calloc(strlen(file_location) + 1, char);
  strcpy(filename, file_location);

  if(!clear() || !open())
    printf("Failed to open logger file: %s\n", filename);

  stdout = true;
}

void Logger::setlevel(enum logging_levels lvl){
  file_lvl = lvl;
}

void Logger::setlevel_stdout(enum logging_levels lvl){
  stdout_lvl = lvl;

  if(stdout_lvl == NONE){
    enabled = fileout;
    stdout = false;
  }
  else{
    enabled = true;
    stdout = true;
  }

}

void Logger::f(enum logging_levels level, const char * file, const int line, const char * text, ...){
  if(!enabled || (level > file_lvl && level > stdout_lvl))
    return;

  pthread_mutex_lock(&mutex);

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

  char msg[10000];
  char loggertext[900];

  vsprintf(loggertext, text, arglist);

  va_end( arglist );

  char * newline = strchr(loggertext, '\n');

  if(newline){
    char * ptr = &msg[0];

    ptr += sprintf(ptr, "%s.%03d - %s%s -%20s:%4i- ", c_time, (uint16_t)(clock.tv_nsec / 1e6), levels_colour[level],
                                                    levels_str[level], file, line);

    char * token = strtok(loggertext, "\n");

    bool first = true;

    while( token != NULL ) {
      if(!first){
        ptr += sprintf(ptr, "\n             -          -                         - ");
      }
      else{
        first = false;
      }
      ptr += sprintf(ptr, "%s", token); //printing each token

      token = strtok(NULL, "\n");
    }

    ptr += sprintf(ptr, "%s\n", levels_colour[8]); // reset colour
  }
  else{
    sprintf(msg, "%s.%03d - %s%s -%20s:%4i- %s%s\n", c_time, (uint16_t)(clock.tv_nsec / 1e6), levels_colour[level],
                                                     levels_str[level], file, line, loggertext, levels_colour[8]);
  }


  if(stdout && level <= stdout_lvl)
    printf("%s", msg);

  if(fileout && level <= file_lvl){
    fprintf(this->file, "%s", msg);
  }

  pthread_mutex_unlock(&mutex);
}

void Logger::hexdump(const char * file, const int line, const char * header, void * data, int length){
  char text[8000];
  char * ptr = text;

  ptr += sprintf(ptr, "%s\n", header);
  for(int i = 0; i < length; i++){
    ptr += sprintf(ptr, "%02x ", ((uint8_t *)data)[i]);
    if((i % 16) == 15)
      ptr += sprintf(ptr, "\n");
  }

  f(DEBUG, file, line, (const char *)text);
}