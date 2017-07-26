#ifndef TIMESERVICE_H
#define TIMESERVICE_H

#include <SPI.h>
#include "Util.h"

#define MAX_TIME_SLEW_DURATION 60000U

class TimeService {
  public:
    static void setTime(unsigned long long);
    static void slewTime(unsigned int, int);
    static unsigned long long now();
    static unsigned long long ucNow();
    static void debugTimeChanges(bool);
  private:
    static int calculateSlew();
    static void freezeSlew();
};

#endif
