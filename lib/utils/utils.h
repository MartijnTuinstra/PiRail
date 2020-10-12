#ifndef _INCLUDE_UTILS_UTILS_H
#define _INCLUDE_UTILS_UTILS_H

int moveFile(char *, char *);

void replaceCharinString(char * str, char orig, char sub);

int ctsTempFile(int time, char * filename, bool img, bool png);

#endif