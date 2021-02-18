  
#ifndef APPCFG_H
#define	APPCFG_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include <limits.h>
// Integers
#define INTEGER8 signed char
#define INTEGER16 signed short
#define INTEGER24 signed short long
#define INTEGER32 signed long
#define INTEGER40
#define INTEGER48
#define INTEGER56
#define INTEGER64

// Unsigned integers
#define UNS8   unsigned char
#define UNS16  unsigned short
#define UNS24  unsigned short long
#define UNS32  unsigned long

typedef union {
    unsigned int value;

    struct {
        unsigned char lo;
        unsigned char hi;
    };
} byteOfWord;

/* CANopen usefull helpers */
//#define GET_NODE_ID(m)         (UNS16_LE(m.cob_id) & 0x7f)
//#define GET_FUNCTION_CODE(m)   (UNS16_LE(m.cob_id) >> 7)
#endif	/* APPCFG_H */

