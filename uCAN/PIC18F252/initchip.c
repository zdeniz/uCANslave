
/*
 *Функции нициализации МК
 */
#include <xc.h> 
#include "initchip.h"
#include "../epprom.h"
//------------------------------------------------------------------------------
/**
 * \file
 * \brief Initialization of registers PIC port configuration
 */
//------------------------------------------------------------------------------

void initChip(void) {
    initCPU();
    initPORTA();
    initPORTB();
    initPORTC();
    initWDT();
}
//------------------------------------------------------------------------------

void initCPU(void) {
    //init_OSC
    OSCCONbits.SCS = 0;

}
//------------------------------------------------------------------------------

void initPORTA(void) {
    //init_PORTA
    PORTA = 0x00;
    ADCON1 = 0x07; // all analog to digital I/O      
    TRISA = 0xFF; //all pins input
    LATA = 0; //all pins set 0
}
//------------------------------------------------------------------------------

void initPORTB(void) {
    //init_PORTB
    PORTB = 0x00;
    TRISB = 1; //all pins out, 0-in
    LATB = 0; //all pins set 0
}

void initPORTC(void) {
    //init_PORTC
    PORTC = 0x00;
    TRISC = 0xF0; //low pins out, high pins in
    LATC = 0; //all pins set 0
}

//----------------------------------------------------------------------

/**
 * Инициализация UART
 */

unsigned char eeprom18_read(unsigned char offset);

void initUART(unsigned char num) {
    //   ANSELHbits.ANS11 = 0;

    TRISBbits.RB6 = 0; //TxD 
    TRISBbits.RB7 = 1; //RxD
    //SYNC=0 BRGH=1 BRG16=1   ->  Fosc/4*[N+1]    
    //   BAUDCON = 0;
    //   BAUDCONbits.BRG16 = 1; // BRG16 16 bit rate generator
    //    setSpeed(0);
    TXSTA = 0;
    RCSTA = 0;
    TXSTAbits.TXEN = 1; // TX Enable TX transmission
    RCSTAbits.CREN = 1; // CREN Enable RX receive
    TXSTAbits.SYNC = 0; // TX Asynchronous SYNC mode
    TXSTAbits.BRGH = 1; // high speed BRGH
    SPBRG = eeprom18_read(eTAB_SPEED + num);
    RCSTAbits.SPEN = 1; // SPEN Enable UART
    // Setting the interrupt priority
    IPR1bits.RCIP = 1; //HI priority RxD
}
//----------------------------------------------------------------------

void initTMR0(void) {
    //init_TMR0

    TMR0H = 0xFF;
    TMR0L = 0xA8;
    T0CON = 0b10000000;
    // Prescaler 1: 4, clocking Fosc / 4,
    // one tick 1000000/256 = 0.064μs (us)
    // Set the priority of the interrupt
    INTCON2bits.TMR0IP = 1; //HI priority
}
//------------------------------------------------------------------------------

void initWDT(void) {
    //init_WDT

    WDTCON = 0b00010010; //512ms
    //        WDTCONbits.SWDTEN=1;
}

void initINT(void) {
    // Enable Timer 0 Interrupt
    INTCONbits.TMR0IF = 0;
    INTCONbits.TMR0IE = 1;
    // Enable Timer 1 Interrupt
    PIE1bits.TMR1IE = 0;

    // Enable interrupt from INT0
    INTCON2bits.INTEDG0 = 0; // interrupt on falling edge
    INTCONbits.INT0IF = 0;
    INTCONbits.INT0IE = 0; // Enable interrupt from INT0

    PIE1bits.RCIE = 1; // Enable interrupt from UART receiver

    RCONbits.IPEN = 1; // enable interrupt priority
//    INTCONbits.PEIE = 1; // Enable peripheral interrupts
    INTCONbits.GIEH = 1; // enable high priority interrupts
    //    INTCONbits.GIEL = 0; // enable low priority interrupts
}
