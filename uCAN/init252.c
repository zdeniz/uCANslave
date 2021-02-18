
/*
 *Функции нициализации МК
 */
#include <xc.h> 
#include "init252.h"

//                                 250, 125, 76.8, 38.4, 19.2, 9.6 | 4.8   2.4 
const unsigned char speedConst[8] = {3, 7, 12, 24, 51, 103, 206, 103};
const unsigned char divConst[8] = {1, 2, 4, 8, 12, 24, 48, 96};

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
    TRISB = 0; //all pins out
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
 * Установить скорость uCAN и множитель GAP
 * 250, 125, 76.8, 38.4, 19.2, 9.6 | 4.8   2.4 
 * @param num - номер параметра скорости
 */
inline void setSpeed(unsigned char num) {
    if (num < 6) TXSTAbits.BRGH = 1; //высокоскоростной BRGH
    else TXSTAbits.BRGH = 0; //низкоскоростной BRGH
    SPBRG = speedConst[num]; //RES_LO; -младший байт скорости
}
//----------------------------------------------------------------------

/**
 * Инициализация UART
 */
void initUART(void) {
    //   ANSELHbits.ANS11 = 0;
    TRISBbits.RB6 = 0; //TxD 
    TRISBbits.RB7 = 1; //RxD
    //SYNC=0 BRGH=1 BRG16=1   ->  Fosc/4*[N+1]    
    //   BAUDCON = 0;
    //   BAUDCONbits.BRG16 = 1; //BRG16 16-битный генератор скорости
    setSpeed(0);
    TXSTA = 0;
    TXSTAbits.SYNC = 0; //TX Асинхронный режим SYNC
    //    TXSTAbits.BRGH = 1; //высокоскоростной BRGH
    TXSTAbits.TXEN = 1; //TX Включить TX передачу
    RCSTA = 0;
    RCSTAbits.SPEN = 1; //SPEN Включить УАРТ   
    RCSTAbits.CREN = 1; //CREN Включить RX прием
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
    INTCONbits.INT0E = 0; //Разрешаем прерывание от INT0

    //    INTCONbits.PEIE = 1; //Разрешаем прерывания от переферии
    RCONbits.IPEN = 1; //разрешаем приоритет прерываний

 //   PIE1bits.RCIE = 1; //Разрешение прерывание от приемника УАРТ


    INTCONbits.GIEH = 1; //разрешаетвысокоприоритетные прерывания
//    INTCONbits.GIEL = 0; //разрешает низкоприоретееные прерывания    
}
