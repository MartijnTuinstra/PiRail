#include <string.h>
#include <stdio.h>

#include "utils/utils.h"
#include "utils/logger.h"

int moveFile(char * src, char * dest){
  int ch;
  FILE *source, *target;

  source = fopen(src, "r");

  if (source == NULL){
    loggerf(ERROR, "Could not open source file (%s)", src);
    return 0;
  }

  target = fopen(dest, "w");

  if (target == NULL){
    fclose(source);
    loggerf(ERROR, "Could not create destination file (%s)", dest);
    return 0;
  }

  while ((ch = fgetc(source)) != EOF)
    fputc((char)ch, target);

  fclose(source);
  fclose(target);

  return 1;
}

void replaceCharinString(char * str, char orig, char sub){
  for (char* current_pos = NULL; (current_pos = strchr(str, orig)) != NULL; *current_pos = sub);
}

int ctsTempFile(int time, char * filename, bool img, bool png){
  time = (time / 60) * 100 + (time % 60);

  char sourceFile[50] = "";
  char destinationFile[50] = "";

  sprintf(&filename[strlen(filename)], "%s.%s", img ? "im" : "ic", png ? "png" : "jpg");

  sprintf(sourceFile, "web/tmp_%s.%04i.%s", img ? "img" : "icon", time, png ? "png" : "jpg");
  sprintf(destinationFile, "web/trains_img/%s", filename);

  int status = moveFile(sourceFile, destinationFile);

  remove(sourceFile);

  return status;
}