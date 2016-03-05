# audprog
AUD interface read/write utility for SH7055 and SH7058 CPUs based on FTDI's FT232R/FT2232x chips

connections:  
FTDI  - SH705x CPU  
DBUS0 - AUD_RST  
DBUS1 - AUD_SYNC  
DBUS2 - AUD_MD  
DBUS3 - AUD_CK  
DBUS4 - AUD_DATA0  
DBUS5 - AUD_DATA1  
DBUS6 - AUD_DATA2  
DBUS7 - AUD_DATA3  
  
FTDI  - ST95xxx EEPROM  
DBUS0 - SCK  
DBUS1 - DO  
DBUS2 - DI  
DBUS3 - CS  
