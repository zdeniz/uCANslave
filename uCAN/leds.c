
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
    allOFF();
    if (state == 4) {
        led0ON();
        return;
    }
    if (state == 0x7F) {
        led1ON();
        return;
    }
    if (state == 05) {
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
void ledBeat(void) {
    LATCbits.LATC3 =LATCbits.LATC3^1;
}