#include "audprog.h"

#define exit()  printf_s("\nPress any key...");\
				getchar();\
			    return 0\

void printHelp() {
	const wchar_t* help = L"AUD Programmimg utility v0.1. Please report errors to shuher@list.ru\nUsage:\n\
audprog.exe -d <FTDI device number> -i <interface> -m <AUD mode> -a <action> -o <offset> -s <size> -l <lenght> -f <filename>\n\
               <FTDI device number> - number of FTDI device (useful for multichannel devices like FT2232)\n\
               <--list> - use \"audprog.exe --list\" to query the list of FTDI devices attached to the system to determine the correct one for usage\n\
               <interface> - \"AUD\" or \"EE\"\n\
               <AUD mode> - \"byte\", \"word\" or \"longword\"\n\
               <action> - \"read\", \"write\", \"print\", \"dump\" or \"monitor\"\n\
               <offset> - to start from, decimal or hexadecimal\n\
               <size> - ROM size for AUD dumping - \"512\" or \"1024\", EEPROM size for EEPROM dumping (in kBytes) - \"1\" or \"4\" for 93cxx ICs, \"32\" or \"64\" for 95xxx ICs\n\
               <lenght> - data size to be read from AUD (in bytes, words or longwords depending on AUD mode)\n\
               <filename> - file to save/dump to or to read from";

	wprintf(help);
}

unsigned long le2be(unsigned long leData)
{
	return ((leData & 0xFF00FF00) >> 8) | ((leData & 0x00FF00FF) << 8);
}

unsigned long lw2bw(unsigned long leData)
{
	return ((leData & 0xFFFF0000) >> 16) | ((leData & 0x0000FFFF) << 16);
}

HANDLE CreateStorage(wchar_t* filename) {
	HANDLE hStorage = CreateFile(filename,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL, // no security attributes
		CREATE_ALWAYS,
		0, // not overlapped I/O
		NULL // hTemplate must be NULL for comm devices
		);
	if (hStorage == INVALID_HANDLE_VALUE) {
		// Handle the error.
		wprintf(L"CreateFile %s failed with error %d.\n", filename, GetLastError());
		return (INVALID_HANDLE_VALUE);
	}
	return hStorage;
}

HANDLE OpenStorage(wchar_t* filename) {
	HANDLE hStorage = CreateFile(filename,
		GENERIC_READ,
		0,
		NULL, // no security attributes
		OPEN_EXISTING,
		0, // not overlapped I/O
		NULL // hTemplate must be NULL for comm devices
		);
	if (hStorage == INVALID_HANDLE_VALUE) {
		// Handle the error.
		wprintf(L"Open file %s failed with error %d.\n", filename, GetLastError());
		return (INVALID_HANDLE_VALUE);
	}
	return hStorage;
}

void FT_PrintDeviceList() {
	FT_DEVICE_LIST_INFO_NODE *devInfo;
	unsigned long numDevs;
	FT_CreateDeviceInfoList(&numDevs);

	if (numDevs > 0) {
		devInfo = (FT_DEVICE_LIST_INFO_NODE*)malloc(sizeof(FT_DEVICE_LIST_INFO_NODE)*numDevs);

		if (FT_GetDeviceInfoList(devInfo, &numDevs) == FT_OK) {
			for (unsigned int i = 0; i < numDevs; i++)  {
				printf_s("Device %d:  \"%s\"\n", i, devInfo[i].Description);
			}
		}
	} else {
		wprintf_s(L"No FTDI deivces found\n");
	}
}

FT_HANDLE FTDI_Init(unsigned int devNum) {
	FT_HANDLE ftDevice;
	unsigned long numDevs = 0;

	if (FT_CreateDeviceInfoList(&numDevs) != FT_OK) {
		printf("No FTDI devices found\n");
		return NULL;
	}

	if (devNum > numDevs) {
		printf("Incorrect deivice number specified\n");
		return NULL;
	}

	FT_DEVICE_LIST_INFO_NODE *devInfo = (FT_DEVICE_LIST_INFO_NODE*)malloc(sizeof(FT_DEVICE_LIST_INFO_NODE)*numDevs);
	FT_GetDeviceInfoList(devInfo, &numDevs);
	FT_OpenEx(devInfo[devNum].Description, FT_OPEN_BY_DESCRIPTION, &ftDevice);

	FT_ResetDevice(ftDevice);

	if (FT_SetBaudRate(ftDevice, 460800) != FT_OK)
	{
		printf("Baudraet set error\n");
		return NULL;
	}

	if (FT_SetLatencyTimer(ftDevice, 1) != FT_OK) {
		printf("Latency set error\n");
		return NULL;
	}

	if (FT_SetUSBParameters(ftDevice, 128, 128) != FT_OK) {
		printf("USB transfer size error\n");
		return NULL;
	}

	return ftDevice;
}

int getopts(PARAMS *params, int argc, wchar_t* argv[]){
	
	for (int q = 1; q < argc - 1; q++) {

		// getting filename 
		if (wcscmp(argv[q], L"-f") == 0) {
			if (wcsstr(argv[q + 1], L"-") != argv[q + 1]) { // filename starting from "-" is probably a result of ommiting filename itself
				params->fileName = (wchar_t*)(argv[q + 1]);
			}
			else {
				wprintf_s(L"Filename seems to be omitted \"%s\"\nPlease enter filename not starting from \"-\" symbol\n", argv[q + 1]);
				return 1;
			}
		}

		// getting FTDI device number
		if (wcscmp(argv[q], L"-d") == 0) {
			if (swscanf_s(argv[q + 1], L"%i", &params->ftDeviceNum) != 1) {
				wprintf_s(L"Incorrect device numder - \"%s\"  Using default value %d\n", argv[q + 1], params->ftDeviceNum);
			}
		}

		// choosing EEPROM or AUD 
		if (wcscmp(argv[q], L"-i") == 0) {
			if (wcscmp(argv[q + 1], L"AUD") == 0) {
				params->iface = AUD;
			}
			if (wcscmp(argv[q + 1], L"EE") == 0) params->iface = EEPROM;
			if (params->iface == DEVICE_IFACE_NONE) {
				wprintf_s(L"Incorrect interface type specified - \"%s\"\n", argv[q + 1]);
				return 1;
			}
		}

		// choosing AUD mode
		if (wcscmp(argv[q], L"-m") == 0) {
			if (wcscmp(argv[q + 1], L"byte") == 0) params->mode = AUD_BYTE;
			if (wcscmp(argv[q + 1], L"word") == 0) params->mode = AUD_WORD;
			if (wcscmp(argv[q + 1], L"longword") == 0) params->mode = AUD_LONGWORD;
			if ((params->mode == AUD_LONGWORD) && (wcscmp(argv[q + 1], L"longword") != 0)) {
				wprintf_s(L"Incorrect AUD mode specified - \"%s\"  Using default mode - \"LONGWORD\"\n", argv[q + 1]);
			}
		}

		// setting action
		if (wcscmp(argv[q], L"-a") == 0) {
			if (wcscmp(argv[q + 1], L"read") == 0) params->action = READ;
			if (wcscmp(argv[q + 1], L"write") == 0) params->action = WRITE;
			if (wcscmp(argv[q + 1], L"dump") == 0)  params->action = DUMP;
			if (wcscmp(argv[q + 1], L"print") == 0) params->action = PRINT;
			if (wcscmp(argv[q + 1], L"monitor") == 0) params->action = MONITOR;
			if (params->action == DEVICE_ACTION_NONE) {
				wprintf_s(L"Incorrect action specified - \"%s\"\n", argv[q + 1]);
				return 1;
			}
		}

		// getting offset
		if (wcscmp(argv[q], L"-o") == 0) {
			if (swscanf_s(argv[q + 1], L"%i", &params->offset) != 1) {
				wprintf_s(L"Incorrect offset specified - \"%s\" Using default value %d\n", argv[q + 1], params->offset);
			}
		}

		// getting lenght
		if (wcscmp(argv[q], L"-l") == 0) {
			if (swscanf_s(argv[q + 1], L"%i", &params->len) != 1) {
				wprintf_s(L"Incorrect lenght specified - \"%s\" Using default value %d\n", argv[q + 1], params->len);
			}
		}

		// choosing ROM size for dumping
		if (wcscmp(argv[q], L"-s") == 0) {
			if (wcscmp(argv[q + 1], L"512") == 0) params->romSize = ROM_512;
			if (wcscmp(argv[q + 1], L"1024") == 0) params->romSize = ROM_1024;
			if (wcscmp(argv[q + 1], L"32") == 0) params->romSize = EE_32;
			if (wcscmp(argv[q + 1], L"64") == 0) params->romSize = EE_64;
			if ((params->romSize == ROM_512) && (wcscmp(argv[q + 1], L"512") != 0)) {
				wprintf_s(L"Incorrect ROM size specified - \"%s\" Using default size - \"512\"\n", argv[q + 1]);
			}
		}

	}

	params->ftDevice = FTDI_Init(params->ftDeviceNum);
	return 0;
}


int AUD_uploadFile(PARAMS* params) {
	HANDLE iFile = OpenStorage(params->fileName);
	params->len = GetFileSize(iFile, NULL);
	unsigned long b;
	unsigned long a = params->offset;
	unsigned char bData;
	unsigned short wData;
	unsigned long lData;

	if (params->mode == AUD_WORD) a = (a / 2) * 2; // to avoid unaligned access 4n+1 or 4n+3
	if (params->mode == AUD_LONGWORD) a = (a / 4) * 4; // to avoid unaligned access 4n+1, 4n+2 or 4n+3
	printf("\nUploading from offset 0x%06x\n", a);
	AUD_SHSetRAMmode(params->ftDevice);

	while (a < params->offset + params->len) {
		switch (params->mode) {
		case AUD_BYTE:
			ReadFile(iFile, &bData, sizeof(bData), &b, NULL);
			AUD_writeByte(params->ftDevice, a, bData);			
			a++;
			printf("%2d%%\r", (100 * (a - params->offset) / (params->len)));
			break;
		case AUD_WORD:
			ReadFile(iFile, &wData, sizeof(wData), &b, NULL);
			AUD_writeWord(params->ftDevice, a, wData);
			a += 2;
			printf("%2d%%\r", (100 * (a - params->offset) / (params->len)));
			break;
		case AUD_LONGWORD:
			ReadFile(iFile, &lData, sizeof(lData), &b, NULL);
			AUD_writeLWord(params->ftDevice, a, lData);
			a += 4;
			printf("%2d%%\r", (100 * (a - params->offset) / (params->len)));
			break;
		}
	}
	CloseHandle(iFile);
	printf_s("%d bytes written\n", params->len);
	return 0;
}

int AUD_readToFile(PARAMS* params) {
	HANDLE oFile = CreateStorage(params->fileName);
	unsigned long b;
	unsigned long a = params->offset;
	unsigned char bData;
	unsigned short wData;
	unsigned long lData;

	if (params->mode == AUD_WORD) {
		a = (a / 2) * 2; // to avoid unaligned access 4n+1 or 4n+3
		params->len *= 2;
	}
	if (params->mode == AUD_LONGWORD) {
		a = (a / 4) * 4; // to avoid unaligned access 4n+1, 4n+2 or 4n+3
		params->len *= 4;
	}
	printf("\nReading from offset 0x%06x\n", a);

	AUD_SHSetRAMmode(params->ftDevice);

	while (a < params->offset + params->len) {
		switch (params->mode) {
		case AUD_BYTE:
			bData = AUD_readByte(params->ftDevice, a);
			WriteFile(oFile, &bData, sizeof(bData), &b, NULL);
			a++;
			printf("%2d%%\r", (100 * (a - params->offset) / (params->len)));
			break;
		case AUD_WORD:
			wData = AUD_readWord(params->ftDevice, a);
			WriteFile(oFile, &wData, sizeof(wData), &b, NULL);
			a += 2;
			printf("%2d%%\r", (100 * (a - params->offset) / (params->len)));
			break;
		case AUD_LONGWORD:
			lData = AUD_readLWord(params->ftDevice, a);
			WriteFile(oFile, &lData, sizeof(lData), &b, NULL);
			a += 4;
			printf("%2d%%\r", (100 * (a - params->offset) / (params->len)));
			break;
		}
	}
	CloseHandle(oFile);
	return 0;
}

int AUD_readToScreen(PARAMS* params) {
	
	unsigned long pData;

	if (params->mode == AUD_WORD) params->offset = (params->offset / 2) * 2; // to avoid unaligned access 4n+1 or 4n+3
	if (params->mode == AUD_LONGWORD) params->offset = (params->offset / 4) * 4; // to avoid unaligned access 4n+1, 4n+2 or 4n+3
	unsigned long a = params->offset;

	printf_s("\n");
	
	AUD_SHSetRAMmode(params->ftDevice);

	while (a < params->offset + params->len) {
		switch (params->mode) {
		case AUD_BYTE:
			pData = le2be(AUD_readByte(params->ftDevice, a));
			if ((a - params->offset) % 0x10 == 0) printf_s("\n0x%06x  ", a);
			printf_s("%02x ", pData);
			a++;
			break;
		case AUD_WORD:
			pData = le2be(AUD_readWord(params->ftDevice, a));
			if ((a - params->offset) % 0x10 == 0) printf_s("\n0x%06x  ", a);
			printf_s("%04x ", pData);
			a += 2;
			break;
		case AUD_LONGWORD:
			pData = lw2bw(le2be(AUD_readLWord(params->ftDevice, a)));
			if ((a - params->offset) % 0x010 == 0) printf_s("\n0x%06x  ", a);
			printf_s("%08x ", pData);
			a += 4;
			break;
		}
	}
	return 0;
}

int AUD_monitor(PARAMS* params) {

	unsigned long lData;

	params->offset = (params->offset / 4) * 4; // to avoid unaligned access 4n+1, 4n+2 or 4n+3
	unsigned long a = params->offset;

	AUD_SHSetRAMmode(params->ftDevice);
	while (1) {
		lData = lw2bw(le2be(AUD_readLWord(params->ftDevice, a)));
		printf_s("\r0x%06x  ", a);
		printf_s("%08x ", lData);
	}
	return 0;
}


int AUD_dumpToFile(PARAMS* params) {
	unsigned long a = 0;
	int speed = 0, total = 0, elapsed = 0, left = 0;
	unsigned long b;
	unsigned long long oTick = GetTickCount64();
	unsigned long long iTick = GetTickCount64();
	unsigned long size = 0x80000;

	if (params->romSize == ROM_1024) size *= 2;
	printf("\nDumping %d bytes\n", size);

	AUD_SHSetRAMmode(params->ftDevice);

	HANDLE oFile = CreateStorage(params->fileName);
	while (a < size) {
		unsigned long d = (AUD_readLWord(params->ftDevice, a));
		WriteFile(oFile, &d, sizeof(d), &b, NULL);
		a += 4;
		if (a % 0x400 == 0) {
			unsigned long long cTick = GetTickCount64();
			speed = (int)(0x400 * 1000 / (cTick - oTick));

			total = (int)(size / speed) / 60;
			elapsed = (int)((cTick - iTick) / 1000) / 60;
			left = total - elapsed;
			oTick = cTick;
		}
		wprintf(L"Reading... 0x0%06x %02d%%  %d bps ", a, (a * 100 / size), speed);
		wprintf(L"%d min elapsed, %d min left.\r", elapsed, left);
	}
	CloseHandle(oFile);
	return 0;
}

//EEPROM
int EE_readToFile(PARAMS* params) {
	if (!((params->romSize == EE_32) || (params->romSize == EE_64))) {
		wprintf(L"\nERROR - incorrect EEPROM size\n");
		return 1;
	};

	HANDLE oFile = CreateStorage(params->fileName);
	DWORD b;
	unsigned int a = 0;

	printf("\nReading\n");

	if (FT_SetBitMode(params->ftDevice, MOSI | CS | CLK, FT_BITMODE_SYNC_BITBANG) != FT_OK) {
		printf("Error setting bit bang mode\n");
		return 1;
	}

	unsigned long size = 0xFFF;
	if (params->romSize == EE_64) size = 0x1FFF;

	for (a = 0; a <= size; a++) {
		unsigned char data = devReadByte(params->ftDevice, a);
		WriteFile(oFile, &data, sizeof(data), &b, NULL);
		printf("  \r%02d%%", ((100 * a) / size));
	}
	printf("\r100%%\n%d bytes saved\n", a);
	CloseHandle(oFile);
	return 0;
}

int EE_writeFile(PARAMS* params) {
	DWORD b;
	unsigned int a = 0;
	HANDLE iFile = OpenStorage(params->fileName);
	unsigned int len = GetFileSize(iFile, NULL);
	unsigned char data = 0;

	printf("\nWriting...\n");

	if (FT_SetBitMode(params->ftDevice, MOSI | CS | CLK, FT_BITMODE_SYNC_BITBANG) != FT_OK) {
		printf("Error setting bit bang mode\n");
		return 1;
	}

	for (a = 0; a < len; a++) {
		ReadFile(iFile, &data, sizeof(data), &b, NULL);
		devWriteByte(params->ftDevice, data, a);
		printf("  \r%02d%%", ((100 * a) / len));
	}

	writeDisable(params->ftDevice);
	printf("\r100%%\n%d bytes written.\n", a);
	CloseHandle(iFile);
	return 0;
}

int _tmain(int argc, _TCHAR* argv[]) {
	// audprog.exe -d <FTDI device number> -i <interface> -m <AUD mode> -a <action> -o <offset> -l <length> -s <size> -f <filename>

	PARAMS params;
	unsigned long num = 0;
	FT_CreateDeviceInfoList(&num);

	if (argc == 1) {
		printHelp();
		return 0;
	}

	//printing FTDI device list
	if ((wcscmp(argv[1], L"--list") == 0) && argc == 2) {
		FT_PrintDeviceList();
		return 0;
	}
	else {
		if (num > 1) wprintf_s(L"There are %d FTDI devices found, please ensure that the correct one is used. Use \"audprog.exe --list\" to query available devices.\n", num);
	}

	if (getopts(&params, argc, argv) != 0) return 1; // parameters were exctracted OK, let's decide what to do next

	if (params.iface == AUD) {
		switch (params.action) {
		case READ:
			AUD_readToFile(&params); break;
		case WRITE:
			AUD_uploadFile(&params); break;
		case PRINT:
			AUD_readToScreen(&params); break;
		case DUMP:
			AUD_dumpToFile(&params); break;
		case MONITOR:
			AUD_monitor(&params); break;
		}
		return 0;
	}

	if (params.iface == EEPROM) { 
			switch (params.action) {
			case READ: 
				if (params.romSize == ROM_512 || params.romSize == ROM_1024) {
					params.romSize = EE_32; // stupid way to process EE_SIZE options in the same way as ROM_SIZE
					wprintf_s(L"Incorrect EEPROM size specified (use \"-s 32\" or \"-s 64\" option for this), defaulting to 32kb EEPROM size\n");
				}
				EE_readToFile(&params); 
				break;
			case WRITE: EE_writeFile(&params); break;
			default: wprintf_s(L"Requested action cannot be performed for EEPROM\n");
			}
			return 0;
		}
	printHelp();
	return 1;
}