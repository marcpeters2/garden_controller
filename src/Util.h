#ifndef UTIL_H
#define UTIL_H

#include <limits.h>

class Util {
  public:
    static size_t freeRAM(void);
    static String toString(unsigned long long);
    static int parseIntFromString(char*);
    static unsigned long long parseULongLongFromString(char*);
    static int charToInt(char c);
    static bool charIsNumeric(char c);
    static unsigned long long now(unsigned long long);
    static void enableTimer4Interrupts();
    static void disableTimer4Interrupts();
    static void printWelcomeMessage();
};

#endif
