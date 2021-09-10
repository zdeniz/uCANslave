
//PAPA
#include "ucan.h"

#include "PIC18F252/initchip.h"
#include "data.h"
#include "epprom.h"
#include "leds.h"
//------------------------------------------------------------------------------
/**
 * \file
 * \brief RsCAN driver and API calls.
 * 
* Timer
 * Interpacket interval 100μs, interval counter 1ms, interval counter 1s.
 * Receiver
 * UART with myCAN packet reception and transmission
 * Control
 * activity of the bus INT0 interrupts
 *
 */
//------------------------------------------------------------------------------
extern CO_Data uData;

unsigned int systemTick;  ///<system time

static volatile unsigned char divCnt;   /// <packet interval multiplier counter
static volatile unsigned char gapCnt;   /// <packet interval counter
static volatile unsigned char actStep;  /// <active state (state machine)

static volatile BYTESTATE STATE;  /// <status and control bits
static volatile BYTEERROR ERROR;  /// <status and control bits

volatile unsigned char countTick;  /// <CAN time slot counter - 1ms

static unsigned char bufferTx[12];  /// <transfer buffer
static unsigned char bufferRx[12];  /// <receive buffer

tConst CONST;

struct {
    // packet counters bits, bytes, packet size control
    unsigned char ptrPack;  /// <received bit number 0-80
    unsigned char lenPack;  /// <length of the received packet in bytes 0-8
    unsigned char cntPack;  /// <byte counter in packet 0-10
    unsigned char lenData;  /// <data block length in bytes 0-8
    // counters and variables of bit receive / transmit bytes
    // unsigned char rBit; /// <received bit
    // unsigned char sBit; /// <transmitted bit
    unsigned char nBit;     /// <bit number in the current byte
    unsigned char posMask;  /// <bit position mask in byte
    // variables for receiving / transmitting bytes
    unsigned char tmpByte;  /// <working variable
    unsigned char sndByte;  /// <transmitted byte
    unsigned char rcvByte;  /// <received byte

    unsigned char crcByte;  /// <checksum byte
} MYCAN;
//----------------------------------------------------------------------
//----------------------------------------------------------------------

/**
 * RsCAN driver initialization
 * 0-115; 1-57.6; 2-38.4; 3-19.2; 4-9.6; 5-4.8; 6-2.4; 7-1.2
 * @param num - speed parameter number
 */
unsigned char initCAN(unsigned char num) {
    loadConstEngine();
    initPackEngine();
    initUART(num);
    setMult(num);
    initTMR0();
    return 1;
}
//----------------------------------------------------------------------
unsigned char eeprom18_read(unsigned char offset);

inline void setMult(unsigned char num) {
    CONST.gapMult = eeprom18_read(eTAB_MULT + num);
}

inline void loadConstEngine(void) {
    CONST.rcvMask = eeprom18_read(eBIT_MASK);  // data fetch bit mask
    CONST.crcMask = eeprom18_read(eCRC_POLY);  // CRC polynomial
    CONST.gapTime = eeprom18_read(ePACK_GAP);
}
//----------------------------------------------------------------------

/**
 * Guard interpacket separation handler
 *
 * At the penultimate n-1 count of the interval after the activity, we initialize the driver
 * At the last one, we start the transfer, if there is a ready package
 * For maximum channel utilization, 3-2 GAP counts are recommended.
 */
inline void timeCtlEngine(void) {
    if (gapCnt > 0) {
        LATBbits.LATB7 = LATBbits.LATB7 ^ 1;
        gapCnt--;                    // decrease the GAP counter
        if (gapCnt == 0) {           // in the last GAP interval
            if (actStep != S_IDL) {  // if there was activity
                ERROR.act = 1;
                initPackEngine();  // initialize the transmit / receive counters
                setStateIdle();    // stand in idle mode
            }
        }
    } else {
        LATBbits.LATB4 = 0;
        actNextSend();
    }
}
//----------------------------------------------------------------------

inline void initPackEngine(void) {
    MYCAN.crcByte = 0;
    MYCAN.cntPack = 0;
    MYCAN.ptrPack = 0;
    MYCAN.lenPack = LEN_PACK;
    MYCAN.nBit = 8;
    MYCAN.posMask = 0x80;
}
//----------------------------------------------------------------------

/**
 * Start the transfer of the packet
 *
 * If the transfer is incomplete and there are still pending replays, then
 * - we prohibit the transition to the receive mode,
 * - data to the transfer register
 * - further transmission in the receive control interrupt mode
 * otherwise
 * -setting the error flag
 * -setting the transfer completion flag
 * 
 * * @return none
 */

inline void setStateSend(void) {
    INTCONbits.INT0IE = 0;  // disable
    INTCONbits.INT0IF = 0;  // reset the receive interrupt
    actStep = S_SND;
}

inline void setStateReceive(void) {
    INTCONbits.INT0E = 0;  // disable interrupt from INT0
    INTCONbits.INT0F = 0;  // clear the flag
    actStep = S_RCV;
}

inline void setStateIdle(void) {
    INTCONbits.INT0IF = 0;
    INTCONbits.INT0IE = 1;  // receive interrupt active
    actStep = S_IDL;        // wait for reception or transmission
}

inline void actNextSend(void) {
    if (STATE.sndRDY) {  // is the transfer complete flag set?
        return;          // do nothing
    }
    if (STATE.sndRPT)  // number of retries exceeded?
    {
        STATE.sndRPT--;  // No

        setStateSend();  // switch to transfer mode
        ERROR.flag = 0;
        MYCAN.sndByte = bufferTx[0];
        TXREG = nextBit();  // first went (transmission)

        return;
    }
    ERROR.rpt = 1;     // exceeded the number of repetitions
    STATE.sndRDY = 1;  // end transfers
}
//----------------------------------------------------------------------
//----------------------------------------------------------------------

/**
 * Data packet reception handler
 *
 * receive bit, control UART
 * priority control (0 to 12 bits), transition to receiving a priority packet
 * determination of the length of the packet (after receiving the 15th bit)
 * control overflow of the receive buffer (before writing the first received byte)
 * CRC control
 * Control of the actual packet length
 * Setting flags of completion of reception / transmission
 */

inline void packActEngine(void) {
    gapCnt = CONST.gapTime;
    LATBbits.LATB4 = 1;
    //     if (actStep == S_IDL) {actStep = S_RCV;}
    //----------------- receive bit -------------------------
    MYCAN.tmpByte = RCREG;  // read byte

    //----------------- receiver control ------------------------
    if (RCSTAbits.FERR || RCSTAbits.OERR)  // Were there any mistakes?
    {
        // In case of receiver error
        RCSTAbits.CREN = 0;  // Disable the receiver to reset the error
        NOP();
        // Turn on the receiver to continue the work of UART
        RCSTAbits.CREN = 1;
        ERROR.drv = 1;
        actStep = S_ERR;
        return;
    }
    // ----- state ERR, received a bit and exit, wait for the end of reception via GAP
    if (actStep == S_ERR) {
        return;
    }
    // --------- further in operating mode RCV or SND --------
    if (MYCAN.tmpByte & CONST.rcvMask) {                // what was accepted?
        STATE.rBit = 1;                                 // unit
        MYCAN.rcvByte = MYCAN.rcvByte | MYCAN.posMask;  // commit 1 to byte
    } else {
        STATE.rBit = 0;                                    // zero
        MYCAN.rcvByte = MYCAN.rcvByte & (~MYCAN.posMask);  // commit 0 to byte
    }
    // -------- bit is accepted, add it to the checksum of the packet -------
    MYCAN.crcByte = MYCAN.crcByte & 0x80 ? (((MYCAN.crcByte << 1) & STATE.rBit) ^ CONST.crcMask) : ((MYCAN.crcByte << 1) & STATE.rBit);

    // ----- Transmission collision control, is there a priority bit? -------
    if (actStep == S_SND) {
        if (STATE.rBit != STATE.sBit) { // bits matched
            if ((MYCAN.ptrPack < 12) && (STATE.rBit == 0)) {
                // no! valid for the first 12 bits - dominant bit received?
                // Yes! this is the priority / address bit
                // insert the bit into the packet and switch to RCV receive mode
                actStep = S_RCV;
            } else {
                // No! bus error
                ERROR.bus = 1;
                actStep = S_ERR;
                return;
            }
        }
        // data on the bus is actual SND transmission mode
    }
    if (MYCAN.ptrPack == 15) {  // last bit of header received
        // take the length of the packet from the received byte
        MYCAN.lenData = MYCAN.rcvByte & 0x0F;
        MYCAN.lenPack = MYCAN.lenPack + MYCAN.lenData;
    }
    // proceed to receive the next bit
    MYCAN.posMask = MYCAN.posMask >> 1;
    MYCAN.nBit--;
    MYCAN.ptrPack++;

    if (MYCAN.nBit == 0) {  // all bits of the byte received / transmitted?
        MYCAN.nBit = 8;     // YES! initiate the reception / transmission of the next byte
        MYCAN.posMask = 0x80;
        // if after receiving the first byte the read flag has not yet been cleared - OVERFLOW
        // received byte into the receive buffer and take the next one from the transmit buffer
        if (STATE.rcvRDY) {  // control overflow of the receive buffer
            ERROR.ovr = 1;   // receive buffer not cleared
        }
        if (MYCAN.cntPack < MYCAN.lenPack) {          // was the packet accepted?
            bufferRx[MYCAN.cntPack] = MYCAN.rcvByte;  // no, accept more
        } else {                                      // finish receiving the packet? CRC reception
            if (MYCAN.cntPack == MYCAN.lenPack) {     // received CRC
                // yes, the packet was accepted.
                if (MYCAN.crcByte == 0) {
                    // In the checksum zero - everything is OK!
                    if (actStep == S_RCV) {  // receive completed
                    echoTxD:
                        STATE.rcvRDY = 1;  // no errors, packet received
                        return;
                    }
                    if (actStep == S_SND) {  // complete transfer
                        STATE.sndRDY = 1;
                        if (STATE.echoTX) {
                            goto echoTxD;
                        }
                        return;
                    }
                } else {  // common CRC error
                    ERROR.crc = 1;
                    actStep = S_ERR;  // CRC error
                    return;
                }
            } else {  // frame error
                ERROR.frm = 1;
                actStep = S_ERR;  // frame error
                return;
            }
        }
        MYCAN.cntPack++;                                  // next byte
        if (actStep == S_SND) {                           // pass?
            if (MYCAN.cntPack <= MYCAN.lenPack) {         // end of transfer?
                MYCAN.sndByte = bufferTx[MYCAN.cntPack];  // no, pass more
            } else {                                      // shouldn't be, frame error
                ERROR.frm = 1;
                actStep = S_ERR;  // frame error
                return;
            }
        }
    }
    if (actStep == S_SND) {
        TXREG = nextBit();
    }
}
//----------------------------------------------------------------------

/**
 * Get the next bit to transmit
 * @return - HI_SEND - if bit = 1, otherwise - LO_SEND
 */

inline unsigned char nextBit(void) {
    if (MYCAN.sndByte & MYCAN.posMask)  // check bit 0 or 1
    {
        STATE.sBit = 1;
        return HI_SEND;  //send 1
    } else {
        STATE.sBit = 0;
        return LO_SEND;  //send 0
    }
}

//----------------------------------------------------------------------

void doPack(unsigned char pack[]) {
    unsigned char len, crc;
    crc = (pack[1] & 0x0F) + 2;
    len = crc;
    bufferTx[crc] = 0;
    for (unsigned char i = 0; i < len; i++) {
        bufferTx[i] = pack[i];
        bufferTx[crc] = doCRC(bufferTx[crc], bufferTx[i]);
    }
    doTxD();
}
//----------------------------------------------------------------------

/**
 * We recalculate the CRC with a new byte
 * @param crc - old CRC value
 * @param byte - new byte
 * @return - new CRC value
 */
unsigned char doCRC(unsigned char crc, unsigned char byte) {
    //    unsigned char acc;
    crc = crc ^ byte;
    for (unsigned char i = 0; i < 8; i++) {
        crc = crc & 0x80 ? (unsigned char)(crc << 1) ^ CONST.crcMask : (unsigned char)(crc << 1);
    }
    return crc;
}

//----------------------------------------------------------------------

/**
 * Set the echo mode (set the receive flag after each transmission).
 * @return none
 */
void setEchoTX(void) {
    STATE.echoTX = 1;  // set the echoTX flag
};
//----------------------------------------------------------------------

/**
 * We reset the echo mode.
 * @return none
 */
void clrEchoTX(void) {
    STATE.echoTX = 0;  // reset echoTX flag
};
//----------------------------------------------------------------------

/**
 * Get the receive buffer address.
 * @return  - receive buffer address
 */
inline unsigned char *ptrRxD(void) {
    return &bufferRx[0];  // give the address of the received packet
}
//----------------------------------------------------------------------

/**
 * Get the address of the transmit buffer.
 * @return  - transmit buffer address
 */
inline unsigned char *ptrTxD(void) {
    return &bufferTx[0];  // give the address of the received packet
}
//----------------------------------------------------------------------

/**
 * Checking the package transfer is complete.
 * @return 1 - the transmit buffer is free, 0 - the transmit buffer is busy
 */
inline unsigned char isTxD(void) {  // free transfer buffer
    return (char)STATE.sndRDY;
}
//----------------------------------------------------------------------

/**
 * Allow transmission of a packet from the transmission buffer.
 * @return none
 */

inline void doTxD(void) {  // enable transfer
    di();
    STATE.sndRPT = MAX_REPIT;
    STATE.sndRDY = 0;
    ei();
}
//----------------------------------------------------------------------

/**
 * Checking the completion of packet reception.
 * @return 1 - the next packet is received in the buffer, 0 - the buffer is empty
 */
inline unsigned char isRxD(void) {  // is something recieved
    return (char)STATE.rcvRDY;
}
//----------------------------------------------------------------------

/**
 * Clears the flag of the presence of a received packet in the buffer.
 * @return none
 */
inline void clrRxD(void) {  // buffer is free for next receive
    STATE.rcvRDY = 0;
}
//----------------------------------------------------------------------

/**
 * Returns the error flags of the last bus activity (and clears all flags).
 * @return the value of the error flags with the active flag set (if any)
 */
unsigned char isERR(void) {  //возвращаем флаг ошибки
    unsigned char err;
    err = ERROR.flag;
    ERROR.flag = 0;
    return err;
}

UNS8 canReceive(MESSAGE *m) {
    CANPACK *p;
    unsigned char cnt;
    if (isRxD() == _READY) {
        p = (CANPACK *)ptrRxD();
        m->len = p->len;
        cnt = p->len;
        m->rtr = p->rtr;
        m->cmd = p->cmd;
        m->node = (p->loNode) + (unsigned char)(p->hiNode << 3);
        while (cnt) {
            cnt--;
            m->mdata[cnt] = p->data[cnt];
        }
        clrRxD();
        return _READY;
    }
    return _BUSY;
}

UNS8 canSend(MESSAGE *m) {
    CANPACK *p;
    unsigned char cnt, crc, ptr;

    if (isTxD() == _READY) {
        p = (CANPACK *)ptrTxD();
        p->cmd = m->cmd;
        p->hiNode = m->node >> 3;
        p->loNode = m->node & 0x07;
        p->rtr = m->rtr;
        p->len = m->len;
        cnt = m->len;
        ptr = 0;

        crc = 0;
        crc = doCRC(crc, p->pack[0]);
        crc = doCRC(crc, p->pack[1]);

        while (cnt) {
            cnt--;
            p->data[ptr] = m->mdata[ptr];
            crc = doCRC(crc, m->mdata[ptr]);
            ptr++;
        }
        p->pack[ptr + 2] = crc;
        doTxD();
        return _READY;
    }
    return _BUSY;
};