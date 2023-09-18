#pragma once

#include <winsock2.h>

// Types
#define U8  unsigned char
#define U16 unsigned short
#define U32 unsigned int 
#define U64 unsigned __int64
#define S8  signed char
#define S16 signed short
#define S32 signed int
#define S64 signed __int64

#define VER_STRING              "2.0.1"

////////////////////////////////////////////////////////////////////////////////////////////

// General Dworx Error Codes - should be common for xtags, xgateways, xtagd, and apps.
#define DW_SUCCESS						0x00    // All good.
#define DW_ERR_BAD_ARG					0x01    // Bad cmd param like length.
#define DW_ERR_CMD_PROCESS				0x02    // Could not process cmd (i.e. BMI-160 error).
#define DW_ERR_NO_CONN					0x03    // Tried executing a command without properly connecting first.
#define DW_ERR_CMD_STATE				0x04    // Target was in an impropper state when getting the cmd (eg. try stop streaming without start).
#define DW_ERR_GEN						0x7F    // Gen purpose bad.


// xGw and xTg BLE and USB General Msgs.
#define XGWBLE_READ_USB_DEV_ID_CMD      0x01    // Get the ID of a usb-connected device (MAC). Used only on USB-connected xTAGs.
#define XGWBLE_META_READ_CMD            0x01    // Read xGATEWAY-Resident BLE Chip meta data (sec running and firmware ver). Sckt and usb cmd.
#define XGWBLE_LIST_XTAGS_CMD           0x02    // List scanned BLE and connected USB xTAGs
#define XGWBLE_CONNECT_XTAG_CMD         0x03    // Connect to BLE and USB xTAGs
#define XGWBLE_DISCONNECT_XTAG_CMD      0x04    // Disconnect BLE and USB xTAGs
#define XTAGBLE_META_READ_CMD           0x05    // xTAG BLE/USB Meta Read. BLE on xGw and xTg Explorer should be updated to include cfg timestamp.
#define XTAGBLE_META_WRITE_CMD          0x06    // xTAG BLE/USB Meta Write. BLE on xGw and xTg Explorer should be updated to include cfg timestamp.
#define XTAGBLE_METAEX_READ_CMD         0x07    // xTAG BLE Extended Meta Read - Used by xTAG Explorer app only.
#define XTAGBLE_METAEX_WRITE_CMD        0x08    // xTAG BLE Extended Meta Read - Used by xTAG Explorer app only.
#define XGWBLE_SYNC_XTAGS_CMD           0x09    // Gets a list of connected BLE xTAGs through the Usb interface to the xGw BLE chip.

// xTg HaLow General Msgs.
#define XTAGHLO_HBEAT_MSG               0x0A    // Unsolicited xTAG HaLow msgs that are streamed through the stream sckt.
#define XGWHLO_XTAG_META_READ_CMD       0x0D    // Sckt Cmd to read xTg meta data.
#define XGWHLO_XTAG_META_WRITE_CMD      0x0E    // Sckt Cmd to write xTg meta data and HaLow cmd to write meta on xTAG
#define XGWHLO_XTAG_FOTA_CMD            0x0F    // Sckt Cmd to update FOTA and HaLow cmd to handle FOTA update.
#define XGWHLO_XTAG_DISCONNECT_CMD      0x10    // Sent from xGw to xTg to disconnect HaLow.

// xTg Sensor General Acq Msgs.
#define XTAG_ACQ_START_CMD              0x18    // Sckt and BLE/USB/Halow cmd
#define XTAG_ACQ_STOP_CMD               0x19    // Sckt and BLE/USB/Halow cmd

// xGw HaLow General Msgs.
#define XGWHLO_HBEAT_MSG                0x1A    // Unsolicited xTAG HaLow msgs that are streamed through the stream sckt.
#define XGWHLO_META_READ_CMD            0x1B    // Sckt Cmd to read xGw meta data.
#define XGWHLO_META_WRITE_CMD           0x1C    // Sckt Cmd to write xGw meta data.

// xTg Vibration BLE/USB/HaLow Msgs
#define XTAG_VIB_CALIBRATE_CMD          0x21
#define XTAG_VIB_CONFIG_READ_CMD        0x23
#define XTAG_VIB_CONFIG_WRITE_CMD       0x24
#define XTAG_VIB_STREAM_START_CMD       0x26
#define XTAG_VIB_STREAM_STOP_CMD        0x27
#define XTAG_VIB_ACQ_STREAM_MSG         0x2A

// xTg Beacon BLE Msgs (Only the xTg Explorer App  supports these). 
#define XTAGBLE_BCN_RESERVED1           0x33   
#define XTAGBLE_BCN_RESERVED2           0x34    
#define XTAGBLE_BCN_RESERVED3           0x35 
#define XTAGBLE_BCN_RESERVED4           0x36  
#define XTAGBLE_BCN_RESERVED5           0x37    

// xTg Environmental BLE/USB/HaLow Msgs (Supports both BME688 and SHT4x sensor chips).
#define XTAG_ENV688_CONFIG_READ_CMD     0x40
#define XTAG_ENV688_CONFIG_WRITE_CMD    0x41
#define XTAG_ENVSHT4x_CONFIG_READ_CMD   0x42
#define XTAG_ENVSHT4x_CONFIG_WRITE_CMD  0x43
#define XTAG_ENV_ACQ_READ_CMD           0x4A

// xTg CO2 BLE/USB/HaLow Msgs
#define XTAG_CO2_CONFIG_READ	        0x50
#define XTAG_CO2_CONFIG_WRITE		    0x51
#define XTAG_CO2_FRC                    0x54
#define XTAG_CO2_ACQ_READ_CMD           0x5A

#define XGW_CMD_SHUTDOWN                0xFF    // Supports restart of xGw-Resident BLE firmware and shutdown of this app. (both on sckt cmd).
