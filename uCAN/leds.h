#ifndef LEDS_H
#define	LEDS_H

#include <xc.h> // include processor files - each processor file is guarded.  

void led0ON(void);
void led1ON(void);
void led2ON(void);
void led3ON(void);
void led3TG(void);
void allOFF(void);
void stateLED(unsigned char state);


void ledTxEoff(void);
void ledTxEon(void);
void ledRxEoff(void);
void ledRxEon(void);
void ledBeat(void);

#endif	/* LEDS_H */

