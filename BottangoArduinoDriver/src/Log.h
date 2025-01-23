
#ifndef BOTTANGO_LOG_H
#define BOTTANGO_LOG_H

// #define BOTTANGO_DEBUG

#include "Arduino.h"
#include "Outgoing.h"

#define MKBUF char buffer[8];

#define PRINT(msg) Outgoing::printOutputString(msg);
#define PRINT_LN(msg)                 \
    Outgoing::printOutputString(msg); \
    Outgoing::printLine();
#define PRINT_NEWLINE() Outgoing::printLine();
#define PRINT_INT(i)               \
    sprintf(buffer, "%d", (int)i); \
    Outgoing::printOutputString(buffer);
#define PRINT_ULONG(l)         \
    sprintf(buffer, "%lu", l); \
    Outgoing::printOutputString(buffer);
#define PRINT_LONG(l)          \
    sprintf(buffer, "%ld", l); \
    Outgoing::printOutputString(buffer);

#define PRINT_FLOAT(flt)                  \
    {                                     \
        char str[20];                     \
                                          \
        sprintf(str, "%.4f", flt);        \
        Outgoing::printOutputString(str); \
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

#endif // BOTTANGO_DEBUG

#endif // BOTTANGO_LOG_H
