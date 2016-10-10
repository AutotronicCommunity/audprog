#ifndef __AUD_H_
#define __AUD_H_

#include "stdafx.h"
#include "ftd2xx.h"

// AUD pin definition


// NEW HW

#define _AUD_RST	(1<<4)
#define _AUD_SYNC	(1<<3)

#define AUD_CK		(1<<6)
#define AUD_D0		(1<<2)
#define AUD_D1		(1<<1)
#define AUD_D2		(1<<7)
#define AUD_D3		(1<<5)

#define AUD_DMASK (AUD_D0 | AUD_D1 | AUD_D2 | AUD_D3)


// OLD HW

/*
#define _AUD_RST	(1<<0)
#define _AUD_SYNC	(1<<1)

#define AUD_CK		(1<<3)
#define AUD_D0		(1<<4)
#define AUD_D1		(1<<6)
#define AUD_D2		(1<<7)
#define AUD_D3	(1<<5)

*/

// AUD routines prototypes
unsigned long AUD_readLWord(FT_HANDLE ftDevice, unsigned long a);
unsigned char AUD_readByte(FT_HANDLE ftDevice, unsigned long a);
unsigned short AUD_readWord(FT_HANDLE ftDevice, unsigned long a);

unsigned char AUD_writeByte(FT_HANDLE ftDevice, unsigned long a, unsigned char bData);
unsigned char AUD_writeWord(FT_HANDLE ftDevice, unsigned long a, unsigned int wData);
unsigned char AUD_writeLWord(FT_HANDLE ftDevice, unsigned long a, unsigned long lData);

unsigned char AUD_bitSwap(unsigned char in);
unsigned char AUD_bitUnSwap(unsigned char in);
unsigned char * AUD_cDataFill(unsigned char * cData, unsigned int cDataSize, unsigned char cmd, unsigned long addr);
unsigned char * AUD_cwDataFill(unsigned char * cData, unsigned int cDataSize, unsigned char cmd, unsigned long addr, unsigned long data);
unsigned char * AUD_rDataFill(unsigned char * rData, unsigned int rDataSize);
unsigned char * AUD_wDataFill(unsigned char * wData, unsigned int wDataSize, unsigned long data);

void AUD_Poll(FT_HANDLE ftDevice);
void AUD_SHSetRAMmode(FT_HANDLE ftDevice);
void AUD_wTick(FT_HANDLE ftDevice);

#endif