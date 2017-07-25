#ifndef UTIL_H
#define UTIL_H

#include <limits.h>

#define MAX_TIME_SLEW_DURATION 60000U

class Util {
  public:
    static size_t freeRAM(void);
    static String toString(unsigned long long);
    static int parseIntFromString(char*);
    static unsigned long long toULL(char*);
    static int charToInt(char c);
    static bool charIsNumeric(char c);
    static void setTime(unsigned long long);
    static void slewTime(unsigned int, int);
    static unsigned long long now();
    static unsigned long long ucNow();
    static void enableTimer4Interrupts();
    static void disableTimer4Interrupts();
    static void printWelcomeMessage();
    static void T4interrupts();
    static void noT4interrupts();
  private:
    static int calculateSlew();
    static void freezeSlew();
};

#endif
