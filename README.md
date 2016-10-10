# audprog
AUD interface read/write utility for SH7055 and SH7058 CPUs based on FTDI's FT232R/FT2232x chips

connections:  
FTDI  - SH705x CPU  
DBUS0 - Not Connected  
DBUS1 - AUD_DATA1  
DBUS2 - AUD_DATA0  
DBUS3 - AUD_SYNC  
DBUS4 - AUD_RST  
DBUS5 - AUD_DATA3  
DBUS6 - AUD_CK  
DBUS7 - AUD_DATA2  
  
FTDI  - ST95xxx EEPROM  
DBUS0 - DO  
DBUS3 - DI  
DBUS4 - CS  
DBUS6 - CK  

HW schematics in KiCAD format is also inclueded in .\hw\ folder