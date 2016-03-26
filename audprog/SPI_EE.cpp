#include "SPI_EE.h"

void serialize(unsigned char* data, unsigned char* rawData, const unsigned int dataSize) {
	for (unsigned int q = 0; q < dataSize; q++) {
		for (unsigned char i = 0; i < 8; i++) {
			if (*data & (1 << (7 - i))) {
				*rawData = MOSI;
				rawData++;
				*rawData = MOSI | CLK;
				rawData++;
			}
			else {
				*rawData = 0;
				rawData++;
				*rawData = CLK;
				rawData++;
			}
		}
		data++;
	}
}

void deserialize(unsigned char* rawData, unsigned char* data, unsigned int rawDataSize) {
	for (unsigned char q = 0; q < (rawDataSize / 16); q++) {
		*data = 0;
		for (unsigned char i = 0; i < 8; i++) {
			rawData++;
			if (*rawData & MISO) {
				*data |= 1 << (7 - i);
			};
			rawData++;
		}
		data++;
	}
}


void writeEnable(FT_HANDLE ftDevice) {
	unsigned char header[] = { 0x06 };
	DWORD b;
	struct {
		unsigned char csInit;
		unsigned char data[sizeof(header) * 16];
		unsigned char csHigh;
	} rawData;

	rawData.csHigh = CS;
	rawData.csInit = CS;

	serialize(header, rawData.data, sizeof(header));
	FT_Write(ftDevice, &rawData, sizeof(rawData), &b);
	FT_Read(ftDevice, &rawData, sizeof(rawData), &b);
}

void writeDisable(FT_HANDLE ftDevice) {
	unsigned char header[] = { 0x04 };
	DWORD b;
	struct {
		unsigned char csInit;
		unsigned char data[sizeof(header) * 16];
		unsigned char csHigh;
	} rawData;

	rawData.csHigh = CS;
	rawData.csInit = CS;

	serialize(header, rawData.data, sizeof(header));
	FT_Write(ftDevice, &rawData, sizeof(rawData), &b);
	FT_Read(ftDevice, &rawData, sizeof(rawData), &b);
}



void devWriteByte(FT_HANDLE ftDevice, unsigned char wByte, unsigned short address) {
	unsigned char header[] = {0x06};
	unsigned char packet[] = {0x02, (unsigned char)((address >> 8) & 0xFF), (unsigned char)(address & 0xFF), wByte };
	DWORD b = 0;
	struct {
		unsigned char csInit;
		unsigned char header[sizeof(header) * 16];
		unsigned char csHighMid;
		unsigned char data[sizeof(packet) * 16];
		unsigned char csHigh;
	} rawData;

	rawData = { CS, {}, CS, {}, CS};
	serialize(header, rawData.header, sizeof(header));
	serialize(packet, rawData.data, sizeof(packet));
	FT_Write(ftDevice, &rawData, sizeof(rawData), &b);
	FT_Read(ftDevice, &rawData, sizeof(rawData), &b);
	while (getStatus(ftDevice) & 1);
}


unsigned char devReadByte(FT_HANDLE ftDevice, unsigned short address) {
	unsigned char packet[] = { 0x03, (unsigned char)((address >> 8) & 0xFF), (unsigned char)(address & 0xFF), 0x00 };
	DWORD b;
	struct {
		unsigned char csInit;
		unsigned char data[sizeof(packet) * 16];
		unsigned char csHigh;
	} rawData;

	rawData.csInit = CS;
	rawData.csHigh = CS;

	serialize(packet, rawData.data, sizeof(packet));
	FT_Write(ftDevice, &rawData, sizeof(rawData), &b);
	FT_Read(ftDevice, &rawData, sizeof(rawData), &b);
	deserialize(rawData.data, packet, sizeof(rawData));
	return(packet[3]);
}

unsigned char getStatus(FT_HANDLE ftDevice) {
	unsigned char header[] = { 0x05 };
	unsigned char data[] = { 0x00 };
	DWORD b;
	struct {
		unsigned char csInit;
		unsigned char header[sizeof(header) * 16];
		unsigned char data[(sizeof(data)) * 16];
		unsigned char csHigh;
	} rawData;

	rawData = { CS, {}, {}, CS};

	serialize(header, rawData.header, sizeof(header));
	serialize(data, rawData.data, sizeof(data));
	FT_Write(ftDevice, &rawData, sizeof(rawData), &b);
	FT_Read(ftDevice, &rawData, sizeof(rawData), &b);
	unsigned char status = 0;
	deserialize(rawData.data, &status, sizeof(rawData.data));
	return status;
}
