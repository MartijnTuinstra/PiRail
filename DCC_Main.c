#include <wiringPi.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main() {
  struct timespec tim, tim2;
  tim.tv_sec = 0;
  tim.tv_nsec = 25000;


  if (wiringPiSetup () == -1)
    exit (1) ;

  pinMode(7, OUTPUT);

  while(1) {
    digitalWrite(7, 0);
    delayMicroseconds(50);
    digitalWrite(7, 1);
    delayMicroseconds(50);
  }

  return 0 ;
}
