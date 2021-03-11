#ifndef EPPROM_H
#define	EPPROM_H

#include <xc.h> // include processor files - each processor file is guarded. 
#include "const.h"

enum {
    eCAN_NODE, //nodeID 0
    eCAN_SPEED,
    ePACK_GAP,   //CAN speed 2
    eBIT_MASK,
    eCRC_POLY,  //CRC polynom 4
    
    eTAB_SPEED = 8,
    eTAB_MULT = 16
 };


unsigned char eeprom18_read(unsigned char offset);
void eeprom18_write(unsigned char offset, unsigned char value);

#endif	/* EPPROM_H */

