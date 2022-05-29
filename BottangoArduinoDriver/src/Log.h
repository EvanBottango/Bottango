
#ifndef BOTTANGO_LOG_H
#define BOTTANGO_LOG_H

// #define BOTTANGO_DEBUG

#include "Arduino.h"

#define MKBUF char buffer[8];

#define PRINT(msg) Serial.print(msg);
#define PRINT_LN(msg)  \
    Serial.print(msg); \
    Serial.print("\n");
#define PRINT_NEWLINE() Serial.print("\n");
#define PRINT_INT(i)               \
    sprintf(buffer, "%d", (int)i); \
    Serial.print(buffer);
#define PRINT_ULONG(l)         \
    sprintf(buffer, "%lu", l); \
    Serial.print(buffer);
#define PRINT_LONG(l)          \
    sprintf(buffer, "%ld", l); \
    Serial.print(buffer);

#define PRINT_FLOAT(flt)                       \
    {                                          \
        char str[20];                          \
                                               \
        float tmpVal = (flt < 0) ? -flt : flt; \
                                               \
        int tmpInt1 = tmpVal;                  \
        if (flt < 0)                           \
            PRINT("-")                         \
        PRINT_INT(tmpInt1)                     \
        float tmpFrac = tmpVal - tmpInt1;      \
        int tmpInt2 = trunc(tmpFrac * 10000);  \
        PRINT(".")                             \
        PRINT_INT(tmpInt2)                     \
    }

#ifdef BOTTANGO_DEBUG

#define LOG_MKBUF MKBUF;
#define LOG(msg) PRINT(msg);
#define LOG_LN(msg) PRINT_LN(msg);
#define LOG_NEWLINE() PRINT_NEWLINE();
#define LOG_INT(i) PRINT_INT(i);
#define LOG_ULONG(l) PRINT_ULONG(l);
#define LOG_LONG(l) PRINT_LONG(l);
#define LOG_FLOAT(flt) PRINT_FLOAT(flt);

#else

#define LOG_MKBUF ;
#define LOG(msg) ;
#define LOG_LN(msg) ;
#define LOG_NEWLINE() ;
#define LOG_INT(i) ;
#define LOG_ULONG(l) ;
#define LOG_LONG(l) ;
#define LOG_FLOAT(flt) ;

#endif //BOTTANGO_DEBUG

#endif //BOTTANGO_LOG_H
