
//PAPA
#include "initchip.h"
#include "drive.h"
#include "../ucan.h"
#include "..\data.h"
#include "..\leds.h"
#include "..\epprom.h"
//------------------------------------------------------------------------------
/**
 * \file
 * \brief Interrupt Handler
 * 
 * Timer T0
 * formation of an interval of interruptions every 50 μs,
 * interval counter 1ms,
 * formation of inter-packet interval of bus inactivity,
 * transition to the transfer mode when the bus is inactive.
 *
 * USART receiver
 * receive a byte in the interrupt,
 * subsequent output of the byte in transfer mode.
 *
 * Bus activity monitoring by interrupt INT0
 * detection of falling level on the bus (bus activity).
 *
 */

//----------------------------------------------------------------------
extern CO_Data uData;
extern tConst CONST;
extern unsigned char actStep, divCnt, countTick;
extern unsigned int systemTick;
        
void stepLED(unsigned char state);
void timeCtlEngine(void);
void timeCtlEngine(void);
void setStateReceive(void);
void packActEngine(void);

void __interrupt(high_priority) hiINT(void) {
    if (INTCONbits.TMR0IF && INTCONbits.TMR0IE) {
        stepLED(actStep);
        INTCONbits.TMR0IF = 0;
        TMR0H = 0xFF;
        TMR0L = 0xA9;
        if (countTick) {
            countTick--; // count in 100μs steps (GAP steps) up to 10 == 1ms
        } else {
            countTick = TICK_TIME; // count up to 1ms (TICK_TIME = 10)
            uData.canFlags.sysTickFlag = 1; // I raise the flag 1ms
            systemTick++; // count the system time in milliseconds
        }
        divCnt--;
        if (!divCnt) {
            divCnt = CONST.gapMult;
            timeCtlEngine(); // every GAP interval = 100ms * gap.Mult call the handler
        }
    }
    if (INTCONbits.INT0E && INTCONbits.INT0F) {
        setStateReceive(); // switch to receive mode
    }
    if (PIE1bits.RCIE && PIR1bits.RCIF) {
        packActEngine();
    }
    /*
    if (USBIF && USBIE) {
        USBDeviceTasks();
    }
     */
}
//----------------------------------------------------------------------

