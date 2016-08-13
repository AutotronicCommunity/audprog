#ifndef __AUDPROG_H_
#define __AUDPROG_H_

#include "stdafx.h"
#include "ftd2xx.h"
#include "AUD.h"
#include "SPI_EE.h"

enum DEVICE_IFACE {AUD, EEPROM, DEVICE_IFACE_NONE};
enum AUD_MODE {AUD_BYTE, AUD_WORD, AUD_LONGWORD};
enum DEVICE_ACTION {READ, WRITE, PRINT, DUMP, MONITOR, DEVICE_ACTION_NONE};
enum ROM_SIZE {ROM_512, ROM_1024, EE_32, EE_64};

struct PARAMS {
	int ftDeviceNum = 0;
	FT_HANDLE ftDevice = NULL;
	unsigned long offset = 0;
	unsigned long len = 1;
	wchar_t * fileName = NULL;
	DEVICE_IFACE iface = DEVICE_IFACE_NONE;
	DEVICE_ACTION action = DEVICE_ACTION_NONE;
	AUD_MODE mode = AUD_LONGWORD;
	ROM_SIZE romSize = ROM_512;
};

//main functions prototypes
// AUD
int AUD_uploadFile(PARAMS* params);
int AUD_readToFile(PARAMS* params);
int AUD_readToScreen(PARAMS* params);
int AUD_dumpToFile(PARAMS* params);

//EEPROM
int EE_readToFile(PARAMS* params);
int EE_writeFile(PARAMS* params);

#endif