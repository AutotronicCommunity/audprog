#include "audprog.h"

unsigned long AUD_readLWord(FT_HANDLE ftDevice, unsigned long a) {
	unsigned long addr = a;
	unsigned long lData = 0;
	unsigned long b = 0;

	unsigned char cData[5 * 2 * 2 + 2] = {};
	unsigned char rData[4 * 2 * 3 + 3] = {};

	// filling up command sequense
	AUD_cDataFill(cData, sizeof(cData), 0xA0, a);

	//filling up read sequense
	AUD_rDataFill(rData, sizeof(rData));

	//send command DWORD read
	FT_SetBitMode(ftDevice, 0xFF, FT_BITMODE_SYNC_BITBANG); // all pins are outputs
	FT_Write(ftDevice, cData, sizeof(cData), &b);
	FT_Read(ftDevice, cData, sizeof(cData), &b);

	AUD_Poll(ftDevice); // polling for READY state

	//reading DWORD
	//	FT_SetBitMode(ftDevice, 0x0F, FT_BITMODE_SYNC_BITBANG);
	FT_Write(ftDevice, rData, sizeof(rData), &b);
	FT_Read(ftDevice, rData, sizeof(rData), &b);
	for (int q = 3; q < sizeof(rData); q += 3) {
		lData <<= 4;
		lData |= 0x0F & AUD_bitUnSwap(rData[q] & AUD_DMASK);
	}
	return ((lData & 0x0F0F0F0F) << 4) | ((lData & 0xF0F0F0F0) >> 4);
}

unsigned char AUD_readByte(FT_HANDLE ftDevice, unsigned long a) {
	unsigned long addr = a;
	unsigned long bData = 0;
	unsigned long b = 0;

	unsigned char cData[5 * 2 * 2 + 2] = {};
	unsigned char rData[1 * 2 * 3 + 3] = {};

	// filling up command sequense
	AUD_cDataFill(cData, sizeof(cData), 0x80, a);

	//filling up read sequense
	AUD_rDataFill(rData, sizeof(rData));

	//send command BYTE read
	FT_SetBitMode(ftDevice, 0xFF, FT_BITMODE_SYNC_BITBANG); // all pins are outputs
	FT_Write(ftDevice, cData, sizeof(cData), &b);
	FT_Read(ftDevice, cData, sizeof(cData), &b);

	AUD_Poll(ftDevice); // polling for READY state

	//reading BYTE
	FT_Write(ftDevice, rData, sizeof(rData), &b);
	FT_Read(ftDevice, rData, sizeof(rData), &b);

	for (int q = 3; q < sizeof(rData); q += 3) {
		bData <<= 4;
		bData |= 0x0F & AUD_bitUnSwap(rData[q] & AUD_DMASK);
	}
	return ((bData & 0x0F) << 4) | ((bData & 0xF0) >> 4);
}

unsigned short AUD_readWord(FT_HANDLE ftDevice, unsigned long a) {
	unsigned long addr = a;
	unsigned short wData = 0;
	unsigned long b = 0;

	unsigned char cData[5 * 2 * 2 + 2] = {};
	unsigned char rData[2 * 2 * 3 + 3] = {};

	// filling up command sequense
	AUD_cDataFill(cData, sizeof(cData), 0x90, a);

	//filling up read sequense
	AUD_rDataFill(rData, sizeof(rData));

	//send command WORD read
	FT_SetBitMode(ftDevice, 0xFF, FT_BITMODE_SYNC_BITBANG); // all pins are outputs
	FT_Write(ftDevice, cData, sizeof(cData), &b);
	FT_Read(ftDevice, cData, sizeof(cData), &b);

	AUD_Poll(ftDevice); // polling for READY state

	//reading BYTE
	FT_Write(ftDevice, rData, sizeof(rData), &b);
	FT_Read(ftDevice, rData, sizeof(rData), &b);

	for (int q = 3; q < sizeof(rData); q += 3) {
		wData <<= 4;
		wData |= 0x0F & AUD_bitUnSwap(rData[q] & (AUD_DMASK));
	}
	return ((wData & 0x0F0F) << 4) | ((wData & 0xF0F0) >> 4);
}

unsigned char AUD_writeByte(FT_HANDLE ftDevice, unsigned long a, unsigned char bData) {
	
	unsigned long b = 0;
	unsigned char cwData[5 * 2 * 2 + 2 + 1 * 2 * 2] = {};

	// filling up command sequense
	AUD_cwDataFill(cwData, sizeof(cwData), 0xC0, a, bData);

	//BYTE write
	FT_SetBitMode(ftDevice, 0xFF, FT_BITMODE_SYNC_BITBANG); // all pins are outputs
	FT_Write(ftDevice, cwData, sizeof(cwData), &b);
	FT_Read(ftDevice, cwData, sizeof(cwData), &b);

	AUD_Poll(ftDevice); // polling for READY state
	AUD_wTick(ftDevice);
	return 0;
}

unsigned char AUD_writeWord(FT_HANDLE ftDevice, unsigned long a, unsigned int wData) {
	unsigned long b = 0;
	unsigned char cwData[5 * 2 * 2 + 2 + 2 * 2 * 2] = {};

	// filling up command sequense
	AUD_cwDataFill(cwData, sizeof(cwData), 0xD0, a, wData);

	//WORD write
	FT_SetBitMode(ftDevice, 0xFF, FT_BITMODE_SYNC_BITBANG); // all pins are outputs
	FT_Write(ftDevice, cwData, sizeof(cwData), &b);
	FT_Read(ftDevice, cwData, sizeof(cwData), &b);

	AUD_Poll(ftDevice); // polling for READY state
	AUD_wTick(ftDevice);
	return 0;
}

unsigned char AUD_writeLWord(FT_HANDLE ftDevice, unsigned long a, unsigned long lData) {
	unsigned long b = 0;
	unsigned char cwData[5 * 2 * 2 + 2 + 4 * 2 * 2] = {};

	// filling up command sequense
	AUD_cwDataFill(cwData, sizeof(cwData), 0xE0, a, lData);

	//DWORD write
	FT_SetBitMode(ftDevice, 0xFF, FT_BITMODE_SYNC_BITBANG); // all pins are outputs
	FT_Write(ftDevice, cwData, sizeof(cwData), &b);
	FT_Read(ftDevice, cwData, sizeof(cwData), &b);

	AUD_Poll(ftDevice); // polling for READY state
	AUD_wTick(ftDevice);
	return 0;
}

void AUD_wTick(FT_HANDLE ftDevice) {
	unsigned long b = 0;
	unsigned char tData[] = {
		_AUD_RST | _AUD_SYNC,
		_AUD_RST | _AUD_SYNC | AUD_CK,
		_AUD_RST | _AUD_SYNC,
		_AUD_RST | _AUD_SYNC | AUD_CK
	};

	FT_SetBitMode(ftDevice, 0xFF, FT_BITMODE_SYNC_BITBANG); // all pins outputs
	FT_Write(ftDevice, tData, sizeof(tData), &b);
	FT_Read(ftDevice, tData, sizeof(tData), &b);
}

void AUD_Poll(FT_HANDLE ftDevice) { 	// pooling for READY
	unsigned long b = 0;
	unsigned char pollStatus = 0;
	int a = 0;
	unsigned char pollData[] = {
		_AUD_RST,
		_AUD_RST | AUD_CK,
		_AUD_RST,
		_AUD_RST | AUD_CK,
		_AUD_RST,
		_AUD_RST | AUD_CK,
		_AUD_RST,
		_AUD_RST | AUD_CK,
		_AUD_RST,
		_AUD_RST | AUD_CK,
		_AUD_RST,
		_AUD_RST | AUD_CK
	};

	FT_SetBitMode(ftDevice, 0xff & (~AUD_DMASK), FT_BITMODE_SYNC_BITBANG); // AUD_D0..3 are inputs
	unsigned char r = 1;
	while (r) {
		FT_Write(ftDevice, pollData, sizeof(pollData), &b);
		FT_Read(ftDevice, pollData, sizeof(pollData), &b);
		for (int q = 0; q < sizeof(pollData); q++) {
			pollStatus = pollData[q] & AUD_DMASK;
//			Sleep(200);
//			printf("pool status 0x%02x\n", pollStatus);
			if (pollStatus == AUD_D0) {
				r = 0;
				break;
			}
			if (pollStatus == (AUD_D0 | AUD_D2)) {
				printf("Bus error!\n");
				r = 0;
				break;
			}
			if (pollStatus == (AUD_D0 | AUD_D1)) {
				printf("Command error!\n");
				r = 0;
				break;
			}
		}
		a++;
		if (a > 50) {
			printf_s("Uknown AUD bus error! Status 0x%02x Please on/off the ECU.                                           \n", pollStatus);
			break;
		}
	}
}

void AUD_SHSetRAMmode(FT_HANDLE ftDevice) {
	unsigned long b = 0;
	unsigned char pData[] = {
		_AUD_SYNC | _AUD_RST | AUD_CK,
		_AUD_SYNC,
		_AUD_SYNC | AUD_CK,
		_AUD_SYNC,
		_AUD_SYNC | AUD_CK,
		_AUD_SYNC,
		_AUD_SYNC | AUD_CK,
		_AUD_SYNC,
		_AUD_SYNC | AUD_CK,
		_AUD_SYNC,
		_AUD_SYNC | AUD_CK,
		_AUD_SYNC,
		_AUD_SYNC | _AUD_RST,
		_AUD_SYNC | _AUD_RST | AUD_CK,
		_AUD_SYNC | _AUD_RST,
		_AUD_SYNC | _AUD_RST | AUD_CK
	};
	FT_SetBitMode(ftDevice, 0xFF, FT_BITMODE_SYNC_BITBANG); // all pins are outputs
	FT_Write(ftDevice, pData, sizeof(pData), &b);
	FT_Read(ftDevice, pData, sizeof(pData), &b);
}


unsigned char * AUD_cDataFill(unsigned char * cData, unsigned int cDataSize, unsigned char cmd, unsigned long addr) {
	// filling up command sequense 
	cData[0] = _AUD_RST | _AUD_SYNC;
	cData[1] = _AUD_RST | _AUD_SYNC | AUD_CK;
	for (int q = 2; q < 6; q++) {
		cData[q] = _AUD_RST | AUD_bitSwap(cmd & 0x0f);
		q++;
		cData[q] = _AUD_RST | AUD_CK | AUD_bitSwap(cmd & 0x0f);
		cmd >>= 4;
	}
	for (unsigned int q = 6; q < cDataSize - 2; q++) {
		cData[q] = _AUD_RST | AUD_bitSwap(addr & 0x0f);
		q++;
		cData[q] = _AUD_RST | AUD_CK | AUD_bitSwap(addr & 0x0f);
		addr >>= 4;
	};
	cData[cDataSize - 2] = cData[cDataSize - 4]; // additional clock after CMD
	cData[cDataSize - 1] = cData[cDataSize - 3];

	return cData;
}

unsigned char * AUD_cwDataFill(unsigned char * cwData, unsigned int cwDataSize, unsigned char cmd, unsigned long addr, unsigned long data) {
	// filling up command sequense 
	cwData[0] = _AUD_RST | _AUD_SYNC;
	cwData[1] = _AUD_RST | _AUD_SYNC | AUD_CK;

	for (int q = 2; q < 2*2*2 + 2; q++) {
		cwData[q] = _AUD_RST | AUD_bitSwap(cmd & 0x0f);
		q++;
		cwData[q] = _AUD_RST | AUD_CK | AUD_bitSwap(cmd & 0x0f);
		cmd >>= 4;
	}

	for (unsigned int q = 6; q < (4*2*2) + 6; q++) {
		cwData[q] = _AUD_RST | AUD_bitSwap(addr & 0x0f);
		q++;
		cwData[q] = _AUD_RST | AUD_CK | AUD_bitSwap(addr & 0x0f);
		addr >>= 4;
	};

	for (unsigned int q = 22; q < cwDataSize; q++) {
		cwData[q] = _AUD_RST | AUD_bitSwap((unsigned char)data & 0x0f);
		q++;
		cwData[q] = _AUD_RST | AUD_CK | AUD_bitSwap((unsigned char)data & 0x0f);
		data >>= 4;
	};

	return cwData;
}

unsigned char * AUD_wDataFill(unsigned char * wData, unsigned int wDataSize, unsigned long data) {
	for (unsigned int q = 0; q < wDataSize; q++) {
		wData[q] = _AUD_RST | AUD_bitSwap(data & 0x0F);
		q++;
		wData[q] = _AUD_RST | AUD_CK | AUD_bitSwap(data & 0x0F);
		data >>= 4;
	};
	return wData;
}

unsigned char * AUD_rDataFill(unsigned char * rData, unsigned int rDataSize) {
	for (unsigned int q = 0; q < rDataSize; q++) {
		rData[q] = _AUD_RST | _AUD_SYNC;
		q++;
		rData[q] = _AUD_RST | _AUD_SYNC | AUD_CK;
		q++;
		rData[q] = _AUD_RST | _AUD_SYNC | AUD_CK;
	};
	return rData;
}

unsigned char AUD_bitSwap(unsigned char in) {
	unsigned char out = 0;
	if (in & (1 << 0)) out |= AUD_D0;
	if (in & (1 << 1)) out |= AUD_D1;
	if (in & (1 << 2)) out |= AUD_D2;
	if (in & (1 << 3)) out |= AUD_D3;
	return out;
}

unsigned char AUD_bitUnSwap(unsigned char in) {
	unsigned char out = 0;
	if (in & AUD_D0) out |= (1 << 0);
	if (in & AUD_D1) out |= (1 << 1);
	if (in & AUD_D2) out |= (1 << 2);
	if (in & AUD_D3) out |= (1 << 3);
	return out;
}