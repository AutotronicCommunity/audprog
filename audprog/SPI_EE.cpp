#include "SPI_EE.h"

void CS_Low(FT_HANDLE ftDevice) {
	unsigned char d = 0;
	DWORD b;
	FT_Write(ftDevice, &d, 1, &b);
	FT_Read(ftDevice, &d, 1, &b);
}

void CS_High(FT_HANDLE ftDevice) {
	unsigned char d = CS;
	DWORD b;
	FT_Write(ftDevice, &d, 1, &b);
	FT_Read(ftDevice, &d, 1, &b);
}

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


unsigned char getStatus(FT_HANDLE ftDevice) {
	unsigned char header[] = { 0x05 };
	unsigned char data[] = { 0x00 };
	DWORD b;
	struct {
		unsigned char header[sizeof(header) * 16];
		unsigned char data[(sizeof(data)) * 16];
	} rawData;

	serialize(header, rawData.header, sizeof(header));
	serialize(data, rawData.data, sizeof(data));
	CS_Low(ftDevice);
	FT_Write(ftDevice, &rawData, sizeof(rawData), &b);
	FT_Read(ftDevice, &rawData, sizeof(rawData), &b);
	CS_High(ftDevice);
	unsigned char status = 0;
	deserialize(rawData.data, &status, sizeof(rawData.data));
	return status;
}

void writeEnable(FT_HANDLE ftDevice) {
	unsigned char header[] = { 0x06 };
	DWORD b;
	unsigned char rawHeader[sizeof(header) * 16];
	serialize(header, rawHeader, sizeof(header));
	CS_Low(ftDevice);
	FT_Write(ftDevice, &rawHeader, sizeof(rawHeader), &b);
	FT_Read(ftDevice, &rawHeader, sizeof(rawHeader), &b);
	CS_High(ftDevice);
}

void writeDisable(FT_HANDLE ftDevice) {
	unsigned char header[] = { 0x04 };
	DWORD b;
	unsigned char rawHeader[sizeof(header) * 16];
	serialize(header, rawHeader, sizeof(header));
	CS_Low(ftDevice);
	FT_Write(ftDevice, &rawHeader, sizeof(rawHeader), &b);
	FT_Read(ftDevice, &rawHeader, sizeof(rawHeader), &b);
	CS_High(ftDevice);
}



void devWriteByte(FT_HANDLE ftDevice, unsigned char wByte, unsigned short address) {
	unsigned char packet[] = { 0x02, (unsigned char)((address >> 8) & 0xFF), (unsigned char)(address & 0xFFUL), wByte };
	DWORD b = 0;
	unsigned char rawData[sizeof(packet) * 16] = {};

	serialize(packet, rawData, sizeof(packet));
	writeEnable(ftDevice);
	CS_Low(ftDevice);
	FT_Write(ftDevice, &rawData, sizeof(rawData), &b);
	FT_Read(ftDevice, &rawData, sizeof(rawData), &b);
	CS_High(ftDevice);
	while (getStatus(ftDevice) & 1);
}


unsigned char devReadByte(FT_HANDLE ftDevice, unsigned short address) {
	unsigned char packet[] = { 0x03, (unsigned char)((address >> 8) & 0xFF), (unsigned char)(address & 0xFF), 0x00 };
	unsigned char rawData[sizeof(packet) * 16] = {};
	DWORD b;
	serialize(packet, rawData, sizeof(packet));
	CS_Low(ftDevice);
	FT_Write(ftDevice, &rawData, sizeof(rawData), &b);
	FT_Read(ftDevice, &rawData, sizeof(rawData), &b);
	CS_High(ftDevice);
	deserialize(rawData, packet, sizeof(rawData));
	return(packet[3]);
}



