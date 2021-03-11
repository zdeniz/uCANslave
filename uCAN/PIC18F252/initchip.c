
/*
 *Функции нициализации МК
 */
#include <xc.h> 
#include "initchip.h"
#include "../epprom.h"
//------------------------------------------------------------------------------
/**
 * \file
 * \brief Инициализация регистров  конфигурация портов PIC
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
    //   BAUDCONbits.BRG16 = 1; //BRG16 16-битный генератор скорости
    //    setSpeed(0);
    TXSTA = 0;
    RCSTA = 0;
    TXSTAbits.TXEN = 1; //TX Включить TX передачу
    RCSTAbits.CREN = 1; //CREN Включить RX прием
    TXSTAbits.SYNC = 0; //TX Асинхронный режим SYNC
    TXSTAbits.BRGH = 1; //высокоскоростной BRGH
    SPBRG = eeprom18_read(eTAB_SPEED + num);
    RCSTAbits.SPEN = 1; //SPEN Включить УАРТ 
    //Устанавливаем приориет прерывания
    IPR1bits.RCIP = 1; //HI priority RxD
}
//----------------------------------------------------------------------

void initTMR0(void) {
    //init_TMR0

    TMR0H = 0xFF;
    TMR0L = 0xA8;
    T0CON = 0b10000000;
    //Прескалер 1:4, тактирование Fosc/4,
    //один тик 1000000/256 = 0,064мкс (us)
    //Устанавливаем приориет прерывания   
    INTCON2bits.TMR0IP = 1; //HI priority
}
//------------------------------------------------------------------------------

void initWDT(void) {
    //init_WDT

    WDTCON = 0b00010010; //512ms
    //        WDTCONbits.SWDTEN=1;
}

void initINT(void) {
    //Разрешаем прерывание от Таймера 0
    INTCONbits.TMR0IF = 0;
    INTCONbits.TMR0IE = 1;
    //Разрешаем прерывание от Таймера 1
    PIE1bits.TMR1IE = 0;

    //Разрешаем прерывание от INT0
    INTCON2bits.INTEDG0 = 0; //прерывание по падающему фронту
    INTCONbits.INT0IF = 0;
    INTCONbits.INT0IE = 0; //Разрешаем прерывание от INT0

    PIE1bits.RCIE = 1; //Разрешение прерывание от приемника УАРТ

    RCONbits.IPEN = 1; //разрешаем приоритет прерываний
//    INTCONbits.PEIE = 1; //Разрешаем прерывания от переферии
    INTCONbits.GIEH = 1; //разрешаетвысокоприоритетные прерывания
    //    INTCONbits.GIEL = 0; //разрешает низкоприоретееные прерывания    
}
