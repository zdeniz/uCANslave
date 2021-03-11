
#include "leds.h"

void led0ON(void) {
    LATCbits.LATC0 = 1;
}

void led1ON(void) {
    LATCbits.LATC1 = 1;
}

void led2ON(void) {
    LATCbits.LATC2 = 1;
}

void led3ON(void) {
    LATCbits.LATC3 = 1;
}

void allOFF(void) {
    LATCbits.LATC0 = 0;
    LATCbits.LATC1 = 0;
    LATCbits.LATC2 = 0;
    LATCbits.LATC3 = 0;
}

void stateLED(unsigned char state) {
};

void stepLED(unsigned char state) {
    LATCbits.LATC0 = 0;
    LATCbits.LATC1 = 0;
    if (state == 3) { //Error
        LATCbits.LATC0 = 1;
        LATCbits.LATC1 = 1;
        return;
    }
    if (state == 2) { //read
        LATCbits.LATC0 = 1;
        return;
    }
    if (state == 1) { //write
        LATCbits.LATC1 = 1;
        return;
        //idle

    }
};

/*
    allOFF();
    if (state == 4) {       //Stopped
        led0ON();
        return;
    }
    if (state == 0x7F) {        //Pre_operational
        led1ON();
        return;
    }
    if (state == 05) {      //Operational
        led0ON();
        led1ON();
        return;
    }
    if (state)led2ON();
}

void        ledTxEoff(void){
    
};
void        ledTxEon(void){
    
};        
void        ledRxEoff(void){
    
};
void        ledRxEon(void){
    
};
 */

void ledBeat(void) {
    LATCbits.LATC3 = LATCbits.LATC3^1;
}

void ledNMT(void) {
    LATCbits.LATC2 = LATCbits.LATC2^1;
}