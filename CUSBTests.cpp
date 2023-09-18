#include "CUSBTests.h"

CUSBTests::CUSBTests(void) { }
CUSBTests::~CUSBTests() { }

bool CUSBTests::ExGATEWAYTest(SOCKET PrimeSkt, SOCKET StreamSkt)
{
	// Note that socket connections have been made before this method is called and it simply handles messaging over sockets.

	/////////////////////////////////////////////////////////
	// Read the xGATEWAY Metadata. 
	/////////////////////////////////////////////////////////

	// Indicate Read is starting.
	printf("xGw USB Read Metadata ...  ");
	
	// Setup cmd bytes.
	const U8 ReadxGwMetaMsgLen = 2;
	U8 ReadxGwMetaCmdBytes[] = { XGWHLO_META_READ_CMD, ReadxGwMetaMsgLen };

	// Process the cmd and provide an array for response.
	U8 MetaReadRespBytes[PRIMARY_REC_BUFLEN];
	bool CmdExResult = false;
	CmdExResult = CUtility::ProcCmdForResp(PrimeSkt, ReadxGwMetaCmdBytes, ReadxGwMetaMsgLen, MetaReadRespBytes, PRIMARY_REC_BUFLEN, 1000); // 1000 mSec timeout.

	// Handle gen error.
	if (CmdExResult == false)
	{
		printf("Error executing xGATEWAY USB test. Metadata Read cmd processing error.\r\n");
		return false;
	}

	// Validate response length and status byte.
	if((MetaReadRespBytes[1] != 0x12) || (MetaReadRespBytes[2] != DW_SUCCESS))
	{
		printf("Error executing xGATEWAY USB test. Metadata Read response has bad length of %d instead of 18 or status %d instead of 0\r\n",
			MetaReadRespBytes[1], MetaReadRespBytes[2]);
		return false;
	}

	// Indicate Read is done.
	printf("completed.\r\n");

	// Display meta.
	printf("xGw Metadata: ..................................................................\r\n");
	U32 xGwConfigUnixTime =((U32)MetaReadRespBytes[3] << 24) & 0xFF000000;
	xGwConfigUnixTime += ((U32)MetaReadRespBytes[4] << 16) & 0x00FF0000;
	xGwConfigUnixTime += ((U32)MetaReadRespBytes[5] << 8) & 0x0000FF00;
	xGwConfigUnixTime += ((U32)MetaReadRespBytes[6] << 0) & 0x000000FF;
	printf("Config unix time: %d\r\n", xGwConfigUnixTime);
	U16 xGwChannel = ((U16)MetaReadRespBytes[7] << 8) & 0xFF00;
	xGwChannel += ((U16)MetaReadRespBytes[8] << 0) & 0x00FF;
	printf("Config channel: %d\r\n", xGwChannel);
	printf("Config tx power: %d\r\n", MetaReadRespBytes[9]);
	S32 xGwConfigLat = ((S32)MetaReadRespBytes[10] << 24) & 0xFF000000;
	xGwConfigLat += ((S32)MetaReadRespBytes[11] << 16) & 0x00FF0000;
	xGwConfigLat += ((S32)MetaReadRespBytes[12] << 8) & 0x0000FF00;
	xGwConfigLat += ((S32)MetaReadRespBytes[13] << 0) & 0x000000FF;
	printf("Config anchored lat (with 7 implied decimals): %ld\r\n", xGwConfigLat);
	S32 xGwConfigLon = ((S32)MetaReadRespBytes[14] << 24) & 0xFF000000;
	xGwConfigLon += ((S32)MetaReadRespBytes[15] << 16) & 0x00FF0000;
	xGwConfigLon += ((S32)MetaReadRespBytes[16] << 8) & 0x0000FF00;
	xGwConfigLon += ((S32)MetaReadRespBytes[17] << 0) & 0x000000FF;
	printf("Config anchored lon (with 7 implied decimals): %ld\r\n", xGwConfigLon);
	printf("................................................................................\r\n");


	///////////////////////////////////////////////////////////////////////
	// Write an xGATEWAY Metadata update. 
	// Re-execute read to validate any changed vars like Config Unixtime.
	///////////////////////////////////////////////////////////////////////

	// Indicate Write is starting.
	printf("xGw Write Metadata ... ");

	// Set config channel to 7.
	xGwChannel = 7;

	// Setup cmd bytes.
	const U8 WritexGwMetaMsgLen = 0x11;
	U8 WritexGwMetaCmdBytes[WritexGwMetaMsgLen];
	WritexGwMetaCmdBytes[0] = XGWHLO_META_WRITE_CMD;
	WritexGwMetaCmdBytes[1] = WritexGwMetaMsgLen;

	xGwConfigUnixTime = CUtility::GetCurrentUnixTimeUTC();
	WritexGwMetaCmdBytes[2] = (U8)((xGwConfigUnixTime >> 24) & 0x000000FF);
	WritexGwMetaCmdBytes[3] = (U8)((xGwConfigUnixTime >> 16) & 0x000000FF);
	WritexGwMetaCmdBytes[4] = (U8)((xGwConfigUnixTime >> 8) & 0x000000FF);
	WritexGwMetaCmdBytes[5] = (U8)((xGwConfigUnixTime >> 0) & 0x000000FF);
	WritexGwMetaCmdBytes[6] = (U8)((xGwChannel >> 8) & 0x00FF);			
	WritexGwMetaCmdBytes[7] = (U8)((xGwChannel >> 0) & 0x00FF);
	WritexGwMetaCmdBytes[8] = MetaReadRespBytes[9];								// ReUse incoming Tx power.
	WritexGwMetaCmdBytes[9] = (U8)((xGwConfigLat >> 24) & 0x000000FF);			// ReUse incoming Lat
	WritexGwMetaCmdBytes[10] = (U8)((xGwConfigLat >> 16) & 0x000000FF);
	WritexGwMetaCmdBytes[11] = (U8)((xGwConfigLat >> 8) & 0x000000FF);
	WritexGwMetaCmdBytes[12] = (U8)((xGwConfigLat >> 0) & 0x000000FF);
	WritexGwMetaCmdBytes[13] = (U8)((xGwConfigLon >> 24) & 0x000000FF);			// ReUse incoming Lon
	WritexGwMetaCmdBytes[14] = (U8)((xGwConfigLon >> 16) & 0x000000FF);
	WritexGwMetaCmdBytes[15] = (U8)((xGwConfigLon >> 8) & 0x000000FF);
	WritexGwMetaCmdBytes[16] = (U8)((xGwConfigLon >> 0) & 0x000000FF);
	
	// Process the cmd and provide an array for response.
	U8 MetaWriteRespBytes[PRIMARY_REC_BUFLEN];
	CmdExResult = false;
	CmdExResult = CUtility::ProcCmdForResp(PrimeSkt, WritexGwMetaCmdBytes, WritexGwMetaMsgLen, MetaWriteRespBytes, PRIMARY_REC_BUFLEN, 1000); // 1000 mSec timeout.

	// Handle gen error.
	if (CmdExResult == false)
	{
		printf("Error executing xGATEWAY USB test. Metadata Write cmd processing error.\r\n");
		return false;
	}

	// Validate response length and status byte.
	if ((MetaWriteRespBytes[1] != 3) || (MetaWriteRespBytes[2] != DW_SUCCESS))
	{
		printf("Error executing xGATEWAY USB test. Metadata Write response has bad length of %d instead of 3 or status %d instead of 0\r\n",
			MetaWriteRespBytes[1], MetaWriteRespBytes[2]);
		return false;
	}

	// Indicate Write is done.
	printf("completed.\r\n");

	///////////////////////////////////////////////////////////////////////
	// Iteratively read incoming heartbeats on the stream socket.
	///////////////////////////////////////////////////////////////////////

	// Provide a prompt giving the user a chance to cancel while waiting for heartbeats.
	printf("Printing incoming xGATEWAY heartbeats. Hit spacebar to terminate this last step at any time.\r\n");

	U8 HbStreamBuf[STREAM_REC_BUFLEN];
	U32 NumBytesRecieved = 0;
	const U8 xGwHeartbeatLen = 0x1E;
	while (true)
	{
		// Look for a spacebar key press and exit accordingly.
		if (GetKeyState(VK_SPACE) & 0x8000) { break; }

		// Read into HbStreamBuf
		int NumBytesRecieved = recv(StreamSkt, (char*)HbStreamBuf, STREAM_REC_BUFLEN, 0);
		if (NumBytesRecieved == 0)
		{
			printf("Stream socket disconnected during xGATEWAY USB Test.\n");	// Should never see this as we manage disconnect.
			return false;
		}
		else if (NumBytesRecieved < 0)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				Sleep(50);  // Give other threads some attention.
				continue;	// Try again.
			}
			else
			{
				printf("Socket recieve failed with error %d during xGATEWAY USB Test\n", WSAGetLastError());
				return false;
			}
		}
		else if (NumBytesRecieved > 0)
		{
			// Ignore any other possible incomimg msgs (i.e. xTAG heartbeats, data streams, etc).
			if (HbStreamBuf[0] != XGWHLO_HBEAT_MSG) { continue; }

			// If there are backed up heartbeats (more bytes recieved than expected in 1 heartbeat) just continue.
			if (NumBytesRecieved > (U32)xGwHeartbeatLen) { continue; }
			else
			{
				// Confirm length
				if (NumBytesRecieved != (U32)xGwHeartbeatLen)
				{
					// Must be < xGwHeartbeatLen
					printf("Socket recieved partial heartbeat with length %d during xGATEWAY USB Test\n", WSAGetLastError());
					return false;
				}

				// Print out heartbeat data.
				printf("xGw Heartbeat...................................................................\r\n");

				printf("xGATEWAY Type ('0'-Pure '1'-LTE '2'-HaLow '3'-3.0 '4'-Tablet): %d\r\n", HbStreamBuf[3]);

				printf("xGATEWAY ID: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
					HbStreamBuf[4], HbStreamBuf[5], HbStreamBuf[6], HbStreamBuf[7], HbStreamBuf[8], HbStreamBuf[9]);

				U32 xGwRunTime = ((U32)HbStreamBuf[10] << 24) & 0xFF000000;
				xGwRunTime += ((U32)HbStreamBuf[11] << 16) & 0x00FF0000;
				xGwRunTime += ((U32)HbStreamBuf[12] << 8) & 0x0000FF00;
				xGwRunTime += ((U32)HbStreamBuf[13] << 0) & 0x000000FF;
				printf("Run time in sec: %d\r\n", xGwRunTime);

				U32 xGwUnixTime = ((U32)HbStreamBuf[14] << 24) & 0xFF000000;
				xGwUnixTime += ((U32)HbStreamBuf[15] << 16) & 0x00FF0000;
				xGwUnixTime += ((U32)HbStreamBuf[16] << 8) & 0x0000FF00;
				xGwUnixTime += ((U32)HbStreamBuf[17] << 0) & 0x000000FF;
				printf("Current xGATEWAY Unix time in sec: %d\r\n", xGwUnixTime);

				U16 xGwHWRev = ((U16)HbStreamBuf[18] << 8) & 0xFF00;
				xGwHWRev += ((U16)HbStreamBuf[19] << 0) & 0x00FF;
				printf("xGATEWAY Hardware Rev with 2 implied decimals: %d\r\n", xGwHWRev);

				U16 xGwSWRev = ((U16)HbStreamBuf[20] << 8) & 0xFF00;
				xGwSWRev += ((U16)HbStreamBuf[21] << 0) & 0x00FF;
				printf("xGATEWAY Software Rev with 2 implied decimals: %d\r\n", xGwSWRev);

				S32 xGwLat = ((S32)HbStreamBuf[22] << 24) & 0xFF000000;
				xGwLat += ((S32)HbStreamBuf[23] << 16) & 0x00FF0000;
				xGwLat += ((S32)HbStreamBuf[24] << 8) & 0x0000FF00;
				xGwLat += ((S32)HbStreamBuf[25] << 0) & 0x000000FF;
				printf("xGATEWAY lat (with 7 implied decimals): %ld\r\n", xGwLat);

				S32 xGwLon = ((S32)HbStreamBuf[26] << 24) & 0xFF000000;
				xGwLon += ((S32)HbStreamBuf[27] << 16) & 0x00FF0000;
				xGwLon += ((S32)HbStreamBuf[28] << 8) & 0x0000FF00;
				xGwLon += ((S32)HbStreamBuf[29] << 0) & 0x000000FF;
				printf("xGATEWAY lon (with 7 implied decimals): %ld\r\n", xGwLon);
				printf("................................................................................\r\n");
			}
		}
	} // while(true)

	return true;
}

bool CUSBTests::ExTAGVibTest(SOCKET PrimeSkt, SOCKET StreamSkt)
{
	// Note that socket connections have been made before this method is called and it simply handles messaging over sockets.

	/////////////////////////////////////////////////////////
	// List USB connected xTAGs 
	/////////////////////////////////////////////////////////

	// Indicate Read is starting.
	printf("Getting connected xTAG USB Sensors ... ");

	// Setup cmd bytes.
	const U8 ListxTgSensorsMsgLen = 3;
	U8 ListxTgSensorsCmdBytes[] = { XGWBLE_LIST_XTAGS_CMD, ListxTgSensorsMsgLen, 5};	// Timeout in 5 sec.

	// Process the cmd and provide an array for response.
	U8 ListxTgSensorsRespBytes[PRIMARY_REC_BUFLEN];
	bool CmdExResult = false;
	CmdExResult = CUtility::ProcCmdForResp(PrimeSkt, ListxTgSensorsCmdBytes, ListxTgSensorsMsgLen, ListxTgSensorsRespBytes, PRIMARY_REC_BUFLEN, 1000); // 1000 mSec timeout.

	// Handle gen error.
	if (CmdExResult == false)
	{
		printf("Error executing xTAG USB Vib test. xTAG USB list cmd processing error.\r\n");
		return false;
	}

	// Validate response status byte (length varies based on number of xTAGs listed).
	if (ListxTgSensorsRespBytes[2] != DW_SUCCESS)
	{
		printf("Error executing xTAG USB Vib test. xTAG USB list has bad status %d instead of 0\r\n", ListxTgSensorsRespBytes[2]);
		return false;
	}

	// Indicate list is done.
	printf("completed. Will list xTAG USB Sensors:\r\n");

	// Validate that there is a correct byte length (must be multiple of 7 plus 3 byte header or just 3 bytes when there are no xTAGs)
	U8 NumSensors = 0;
	if (ListxTgSensorsRespBytes[1] == 3)
	{
		printf("No connected xTAG USB Sensors. Cannot continue test.\r\n");
		return false;
	}
	else
	{
		if (((ListxTgSensorsRespBytes[1] - 3) % 7) != 0)
		{
			printf("Invalid number of bytes in xTAG USB list response: %d\r\n", ListxTgSensorsRespBytes[1]);
			return false;
		}
		else
		{
			NumSensors = (ListxTgSensorsRespBytes[1] - 3) / 7;
		}
	}

	// Print Available xTAG USB Sensors.
	for (int x = 0; x < NumSensors; x++)
	{
		printf("xTAG: %02X:%02X:%02X:%02X:%02X:%02X Connected ('0'-No, '1'-Yes): %d\r\n",
			ListxTgSensorsRespBytes[4 + (7 * x)], ListxTgSensorsRespBytes[5 + (7 * x)], ListxTgSensorsRespBytes[6 + (7 * x)],
			ListxTgSensorsRespBytes[7 + (7 * x)], ListxTgSensorsRespBytes[8 + (7 * x)], ListxTgSensorsRespBytes[9 + (7 * x)],
			ListxTgSensorsRespBytes[3+(7*x)]);
	}

	/////////////////////////////////////////////////////////
	// Connect to the first found xTAG USB Sensor.
	/////////////////////////////////////////////////////////

	printf("Connecting to the first listed xTAG USB Sensor ... ");

	// First - store the ID for subsequent test use.
	const U8 TestxTAGId[6] = { ListxTgSensorsRespBytes[4], ListxTgSensorsRespBytes[5], ListxTgSensorsRespBytes[6],
						ListxTgSensorsRespBytes[7], ListxTgSensorsRespBytes[8], ListxTgSensorsRespBytes[9] };

	// Setup cmd bytes.
	const U8 ConnectxTgSensorMsgLen = 8;
	U8 ConnectxTgSensorCmdBytes[] = { XGWBLE_CONNECT_XTAG_CMD, ConnectxTgSensorMsgLen, 
		TestxTAGId[0], TestxTAGId[1], TestxTAGId[2], TestxTAGId[3], TestxTAGId[4], TestxTAGId[5] };	// Timeout in 5 sec.

	// Process the cmd and provide an array for response.
	U8 ConnectxTgSensorsRespBytes[PRIMARY_REC_BUFLEN];
	CmdExResult = false;
	CmdExResult = CUtility::ProcCmdForResp(PrimeSkt, ConnectxTgSensorCmdBytes, ConnectxTgSensorMsgLen, 
											ConnectxTgSensorsRespBytes, PRIMARY_REC_BUFLEN, 1000); // 1000 mSec timeout.

	// Handle gen error.
	if (CmdExResult == false)
	{
		printf("Error executing xTAG USB Vib test. xTAG connect cmd processing error.\r\n");
		return false;
	}

	// Validate response length and status byte.
	if ((ConnectxTgSensorsRespBytes[1] != 3) || (ConnectxTgSensorsRespBytes[2] != DW_SUCCESS))
	{
		printf("Error executing xTAG USB Vib test. xTAG Connect response has bad length of %d instead of 3 or status %d instead of 0\r\n",
			ConnectxTgSensorsRespBytes[1], ConnectxTgSensorsRespBytes[2]);
		return false;
	}

	printf("connected. Will relist xTAG USB Sensors:\r\n");

	// Relist to show connected state change. For brevity - just issue cmd and assume clean result.
	memset(ListxTgSensorsRespBytes, 0, PRIMARY_REC_BUFLEN);
	CUtility::ProcCmdForResp(PrimeSkt, ListxTgSensorsCmdBytes, ListxTgSensorsMsgLen, ListxTgSensorsRespBytes, PRIMARY_REC_BUFLEN, 1000);
	
	// Reset NumSensors.
	NumSensors = (ListxTgSensorsRespBytes[1] - 3) / 7;

	// Print Available xTAG USB Sensors.
	for (int x = 0; x < NumSensors; x++)
	{
		printf("xTAG: %02X:%02X:%02X:%02X:%02X:%02X Connected ('0'-No, '1'-Yes): %d\r\n",
			ListxTgSensorsRespBytes[4 + (7 * x)], ListxTgSensorsRespBytes[5 + (7 * x)], ListxTgSensorsRespBytes[6 + (7 * x)],
			ListxTgSensorsRespBytes[7 + (7 * x)], ListxTgSensorsRespBytes[8 + (7 * x)], ListxTgSensorsRespBytes[9 + (7 * x)],
			ListxTgSensorsRespBytes[3 + (7 * x)]);
	}

	/////////////////////////////////////////////////////////
	// Read Metadata from xTAG USB Sensor.
	/////////////////////////////////////////////////////////

	printf("Read xTAG USB Sensor Metadata ... ");

	// Setup cmd bytes.
	const U8 ReadxTgSensorMetaMsgLen = 8;
	U8 ReadxTgSensorMetaCmdBytes[] = { XTAGBLE_META_READ_CMD, ReadxTgSensorMetaMsgLen,
		TestxTAGId[0], TestxTAGId[1], TestxTAGId[2], TestxTAGId[3], TestxTAGId[4], TestxTAGId[5] };	// Timeout in 5 sec.

	// Process the cmd and provide an array for response.
	U8 ReadxTgSensorMetaRespBytes[PRIMARY_REC_BUFLEN];
	CmdExResult = false;
	CmdExResult = CUtility::ProcCmdForResp(PrimeSkt, ReadxTgSensorMetaCmdBytes, ReadxTgSensorMetaMsgLen,
		ReadxTgSensorMetaRespBytes, PRIMARY_REC_BUFLEN, 1000); // 1000 mSec timeout.

	// Handle gen error.
	if (CmdExResult == false)
	{
		printf("Error executing xTAG USB Vib test. xTAG Meta read cmd processing error.\r\n");
		return false;
	}

	// Validate response length and status byte.
	if ((ReadxTgSensorMetaRespBytes[1] != 18) || (ReadxTgSensorMetaRespBytes[2] != DW_SUCCESS))
	{
		printf("Error executing xTAG USB Vib test. xTAG Meta read response has bad length of %d instead of 18 or status %d instead of 0\r\n",
			ReadxTgSensorMetaRespBytes[1], ReadxTgSensorMetaRespBytes[2]);
		return false;
	}

	// Done.
	printf("completed.\r\n");

	// Print Metadata.
	printf("xTAG USB Sensor Metadata........................................................\r\n");

	printf("xTAG Type ('16'-ACC '17'-BCN '18'-TPH '19'-TH '32'-CO2): %d\r\n", ReadxTgSensorMetaRespBytes[3]);

	printf("xTAG Battery Count (0 - 255 denotes 2.2 - 3.0 Vdc): %d\r\n", ReadxTgSensorMetaRespBytes[4]);

	printf("xTAG Reserved Byte (0): %d\r\n", ReadxTgSensorMetaRespBytes[5]);

	U32 xTgRunTime = ((U32)ReadxTgSensorMetaRespBytes[6] << 24) & 0xFF000000;
	xTgRunTime += ((U32)ReadxTgSensorMetaRespBytes[7] << 16) & 0x00FF0000;
	xTgRunTime += ((U32)ReadxTgSensorMetaRespBytes[8] << 8) & 0x0000FF00;
	xTgRunTime += ((U32)ReadxTgSensorMetaRespBytes[9] << 0) & 0x000000FF;
	printf("Run time in sec: %d\r\n", xTgRunTime);

	U32 xTgUnixTime = ((U32)ReadxTgSensorMetaRespBytes[10] << 24) & 0xFF000000;
	xTgUnixTime += ((U32)ReadxTgSensorMetaRespBytes[11] << 16) & 0x00FF0000;
	xTgUnixTime += ((U32)ReadxTgSensorMetaRespBytes[12] << 8) & 0x0000FF00;
	xTgUnixTime += ((U32)ReadxTgSensorMetaRespBytes[13] << 0) & 0x000000FF;
	printf("Current xTAG Unix time in sec: %d\r\n", xTgUnixTime);

	U16 xTgHWRev = ((U16)ReadxTgSensorMetaRespBytes[14] << 8) & 0xFF00;
	xTgHWRev += ((U16)ReadxTgSensorMetaRespBytes[15] << 0) & 0x00FF;
	printf("xTAG Hardware Rev with 2 implied decimals: %d\r\n", xTgHWRev);

	U16 xTgSWRev = ((U16)ReadxTgSensorMetaRespBytes[16] << 8) & 0xFF00;
	xTgSWRev += ((U16)ReadxTgSensorMetaRespBytes[17] << 0) & 0x00FF;
	printf("xTAG Software Rev with 2 implied decimals: %d\r\n", xTgSWRev);
	printf("................................................................................\r\n");

	///////////////////////////////////////////////////////////////////////
	// Write Metadata to xTAG USB Sensor.
	// Re-execute read to validate any changed vars like Config Unixtime.
	///////////////////////////////////////////////////////////////////////

	// Indicate Write is starting.
	printf("Write xTAG USB Sensor Metadata ... ");

	// Setup cmd bytes.
	const U8 WritexTgMetaMsgLen = 0x0D;
	U8 WritexTgMetaCmdBytes[WritexTgMetaMsgLen];
	WritexTgMetaCmdBytes[0] = XTAGBLE_META_WRITE_CMD;
	WritexTgMetaCmdBytes[1] = WritexTgMetaMsgLen;

	U32 xTgCurrentUnixTime = CUtility::GetCurrentUnixTimeUTC();
	WritexTgMetaCmdBytes[2] = TestxTAGId[0];
	WritexTgMetaCmdBytes[3] = TestxTAGId[1];
	WritexTgMetaCmdBytes[4] = TestxTAGId[2];
	WritexTgMetaCmdBytes[5] = TestxTAGId[3];
	WritexTgMetaCmdBytes[6] = TestxTAGId[4];
	WritexTgMetaCmdBytes[7] = TestxTAGId[5];
	WritexTgMetaCmdBytes[8] = 0; // Reserved (SUP) byte must be 0;
	WritexTgMetaCmdBytes[9] = (U8)((xTgCurrentUnixTime >> 24) & 0x000000FF);	// Set Unix time.
	WritexTgMetaCmdBytes[10] = (U8)((xTgCurrentUnixTime >> 16) & 0x000000FF);	// Set Unix time.
	WritexTgMetaCmdBytes[11] = (U8)((xTgCurrentUnixTime >> 8) & 0x000000FF);	// Set Unix time.
	WritexTgMetaCmdBytes[12] = (U8)((xTgCurrentUnixTime >> 0) & 0x000000FF);	// Set Unix time.

	// Process the cmd and provide an array for response.
	U8 WriteTgSensorMetaRespBytes[PRIMARY_REC_BUFLEN];
	CmdExResult = CUtility::ProcCmdForResp(PrimeSkt, WritexTgMetaCmdBytes, WritexTgMetaMsgLen,
		WriteTgSensorMetaRespBytes, PRIMARY_REC_BUFLEN, 1000); // 1000 mSec timeout.

	// Handle gen error.
	if (CmdExResult == false)
	{
		printf("Error executing xTAG USB Vib test. xTAG Meta write cmd processing error.\r\n");
		return false;
	}

	// Validate response length and status byte.
	if ((WriteTgSensorMetaRespBytes[1] != 3) || (WriteTgSensorMetaRespBytes[2] != DW_SUCCESS))
	{
		printf("Error executing xTAG USB Vib test. xTAG Meta write response has bad length of %d instead of 3 or status %d instead of 0\r\n",
			WriteTgSensorMetaRespBytes[1], WriteTgSensorMetaRespBytes[2]);
		return false;
	}

	// Done
	printf("completed.\r\n");

	/////////////////////////////////////////////////////////
	// Read acc config data from xTAG USB Sensor.
	/////////////////////////////////////////////////////////

	printf("Read xTAG USB Sensor Acc Config Data ... ");

	// Setup cmd bytes.
	const U8 ReadxTgSensorAccCfgMsgLen = 8;
	U8 ReadxTgSensoAccCfgCmdBytes[] = { XTAG_VIB_CONFIG_READ_CMD, ReadxTgSensorAccCfgMsgLen,
		TestxTAGId[0], TestxTAGId[1], TestxTAGId[2], TestxTAGId[3], TestxTAGId[4], TestxTAGId[5] };	// Timeout in 5 sec.

	// Process the cmd and provide an array for response.
	U8 ReadxTgSensorAccCfgRespBytes[PRIMARY_REC_BUFLEN];
	CmdExResult = false;
	CmdExResult = CUtility::ProcCmdForResp(PrimeSkt, ReadxTgSensoAccCfgCmdBytes, ReadxTgSensorAccCfgMsgLen,
		ReadxTgSensorAccCfgRespBytes, PRIMARY_REC_BUFLEN, 1000); // 1000 mSec timeout.

	// Handle gen error.
	if (CmdExResult == false)
	{
		printf("Error executing xTAG USB Vib test. xTAG Acc Config read cmd processing error.\r\n");
		return false;
	}

	// Validate response length and status byte.
	if ((ReadxTgSensorAccCfgRespBytes[1] != 7) || (ReadxTgSensorAccCfgRespBytes[2] != DW_SUCCESS))
	{
		printf("Error executing xTAG USB Vib test. xTAG Acc Config read response has bad length of %d instead of 7 or status %d instead of 0\r\n",
			ReadxTgSensorAccCfgRespBytes[1], ReadxTgSensorAccCfgRespBytes[2]);
		return false;
	}

	// Done.
	printf("completed.\r\n");

	// Print Config.
	printf("xTAG USB Sensor Acc Config data.................................................\r\n");
	printf("xTAG Acc Range Byte ('3'-2G, '5'-4G, '8'-8G, '12'-16G): %d\r\n", ReadxTgSensorAccCfgRespBytes[3]);
	printf("xTAG Acc Samples/sec ('6'-25, '7'-50, '8'-100, '9'-200, '10'-400, '11'-800, '12'-1600): %d\r\n", ReadxTgSensorAccCfgRespBytes[4]);
	printf("xTAG Acc Oversample Byte ('0'-4x, '1'-2X, '2'-1X or No Oversample): %d\r\n", ReadxTgSensorAccCfgRespBytes[5]);
	printf("xTAG Acc Stream Seconds (Unused): %d\r\n", ReadxTgSensorAccCfgRespBytes[6]);
	printf("................................................................................\r\n");

	//////////////////////////////////////////
	// Write acc config data to xTAG USB Sensor.
	//////////////////////////////////////////

	// Indicate Write is starting.
	printf("Write xTAG USB Sensor Acc config data ... ");

	// Setup cmd bytes.
	const U8 WritexTgAccCfgMsgLen = 0x10;
	U8 WritexTgAccCfgCmdBytes[WritexTgAccCfgMsgLen];
	WritexTgAccCfgCmdBytes[0] = XTAG_VIB_CONFIG_WRITE_CMD;
	WritexTgAccCfgCmdBytes[1] = WritexTgAccCfgMsgLen;
	WritexTgAccCfgCmdBytes[2] = TestxTAGId[0];
	WritexTgAccCfgCmdBytes[3] = TestxTAGId[1];
	WritexTgAccCfgCmdBytes[4] = TestxTAGId[2];
	WritexTgAccCfgCmdBytes[5] = TestxTAGId[3];
	WritexTgAccCfgCmdBytes[6] = TestxTAGId[4];
	WritexTgAccCfgCmdBytes[7] = TestxTAGId[5];
	xTgCurrentUnixTime = CUtility::GetCurrentUnixTimeUTC();
	WritexTgMetaCmdBytes[8] = (U8)((xTgCurrentUnixTime >> 24) & 0x000000FF);	// Set Unix time.
	WritexTgMetaCmdBytes[9] = (U8)((xTgCurrentUnixTime >> 16) & 0x000000FF);	// Set Unix time.
	WritexTgMetaCmdBytes[10] = (U8)((xTgCurrentUnixTime >> 8) & 0x000000FF);	// Set Unix time.
	WritexTgMetaCmdBytes[11] = (U8)((xTgCurrentUnixTime >> 0) & 0x000000FF);	// Set Unix time.
	WritexTgAccCfgCmdBytes[12] = 0x05;	// 4G Range
	WritexTgAccCfgCmdBytes[13] = 0x07;	// 50 samples/sec
	WritexTgAccCfgCmdBytes[14] = 0x01;	// OverSample Rate 2x
	WritexTgAccCfgCmdBytes[15] = 0;		// Stream seconds not used in USB Sensors.

	// Process the cmd and provide an array for response.
	U8 WriteTgSensorAccRespBytes[PRIMARY_REC_BUFLEN];
	CmdExResult = CUtility::ProcCmdForResp(PrimeSkt, WritexTgAccCfgCmdBytes, WritexTgAccCfgMsgLen,
		WriteTgSensorAccRespBytes, PRIMARY_REC_BUFLEN, 1000); // 1000 mSec timeout.

	// Handle gen error.
	if (CmdExResult == false)
	{
		printf("Error executing xTAG USB Vib test. xTAG Acc config write cmd processing error.\r\n");
		return false;
	}

	// Validate response length and status byte.
	if ((WriteTgSensorMetaRespBytes[1] != 3) || (WriteTgSensorMetaRespBytes[2] != DW_SUCCESS))
	{
		printf("Error executing xTAG USB Vib test. xTAG Acc config write response has bad length of %d instead of 3 or status %d instead of 0\r\n",
			WriteTgSensorMetaRespBytes[1], WriteTgSensorMetaRespBytes[2]);
		return false;
	}

	// Done
	printf("completed.\r\n");

	//////////////////////////////////////////
	// Start xTAG USB Vib Sensor Streaming.
	//////////////////////////////////////////

	// Indicate Stream start cmd is being sent.
	// Provide a prompt giving the user a chance to cancel while waiting for data.
	printf("Will send xTAG USB Vib Sensor Acc stream start cmd in 4 sec and will display stream data.\r\n");
	printf("Hit spacebar to terminate this last step at any time.\r\n");

	Sleep(4000); // Give the user the chance to see the prompt above.

	// Setup cmd bytes.
	const U8 StartxTgSensorAccAcqMsgLen = 10;
	U8 StartxTgSensoAccAcqCmdBytes[] = { XTAG_VIB_STREAM_START_CMD, StartxTgSensorAccAcqMsgLen,
		TestxTAGId[0], TestxTAGId[1], TestxTAGId[2], TestxTAGId[3], TestxTAGId[4], TestxTAGId[5], 0, 0 };	// Timeout in 5 sec. No threash.

	// Process the cmd and provide an array for response.
	U8 StartxTgSensorAccAcqRespBytes[PRIMARY_REC_BUFLEN];
	CmdExResult = false;
	CmdExResult = CUtility::ProcCmdForResp(PrimeSkt, StartxTgSensoAccAcqCmdBytes, StartxTgSensorAccAcqMsgLen,
		StartxTgSensorAccAcqRespBytes, PRIMARY_REC_BUFLEN, 1000); // 1000 mSec timeout.

	// Handle gen error.
	if (CmdExResult == false)
	{
		printf("Error executing xTAG USB Vib test. xTAG Acc start acq cmd processing error.\r\n");
		return false;
	}

	// Validate response length and status byte.
	if ((StartxTgSensorAccAcqRespBytes[1] != 3) || (StartxTgSensorAccAcqRespBytes[2] != DW_SUCCESS))
	{
		printf("Error executing xTAG USB Vib test. xTAG Acc start acq response has bad length of %d instead of 3 or status %d instead of 0\r\n",
			StartxTgSensorAccAcqRespBytes[1], StartxTgSensorAccAcqRespBytes[2]);
		return false;
	}

	//////////////////////////////////////////
	// Stream xTAG USB Vib Sensor Acq Data.
	//////////////////////////////////////////

	U8 AccStreamBuf[STREAM_REC_BUFLEN];
	U32 NumBytesRecieved = 0;
	U32 MaxAccSampleRead = 9 + 100 * 6;
	U16 NumSamplesRecieved = 0;
	float floatDataX, floatDataY, floatDataZ;
	S16 rawDataX, rawDataY, rawDataZ;
	while (true)
	{
		// Look for a spacebar key press and exit accordingly.
		if (GetKeyState(VK_SPACE) & 0x8000) { break; }

		// Read into AccStreamBuf
		U32 NumBytesRecieved = (U32) recv(StreamSkt, (char*)AccStreamBuf, STREAM_REC_BUFLEN, 0);
		if (NumBytesRecieved == 0)
		{
			printf("Stream socket disconnected during xTAG USB Test.\n");	// Should never see this as we manage disconnect.
			return false;
		}
		else if (NumBytesRecieved < 0)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				Sleep(50);  // Give other threads some attention.
				continue;	// Try again.
			}
			else
			{
				printf("Socket recieve failed with error %d during xTAG USB Test\n", WSAGetLastError());
				return false;
			}
		}
		else if (NumBytesRecieved > 0)
		{
			// If there are backed up data (more bytes recieved than expected in 100 samples) just continue.
			if (NumBytesRecieved > MaxAccSampleRead) { continue; }
			else
			{
				// Discard xGATEWAY heartbeat messages that will come in now and then.
				if(AccStreamBuf[0] == XGWHLO_HBEAT_MSG) { continue; }

				// Confirm correct number of bytes recieved and stream msg type. 
				// Subtract 9 bytes for the header and then there should be a multiple of 6 bytes
				// for each sample... so modulus (%) 6 of sample bytes (after header bytes) should be 0.
				if ((((NumBytesRecieved - 9) % 6) != 0) || (AccStreamBuf[0] != XTAG_VIB_ACQ_STREAM_MSG))
				{
					printf("xTAG USB Vib sensor stream data size or type error. Got %d bytes of msg 0x%02X\r\n", NumBytesRecieved, AccStreamBuf[0]);
					return false;
				}

				// Calc NumSamplesRecieved
				NumSamplesRecieved = (NumBytesRecieved - 9) / 6;
					
				printf("Streamed %d xTAG Acc samples....................................................\r\n", NumSamplesRecieved);

				for (int x = 0; x < NumSamplesRecieved; x++)
				{
					// Get raw signed data (-32767 - 32767).
					rawDataX = ((S16)(AccStreamBuf[(x * 6) + 9]) << 0) & 0x00FF;	// LSByte
					rawDataX += ((S16)(AccStreamBuf[(x * 6) + 10]) << 8) & 0xFF00;	// MSByte
					rawDataY = ((S16)(AccStreamBuf[(x * 6) + 11]) << 0) & 0x00FF;	// LSByte
					rawDataY += ((S16)(AccStreamBuf[(x * 6) + 12]) << 8) & 0xFF00;	// MSByte
					rawDataZ = ((S16)(AccStreamBuf[(x * 6) + 13]) << 0) & 0x00FF;	// LSByte
					rawDataZ += ((S16)(AccStreamBuf[(x * 6) + 14]) << 8) & 0xFF00;	// MSByte

					// Get float by mult raw * percent of range.
					floatDataX = (((float)rawDataX) / 32767.0f) * 4;	// 4 G configured for this eg.
					floatDataY = (((float)rawDataY) / 32767.0f) * 4;	// 4 G configured for this eg.
					floatDataZ = (((float)rawDataZ) / 32767.0f) * 4;	// 4 G configured for this eg.

					//printf("xTAG Acc raw sample: x=%d y=%d z=%d\r\n", rawDataX, rawDataX, rawDataX);
					printf("xTAG Acc sample: x=%2.6f y=%2.6f z=%2.6f\r\n", floatDataX, floatDataY, floatDataZ);
				}
			}
		}
	} // while(true)

	//////////////////////////////////////////
	// Stop xTAG USB Vib Sensor Streaming.
	//////////////////////////////////////////

	// Indicate Write is starting.
	printf("Stopping xTAG USB Sensor Acc streaming ... ");

	// Setup cmd bytes.
	const U8 StopxTgAccAcqMsgLen = 0x08;
	U8 StopxTgAccAcqCmdBytes[StopxTgAccAcqMsgLen];
	StopxTgAccAcqCmdBytes[0] = XTAG_VIB_STREAM_STOP_CMD;
	StopxTgAccAcqCmdBytes[1] = StopxTgAccAcqMsgLen;
	StopxTgAccAcqCmdBytes[2] = TestxTAGId[0];
	StopxTgAccAcqCmdBytes[3] = TestxTAGId[1];
	StopxTgAccAcqCmdBytes[4] = TestxTAGId[2];
	StopxTgAccAcqCmdBytes[5] = TestxTAGId[3];
	StopxTgAccAcqCmdBytes[6] = TestxTAGId[4];
	StopxTgAccAcqCmdBytes[7] = TestxTAGId[5];

	// Process the cmd and provide an array for response.
	U8 StopTgSensorAccAcqBytes[PRIMARY_REC_BUFLEN];
	CmdExResult = CUtility::ProcCmdForResp(PrimeSkt, StopxTgAccAcqCmdBytes, StopxTgAccAcqMsgLen,
		StopTgSensorAccAcqBytes, PRIMARY_REC_BUFLEN, 1000); // 1000 mSec timeout.

	// Handle gen error.
	if (CmdExResult == false)
	{
		printf("Error executing xTAG USB Vib test. xTAG Acc config write cmd processing error.\r\n");
		return false;
	}

	// Validate response length and status byte.
	if ((StopTgSensorAccAcqBytes[1] != 3) || (StopTgSensorAccAcqBytes[2] != DW_SUCCESS))
	{
		printf("Error executing xTAG USB Vib test. xTAG Acc config write response has bad length of %d instead of 3 or status %d instead of 0\r\n",
			StopTgSensorAccAcqBytes[1], StopTgSensorAccAcqBytes[2]);
		return false;
	}

	// Done
	printf("completed.\r\n");

	return true;
}

bool CUSBTests::ExTAGEnvTest(SOCKET PrimeSkt, SOCKET StreamSkt)
{
	return true;
}

bool CUSBTests::ExTAGCo2Test(SOCKET PrimeSkt, SOCKET StreamSkt)
{
	return true;
}
