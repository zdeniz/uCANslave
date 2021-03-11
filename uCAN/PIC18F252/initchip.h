
#ifndef INITCHIP_H
#define	INITCHIP_H

#include <xc.h> // include processor files - each processor file is guarded.  

void initChip(void);
void initCPU(void);

void initPORTA(void);
void initPORTB(void);
void initPORTC(void);

void initUART(unsigned char num);

void initTMR0(void);
void initWDT(void);

void initINT(void);

#endif	/* INITCHIP_H */

