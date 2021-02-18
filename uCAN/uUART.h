#ifndef UDRIVER_H
#define	UDRIVER_H
#include <xc.h> // include processor files - each processor file is guarded. 
#include "data.h"
#include "leds.h"

#define TICK_TIME 19       //1мс = 100 * 10 
#define LAWICEL_TICK    1000    //1сек = 1мс * 1000
#define GAP_TIME    3
#define RCV_MASK 0x08
#define HI_SEND 0xFE
#define LO_SEND 0x80
#define LEN_PACK    2   //номера посылок в пакете 0 и 1, номер CRC =LEN_PACK= 2 
#define MAX_REPIT    5
#define CRC_MASK    0x31
#define DEV_ADDR    0x01

enum state {
    S_IDL = 0, S_RCV, S_SND, S_ERR
};

typedef union {
    unsigned char flag;

    struct {
        unsigned sndRPT : 3;
        unsigned echoTX : 1;
        unsigned        : 1;
        unsigned        : 1;        
        unsigned sndRDY : 1;
        unsigned rcvRDY : 1;
    };
}
BYTESTATE;

typedef union {
    unsigned char flag;

    struct {
        unsigned drv : 1;   /**ошибока UART                         :01*/
        unsigned bus : 1;   /**ошибока приоритета шины               :02*/
        unsigned crc : 1;    /**ошибока CRC                          :04*/    
        unsigned frm : 1;   /**ошибока фрейма                        :08*/
        unsigned ovr : 1;   /**ошибока переполнения буфера приема    :10*/
        unsigned rpt : 1;   /**ошибока передачи                      :20*/
        unsigned : 1;       /**резев                                  :40*/
        unsigned act : 1;   /**флаг завершения активности шины     :80*/
    };
}
BYTEERROR;

void USBDeviceTasks(void);


unsigned char initCAN(unsigned char num);
inline void setMult(unsigned char num);
void loadEngine(void);
void resetEngine(void);
void handlerTMR0(void);
inline void handlerRXD(void);
unsigned char nextBit(void);

void nextSend(void);
void doPack(unsigned char pack[]);
unsigned char doCRC(unsigned char crc, unsigned char byte);

//API calls
void setEchoTX(void);
void clrEchoTX(void);
unsigned char *ptrRxD(void);
unsigned char *ptrTxD(void);
unsigned char isTxD(void);
void doTxD(void);
unsigned char sendAct(void);
unsigned char isRxD(void);
void clrRxD(void);
unsigned char isERR(void);
unsigned char canReceive(MESSAGE *m);
unsigned char canSend(MESSAGE *m);

#endif	/* UDRIVER_H */

