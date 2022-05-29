#include "Log.h"

void printFreeRam()
{
#ifdef BOTTANGO_DEBUG
    extern int __heap_start, *__brkval;
    int v;
    int freeRam = (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);

    LOG_MKBUF;
    LOG(F("Free Ram: "));
    LOG_INT(freeRam);
    LOG_NEWLINE();
#endif
}