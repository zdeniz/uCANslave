#/*
 * File:   main.c
 * Author: Papa
 *
 * Created on 23 ?????? 2021 ?., 15:16
 */
#include <xc.h>
#include "config.h"
#include "appcfg.h"
#include "const.h"
#include "data.h"
#include "timers.h"
#include "init252.h"
#include "states.h"
#include "uUART.h"
#include "nmt.h"


enum {
    eCAN_NODE, //nodeID
    eCAN_SPEED //CAN speed
};
__EEPROM_DATA(CAN_NODE, CAN_SPEED, 2, 3, 4, 5, 6, 7);

UNS8 nodeID;
unsigned char digital_input[1] = {0};
unsigned char digital_output[1] = {0};

CANPACK canPack;
MESSAGE uMessage;

// CANOPEN_NODE_MASTER_DATA
CO_Data uData;

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

UNS8 getID(void) {
    UNS8 node = eeprom18_read(eCAN_NODE);
    return node;
};

//extern const UNS16 objDictSize;

void openInit(void) {
    extern UNS16 obj1017;
    obj1017 = 3;
    uData.nodeId = 0xFF;
    uData.nodeState = Unknown_state;
    uData.canFlags.iam_a_slave = 1;
    //    uData.ObjdictSize = &objDictSize;
};

void main(void) {
    NOP();

    NOP();
/*
    uMessage.node = 127;
    uMessage.cmd = 15;
    uMessage.rtr = 1;
    uMessage.len = 3;
    uMessage.mdata[0] = 3;
    uMessage.mdata[1] = 2;
    uMessage.mdata[2] = 1;
        loadEngine();
    canSend(&uMessage);
 */
    initChip(); // Initialize pic configuration    
    openInit(); // Initialize CANOPEN stack
    nodeID = getID(); // Read node ID from switch
    setNodeId(nodeID); //  Set and configuration default SDO PDO EMCY
    //// Start timer & UART for the CANopen stack
    initCAN(eeprom18_read(eCAN_SPEED)); // Initialize the CANopen driver
    initINT(); //Enable communication & timers

    setState(Initialisation); // Initialisation and set Pre_operational

    for (;;) // forever loop
    {
        if (uData.canFlags.sysTickFlag) // Cycle timer, invoke action on every time slice
        {
            uData.canFlags.sysTickFlag = 0; // Reset timer

            TimeDispatch();
            NOP();
            NOP();
            //           digital_input[0] = get_inputs();
            //           digital_input_handler(&ObjDict_Data, digital_input, sizeof (digital_input));
            //           digital_output_handler(&ObjDict_Data, digital_output, sizeof (digital_output));
            //           set_outputs(digital_output[0]);

            // Check if CAN address has been changed
            //           if (!(nodeID == read_bcd())) {
            //               nodeID = read_bcd(); // Save the new CAN adress
            //               setState(Stopped); // Stop the node, to change the node ID
            //               setNodeId(nodeID); // Now the CAN adress is changed
            //               setState(Pre_operational); // Set to Pre_operational, master must boot it again
            //               //            }
            //           }
        }
        // a message was received pass it to the CANstack
        if (canReceive(&uMessage)) // a message reveived
            canDispatch(); // process it
        else {
            // Enter sleep mode
#ifdef WD_SLEEP		// Watchdog and Sleep
            wdt_reset();
            sleep_enable();
            sleep_cpu();
#endif				// Watchdog and Sleep
        }
        CLRWDT();
    }



    return;
}

