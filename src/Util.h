#ifndef UTIL_H
#define UTIL_H

#include <limits.h>
#include <math.h>
#include "Arduino.h"

class Util {
  public:
    static size_t freeRAM(void);
    static String toString(unsigned long long);
    static int parseIntFromString(char*);
    static unsigned long long toULL(char*);
    static int charToInt(char c);
    static bool charIsNumeric(char c);
    static void enableTimer4Interrupts();
    static void disableTimer4Interrupts();
    static void printWelcomeMessage();
    static void T4interrupts();
    static void noT4interrupts();
};

#endif
