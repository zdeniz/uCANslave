#include "epprom.h"
#include "data.h"
#include "ucan.h"

__EEPROM_DATA(CAN_NODE, CAN_SPEED, GAP_TIME, RCV_MASK, CRC_MASK, 5, 6, 7);
__EEPROM_DATA(1, 3, 7, 12, 25, 51, 103, 207);
__EEPROM_DATA(3, 4, 5, 6, 8, 12, 24, 48);

unsigned char eeprom18_read(unsigned char offset) {
    unsigned char eecon = EECON1;
    EEADR = offset;

    EECON1bits.EEPGD = 0; //accesses data EEPROM memory
    EECON1bits.CFGS = 0; //accesses data EEPROM memory
    EECON1bits.RD = 1; //initiates an EEPROM read

    Nop(); //it can be read after one NOP instruction
    EECON1 = eecon; // Restore EECON1

    return EEDATA;
}

void eeprom18_write(unsigned char offset, unsigned char value) {
    unsigned char eecon = EECON1; //Saved EECON1

    EEDATA = value;
    EEADR = offset;

    EECON1bits.EEPGD = 0; //accesses data EEPROM memory
    EECON1bits.CFGS = 0; //accesses data EEPROM memory
    EECON1bits.WREN = 1; //allows write cycles

    di(); //interrupts be disabled during this code segment
    EECON2 = 0x55; //write sequence unlock
    EECON2 = 0xAA; //write sequence unlock
    EECON1bits.WR = 1; //initiates a data EEPROM erase/write cycle
    while (EECON1bits.WR); //waits for write cycle to complete
    ei(); //restore interrupts

    EECON1bits.WREN = 0; //disable write
    EECON1 = eecon; // Restore EECON1
}