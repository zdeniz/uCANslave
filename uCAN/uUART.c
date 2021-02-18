
//PAPA
#include "init252.h"
#include "uUART.h"
#include "data.h"
#include "leds.h"
//------------------------------------------------------------------------------
/**
 * \file
 * \brief Драйвер rsCAN и API вызовы.
 * 
 * Таймер
 *  межпакетного интервала 100мкс, счетчик интервала 1мс, счетчик 1с интервала.
 * Приемник
 * UART с приемом и передачей пакетов myCAN
 * Контроль
 * активности шины INT0 прерываний
 *
 */
//------------------------------------------------------------------------------
extern CO_Data uData;
extern const unsigned char divConst[8];

unsigned int systemTick; ///<системное время 

static volatile unsigned char divCnt; ///<счетчик множителя межпакетного интервала
static volatile unsigned char gapCnt; ///<счетчик межпакетного интервала
static volatile unsigned char actStep; ///<активное состояние (машина состояний)

static volatile BYTESTATE STATE; ///<биты состояния и управления
static volatile BYTEERROR ERROR; ///<биты состояния и управления


volatile unsigned char countTick; ///<счетчик тайм слотов CAN - 1мс


static unsigned char bufferTx[12]; ///<буфер передачи
static unsigned char bufferRx[12]; ///<буфер приема

typedef struct {
    unsigned char gapTime; ///<величина защитного интервала в 100мкс интервалах
    unsigned char gapMult; ///<множитель защитного интервала в 100мкс интервалах
    unsigned char rcvMask; ///<маска выбоки бита 
    unsigned char crcMask; ///<CRC полином
} tConst;
tConst CONST;

struct {
    //пакетные счетчики бит, байт, контроля размера пакета
    unsigned char ptrPack; ///<номер принятого бита
    unsigned char lenPack; ///<длина принимаемого пакета в байтах
    unsigned char cntPack; ///<счетчик байт в пакете
    unsigned char lenData; ///<длина блока данных в байтах
    //счетчики и переменные битового приема/передачи байта
    unsigned char rBit; ///<принятый бит
    unsigned char sBit; ///<переданный бит
    unsigned char nBit; ///<номер бита в текущем байте
    unsigned char posMask; ///<маска позиции бита в байте
    //переменные приема/передачи байта  
    unsigned char tmpByte; ///<рабочая переменная
    unsigned char firstTX; ///<первая посылка пакета (первый бит)
    unsigned char sndByte; ///<передаваемый байт
    unsigned char rcvByte; ///<принимаемый байт

    unsigned char crcByte; ///<байт контрольной суммы
} MYCAN;
//----------------------------------------------------------------------
//----------------------------------------------------------------------

/**
 * Инициализация драйвера rsCAN
 * 0-115; 1-57.6; 2-38.4; 3-19.2; 4-9.6; 5-4.8; 6-2.4; 7-1.2
 * @param num - номер параметра скорости
 */
unsigned char initCAN(unsigned char num) {
    loadEngine();
    resetEngine();
    setSpeed(num);
    setMult(num);
    initUART();
    initTMR0();
    return 1;
}
//----------------------------------------------------------------------

inline void setMult(unsigned char num) {
    CONST.gapMult = divConst[num];
}

void loadEngine(void) {
    CONST.gapTime = GAP_TIME; //интервал GAP
    CONST.rcvMask = RCV_MASK; //маска бита выборки данных
    CONST.crcMask = CRC_MASK; //полином CRC
}
//----------------------------------------------------------------------
//----------------------------------------------------------------------

void resetEngine(void) {
    actStep = S_IDL;
    MYCAN.crcByte = 0;
    MYCAN.cntPack = 0;
    MYCAN.ptrPack = 0;
    MYCAN.lenPack = LEN_PACK;
    MYCAN.nBit = 8;
    MYCAN.posMask = 0x80;
    gapCnt = CONST.gapTime;
}
//----------------------------------------------------------------------

//----------------------------------------------------------------------

/**
 * Обработчик прерываний
 * 
 * Прерывания таймера с интервалом 100 мкс
 * Прерывания INT0 обнаружения начала приема
 * Прерывания UART
 * Прерывания USB
 */
void __interrupt(high_priority) hiINT(void) {
    if (INTCONbits.TMR0IF && INTCONbits.TMR0IE) {
        INTCONbits.TMR0IF = 0;
        TMR0H = 0xFF;
        TMR0L = 0xA9;
        if (countTick) {
            countTick--; //считаем с шагом 100мкс (шагом GAP) до 10 == 1мс
        } else {
            countTick = TICK_TIME; //считаем до 1мс (TICK_TIME = 10)
            uData.canFlags.sysTickFlag = 1; // взводим флаг 1мс
            systemTick++; //считаем системное время в миллисекундах
        }
        divCnt--;
        if (!divCnt) {
            divCnt = CONST.gapMult;
            handlerTMR0(); //каждый интервал GAP = 100мс*gap.Mult вызываем обработчик
        }
    }
    if (INTCONbits.INT0E && INTCONbits.INT0F) {
        INTCONbits.INT0E = 0; //запрещаем прерывание от INT0
        INTCONbits.INT0F = 0;
        actStep = S_RCV;
    }
    if (PIE1bits.RCIE && PIR1bits.RCIF) {
        handlerRXD();
    }
    /*
    if (USBIF && USBIE) {
        USBDeviceTasks();
    }
     */
}
//----------------------------------------------------------------------

/**
 * Обработчик защитных межпакетных разделительных интервалов
 *
 * На предпоследнем n-1 отсчете интервала после активности инициализируем драйвер
 * На  последнем запускаем передачу, если есть готовый пакет
 * Для максимального использования канала рекомендуется 3-2 отсчета GAP.
 */
void handlerTMR0(void) {
    if (gapCnt > 0) {
        gapCnt--; //уменишем счетчик GAP
        if (gapCnt == 0) { //в последний интервал GAP
            if (actStep != S_IDL) { //если была активность
                ERROR.act = 1;
                resetEngine(); //инициализируем счетчики приема/передачи
                //               INTCONbits.INT0IF = 0;		// разрешаем прием
                //               INTCONbits.INT0IE = 1;		// ждем
            }
        }
    } else {
        nextSend();
    }
}
//----------------------------------------------------------------------

/**
 * Обработчик приема пакета данных
 *
 * прием бита, контроль UART
 * котроль приоритета(0 по 12 бит) , переход к приему приоритетного пакета
 * определение длинны пакета (после приема 15-го бита)
 * контроль переполнения буфера приема ( перед записью первого принятого байта)
 * Контроль CRC
 * Контроль фактической длины пакета
 * Выставление флагов завершения приема/передачи
 */

inline void handlerRXD(void) {
    gapCnt = CONST.gapTime;
    //-----------------прием бита-------------------------
    MYCAN.tmpByte = RCREG; //с приемником все нормально читаем байт

    //-----------------контроль приемника------------------------    
    if (RCSTAbits.FERR || RCSTAbits.OERR) //Были ошибки?
    {
        //В случае ошибки приемника
        RCSTAbits.CREN = 0; //Отключаем приемник для сброса ошибки
        NOP();
        //Включаем приемник для продолжения работу УАРТ
        RCSTAbits.CREN = 1;
        ERROR.drv = 1;
        actStep = S_ERR;
        return;
    }
    //-----состояние ERR, приняли бит и на выход, ждем конца приема по GAP 
    if (actStep == S_ERR) {
        return;
    }
    //---------далее в рабочем режиме RCV или SND--------
    if (MYCAN.tmpByte & CONST.rcvMask) { // что приняли ?
        MYCAN.rBit = 1; // единица
        MYCAN.rcvByte = MYCAN.rcvByte | MYCAN.posMask; //фиксируем 1 в байт
    } else {
        MYCAN.rBit = 0; //ноль         
        MYCAN.rcvByte = MYCAN.rcvByte & (~MYCAN.posMask); //фиксируем 0 в байт
    }
    //-------- бит принят, добавляем его в контрольную сумму пакета-------
    MYCAN.crcByte = MYCAN.crcByte & 0x80 ? (((MYCAN.crcByte << 1) & MYCAN.rBit) ^ CONST.crcMask) : ((MYCAN.crcByte << 1) & MYCAN.rBit);

    //-----коптроль коллизии передачи, есть ли приоритетный бит?-------
    if (actStep == S_SND) {
        if (MYCAN.rBit != MYCAN.sBit) { //совпали биты
            if ((MYCAN.ptrPack < 12)&&(MYCAN.rBit == 0)) {
                //нет!допустимо для первых 12 бит - принят доминантный бит?
                // да! это бит приоритета/адреса
                //бит вставляем в пакет и переходим в режим приема RCV
                actStep = S_RCV;
            } else {
                // нет! ошибка шины
                ERROR.bus = 1;
                actStep = S_ERR;
                return;
            }
        }
        // на шине данные актуальны режим передачи SND            
    }

    //        TXREG = 0xFF;
    //        return;
    if (MYCAN.ptrPack == 15) { //принят последний бит заголовка
        // взять длину пакета из принятого байта
        MYCAN.lenData = MYCAN.rcvByte & 0x0F;
        MYCAN.lenPack = MYCAN.lenPack + MYCAN.lenData;
    }
    // переходим к приему следующего бита
    MYCAN.posMask = MYCAN.posMask >> 1;
    MYCAN.nBit--;
    MYCAN.ptrPack++;

    if (MYCAN.nBit == 0) { //все биты байта приняли/передали?
        MYCAN.nBit = 8; //ДА! инициируем прием/передачу следующего байта
        MYCAN.posMask = 128;
        //сохраняем принятый байт в буфер приема и берем следующий из буфере передачи

        if (STATE.rcvRDY) { //контроль переполнения буфера приема
            ERROR.ovr = 1; //буфер приема не очищен
        }
        if (MYCAN.cntPack < MYCAN.lenPack) { //пакет принят?
            bufferRx[MYCAN.cntPack] = MYCAN.rcvByte; // нет, принимаем еще
        } else { //завершение приема пакета? прием CRC 
            if (MYCAN.cntPack == MYCAN.lenPack) { //принята CRC
                //да, пакет принят.
                if (MYCAN.crcByte == 0) {
                    //В контрольной сумме ноль-все ОК!
                    if (actStep == S_RCV) { //завершен прием
echoExit:
                        //                        ERROR.ovr = 0;
                        STATE.rcvRDY = 1; //  без ошибок, пакет принят
                        return;
                    }
                    if (actStep == S_SND) { //завершена передача
                        STATE.sndRDY = 1;
                        //                        STATE.sndERR = 0; //  без ошибок, пакет отправлен
                        if (STATE.echoTX) {
                            goto echoExit;
                        }
                        return;
                    }

                } else {
                    //банальная ошибка CRC
                    ERROR.crc = 1;
                    actStep = S_ERR; //ошибка CRC
                    return;
                }
            } else {
                //ошибка фрейма
                ERROR.frm = 1;
                actStep = S_ERR; //ошибка фрейма
                return;
            }
        }

        MYCAN.cntPack++; //  следующий байт 

        if (actStep == S_SND) { //  передаем ? 
            if (MYCAN.cntPack <= MYCAN.lenPack) { // конец передачи?
                MYCAN.sndByte = bufferTx[MYCAN.cntPack]; //нет, передаем еще
            } else {
                NOP();
                //быть не должно, завершение по приему последнего байта(CRC)
                //ошибка фрейма
                ERROR.frm = 1;
                actStep = S_ERR; //ошибка фрейма
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
 * Получить следующий бит для передачи
 * @return - HI_SEND - если бит = 1, иначе - LO_SEND
 */

unsigned char nextBit(void) {
    if (MYCAN.sndByte & MYCAN.posMask) // check bit 0 or 1
    {
        MYCAN.sBit = 1;
        return HI_SEND; //send 1
    } else {
        MYCAN.sBit = 0;
        return LO_SEND; //send 0
    }
}
//----------------------------------------------------------------------

/**
 * Запускаем передачу пакета
 * 
 * Если передача не завершена и есть еще неисчерпанные повторы, то
 * - запрещаем переход в режим приема,
 * - данные в регистр передачи
 * - далее передача в режиме прерывание по контролю приема
 * иначе
 * -устанавливаем флаг ошибки
 * -устанавливаем флаг завершения передачи
 * 
 * * @return none
 */

void nextSend(void) {
    if (STATE.sndRDY) { //установлен флаг завершения передачи
        return; //ничего не делаем
    }
    if (STATE.sndRPT) //исчерпано кол-во повторов?
    {
        STATE.sndRPT--; // нет 
        MYCAN.sndByte = bufferTx[0];
        //        MYCAN.firstTX = nextBit();
        //        TXREG = MYCAN.firstTX;
        TXREG = nextBit();
        gapCnt = CONST.gapTime;
        //        INTCONbits.INT0IE = 0; //запрещаем
        //        INTCONbits.INT0IF = 0; // сбрасываем прерывание приема
        actStep = S_SND;
        ERROR.flag = 0;
        return;
    }
    ERROR.rpt = 1; //исчерпали количество повторов
    STATE.sndRDY = 1; //конец передачам
}
//----------------------------------------------------------------------
//----------------------------------------------------------------------

void doPack(unsigned char pack[]) {
    unsigned char len, crc;
    crc = (pack[1]&0x0F) + 2;
    len = crc;
    bufferTx[crc] = 0;
    for (unsigned char i = 0; i < len; i++) {

        bufferTx[i] = pack[i];
        bufferTx[crc] = doCRC(bufferTx[crc], bufferTx[i]);
    }
    STATE.sndRPT = MAX_REPIT;
    STATE.sndRDY = 0;
}
//----------------------------------------------------------------------
//----------------------------------------------------------------------
/*
void doSend(unsigned char cmd, unsigned char adr, unsigned char rtr, unsigned char len, unsigned char data[]) {
    unsigned char i, j, posCRC;
    j = 0;
    posCRC = len + 2;
    bufferTx[posCRC] = 0;
    bufferTx[0] = 0x0F & (adr >> 4);
    bufferTx[0] = bufferTx[0] | (0xF0 & cmd << 4);
    bufferTx[posCRC] = doCRC(bufferTx[posCRC], bufferTx[0]);
    bufferTx[1] = len & 0x0F;
    bufferTx[1] = bufferTx[1] | (0xF0 & adr << 4);
    bufferTx[posCRC] = doCRC(bufferTx[posCRC], bufferTx[1]);
    for (i = 2; i < posCRC; i++) {

        bufferTx[i] = data[j];
        j++;
        bufferTx[posCRC] = doCRC(bufferTx[posCRC], bufferTx[i]);
    }
    STATE.sndRPT = MAX_REPIT;
    STATE.sndRDY = 0;
}
 */
//----------------------------------------------------------------------

/**
 * Пересчитываем CRC с новым байтом
 * @param crc - старое значение CRC
 * @param byte - новый байт
 * @return - новое значение CRC
 */
unsigned char doCRC(unsigned char crc, unsigned char byte) {
    unsigned char acc;
    crc = crc ^ byte;
    for (unsigned char i = 0; i < 8; i++) {

        crc = crc & 0x80 ? (crc << 1) ^ CONST.crcMask : crc << 1;
    }
    return crc;
}

//----------------------------------------------------------------------

/**
 * Установить режим эха (устанавливаем флаг приема после каждой своей передачи).
 * @return none
 */
void setEchoTX(void) {
    STATE.echoTX = 1; //установка флага echoTX
};
//----------------------------------------------------------------------

/**
 * Сбрасываем режим эха.
 * @return none
 */
void clrEchoTX(void) {
    STATE.echoTX = 0; //сброс флага echoTX
};
//----------------------------------------------------------------------

/**
 * Получить адрес буфера приема.
 * @return  - адрес буфера приема
 */
unsigned char *ptrRxD(void) {
    return &bufferRx[0]; //отдаем адрес принятого пакета
}
//----------------------------------------------------------------------

/**
 * Получить адрес буфера передачи.
 * @return  - адрес буфера передачи
 */
unsigned char *ptrTxD(void) {
    return &bufferTx[0]; //отдаем адрес принятого пакета
}
//----------------------------------------------------------------------

/**
 * Проверка завершения передачи пакета.
 * @return 1 - буфер передачи свободен, 0 - буфер передачи занят 
 */
unsigned char isTxD(void) { //свободен буфер передачи
    return (char) STATE.sndRDY;
}
//----------------------------------------------------------------------

/**
 * Разрешаем передачу пакета из буфера передачи.
 * @return none
 */

void doTxD(void) { //разрешаем передачу
    di();
    STATE.sndRPT = MAX_REPIT;
    STATE.sndRDY = 0;
    ei();
}
//----------------------------------------------------------------------

/**
 * Проверка завершения передачи пакета.&
 * Разрешаем передачу пакета из буфера передачи.
 * @return none
 */
unsigned char sendAct(void) {
    if (isTxD()) {
        doTxD();
        ledTxEoff();
        return 0;
    } else {
        ledTxEon();
        return 1;
    }
};

//----------------------------------------------------------------------

/**
 * Проверка завершения приема пакета.
 * @return 1 - принят в буфер очередной пакет, 0 - буфер пуст 
 */
unsigned char isRxD(void) { //принято что либо
    return (char) STATE.rcvRDY;
}
//----------------------------------------------------------------------

/**
 * Очиска флага наличия принятого пакета в буфере.
 * @return none
 */
void clrRxD(void) { //буферр свободен для следующего приема
    STATE.rcvRDY = 0;
}
//----------------------------------------------------------------------

/**
 * Возвращает флаги ошибок последней активности шины (и очищает все флаги).
 * @return значение флагов ошибок с установленным флагом активности(если была)
 */
unsigned char isERR(void) { //возвращаем флаг ошибки
    unsigned char err;
    err = ERROR.flag;
    ERROR.flag = 0;
    return err;
}
UNS8 canReceive(MESSAGE *m) {
    CANPACK *p;
    unsigned char cnt;
    if (isRxD()==_READY) {
        p = (CANPACK *) ptrRxD();
        m->len = p->len;
        m->rtr = p->rtr;
        m->cmd = p->cmd;
        m->node = (p->loNode) + (p->hiNode << 3);
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
    unsigned char cnt,crc,ptr;
    NOP();
    if (isTxD()==_READY) {
        p = (CANPACK *) ptrTxD();

        crc = 0;
    
        p->cmd = m->cmd;
        p->hiNode = m->node >> 3;
        p->loNode = m->node & 0x07;
        crc = doCRC(crc, p->pack[0]);
        p->rtr = m->rtr;
        p->len = m->len;
        cnt = m->len;
        crc = doCRC(crc, p->pack[1]);
        ptr = 0;
        while (cnt) {
            cnt--;
            p->data[ptr] = m->mdata[ptr];
            crc = doCRC(crc, p->data[ptr]);
            ptr++;
        }
                     p->pack[ptr+2] = crc;

        return _READY;
    }
    return _BUSY;
    
};