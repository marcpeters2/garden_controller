#include "Utils.h"

extern "C" char *sbrk(int i);

size_t Utils::freeRAM(void)
{
  char stack_dummy = 0;
  return(&stack_dummy - sbrk(0));
}
