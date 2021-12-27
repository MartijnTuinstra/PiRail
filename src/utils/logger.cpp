#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <time.h>
#include <stdint.h>

#include "utils/logger.h"
#include "utils/mem.h"

Logging BaseLogging;

Logger::Logger(Logging * _Base, const char * _name){Base = _Base; name = _name;}

void Logger::f(enum logging_levels level, const char * file, const int line, const char * text, ...) {
  char logtext[900] = "";
  
  va_list arglist;
  va_start( arglist, text );
  vsprintf(logtext, text, arglist);
  va_end( arglist );

  Base->f(this, level, file, line, logtext);
}


void Logger::hexdump(enum logging_levels level, const char * file, const int line, const char * header, void * data, int length){
  char text[8000];
  char * ptr = text;

  ptr += sprintf(ptr, "%s\n", header);
  for(int i = 0; i < length; i++){
    ptr += sprintf(ptr, "%02x ", ((uint8_t *)data)[i]);
    if((i % 16) == 15)
      ptr += sprintf(ptr, "\n");
  }

  f(level, file, line, (const char *)text);
}

void Logger::setlevel(enum logging_levels lvl){
  file_lvl = lvl;
}

void Logger::setlevel_stdout(enum logging_levels lvl){
  stdout_lvl = lvl;
}

void Logger::setEnabled(bool _enabled){
  enabled = _enabled;
}

bool Logging::clear(){
  FILE * fp = fopen(filename,"w");

  if(!fp){
    return false;
  }

  fclose(fp);
  return true;
}

bool Logging::open(){
  file = fopen(filename,"w");

  if(!file){
    return false;
  }

  fileout = true;
  return true;
}

void Logging::close(){
  if(!fileout)
    return;

  fileout = false;

  if(!file)
    return;

  fclose(file);
}
uint16_t Logging::write(){
  return 0;
}

Logging::Logging(){
  pthread_mutex_init(&mutex, NULL);

  Loggers["root"] = new Logger(this, "root");
}
Logging::~Logging(){
  _free(filename);
  close();
  pthread_mutex_destroy(&mutex);
}

void Logging::setfilename(const char * file_location){
  filename = (char *)_calloc(strlen(file_location) + 1, char);
  strcpy(filename, file_location);

  if(!clear() || !open())
    printf("Failed to open logger file: %s\n", filename);

  stdout = true;
}

void Logging::setDetailLevel(uint8_t level){
  detail_level = level;

  f(DEBUG, "", 0, "Logger detail level set to %s", S_detail_level[level]);
}

Logger * Logging::getLogger(const char * loggername){
  for(auto L: Loggers){
    if(strcmp(loggername, L.first) == 0)
      return L.second;
  }

  Loggers[loggername] = new Logger(this, loggername);
  return Loggers[loggername];
}

void Logging::printLoggers(){

  char loggerText[5000];
  char * p = &loggerText[0];

  p += sprintf(p, "Currently available loggers: \n");
  for(auto L: Loggers){
    p += sprintf(p, " - %-20s  %s  %s\n", L.first, levels_str[L.second->file_lvl], levels_str[L.second->stdout_lvl]);
  }
  
  f(INFO, "logger", 0, "%s", loggerText);
}

void Logging::f(Logger * logger, enum logging_levels level, const char * file, const int line, char * text){
  pthread_mutex_lock(&mutex);

  // time_t current_time;
  struct tm * time_info;
  struct timespec clock;
  char c_time[9];  // space for "HH:MM:SS\0"

  // time(&clock);m
  clock_gettime(CLOCK_REALTIME_COARSE, &clock);
  time_info = localtime(&clock.tv_sec);

  if(detail_level != 1)
    strftime(c_time, sizeof(c_time), "%H:%M:%S", time_info);
  else
    strftime(c_time, sizeof(c_time), "%H%M%S", time_info);

  char msg[10000];
  char * msgP = &msg[0];

  if (detail_level < 4 && detail_level != 1)
    msgP += sprintf(msgP, "%s.%03d", c_time, (uint16_t)(clock.tv_nsec / 1e6));
  else if(detail_level == 1)
    msgP += sprintf(msgP, "%s%03d", c_time, (uint16_t)(clock.tv_nsec / 1e6));

  msgP += sprintf(msgP, "%s", levels_colour[level]);

  if (detail_level == 0)
    msgP += sprintf(msgP, " - %s", levels_str[level]);
  else if(detail_level == 1)
    msgP += sprintf(msgP, "%c", levels_short_str[level]);

  if (detail_level < 3)
    msgP += sprintf(msgP, " -%20s:%4i- ", file, line);

  char * newline = strchr(text, '\n');

  if(newline){
    char * ptr = &msgP[0];
    char * token = strtok(text, "\n");

    bool first = true;

    while( token != NULL ) {
      if(!first)
        ptr += sprintf(ptr, "%s", detail_level_offset[detail_level]);
      else
        first = false;

      ptr += sprintf(ptr, "%s", token); //printing each token

      token = strtok(NULL, "\n");
    }

    ptr += sprintf(ptr, "%s\n", levels_colour[8]); // reset colour
  }
  else{
    msgP += sprintf(msgP, "%s%s\n", text, levels_colour[8]);
  }

  if(stdout && level <= logger->stdout_lvl)
    printf("%s", msg);

  if(fileout && level <= logger->file_lvl){
    fprintf(this->file, "%s", msg);
  }

  pthread_mutex_unlock(&mutex);
}


void Logging::f(enum logging_levels level, const char * file, const int line, const char * text, ...){
  char logtext[900];
  
  va_list arglist;
  va_start( arglist, text );
  vsprintf(logtext, text, arglist);
  va_end( arglist );

  Loggers["root"]->f(level, file, line, logtext);
}

void Logging::hexdump(enum logging_levels level, const char * file, const int line, const char * header, void * data, int length){
  char text[8000];
  char * ptr = text;

  ptr += sprintf(ptr, "%s\n", header);
  for(int i = 0; i < length; i++){
    ptr += sprintf(ptr, "%02x ", ((uint8_t *)data)[i]);
    if((i % 16) == 15)
      ptr += sprintf(ptr, "\n");
  }

  f(level, file, line, (const char *)text);
}

Logger * getLogger(){ return BaseLogging.getLogger("root"); }
Logger * getLogger(const char * name){ return BaseLogging.getLogger(name); }
