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
#include "PIC18F252/initchip.h"
#include "states.h"
#include "ucan.h"
#include "nmt.h"
#include "epprom.h"

UNS8 nodeID;
unsigned char digital_input[1] = {0};
unsigned char digital_output[1] = {0};

CANPACK canPack;
MESSAGE uMessage;

// CANOPEN_NODE_MASTER_DATA
CO_Data uData;

UNS8 getID(void) {
    UNS8 node = eeprom18_read(eCAN_NODE);
    return node;
};

//extern const UNS16 objDictSize;

void initCANOPEN(void) {
    extern UNS16 obj1017;
    obj1017 = 1000;
    uData.nodeId = 0xFF;
    uData.nodeState = Unknown_state;
    uData.canFlags.iam_a_slave = 1;
    uData.canFlags.iam_autostart = 1;
    //    uData.ObjdictSize = &objDictSize;
};
UNS8 sendHeartBeat(void);

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
    initCANOPEN(); // Initialize CANOPEN stack
    nodeID = getID(); // Read node ID from EEPROM
    setNodeId(nodeID); //  Set and configuration default SDO PDO EMCY
    initCAN(eeprom18_read(eCAN_SPEED)); // Initialize the CANopen driver
    // Start timer & UART for the CANopen stack
    initINT(); //Enable communication & timers

    setState(Initialisation); // Initialisation and set Pre_operational

    for (;;) // forever loop
    {
        if (uData.canFlags.sysTickFlag) // Cycle timer, invoke action on every time slice
        {
            uData.canFlags.sysTickFlag = 0; // Reset timer
            //            sendHeartBeat();
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

