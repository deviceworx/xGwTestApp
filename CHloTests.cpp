#include "CHloTests.h"

CHloTests::CHloTests(void) { }
CHloTests::~CHloTests() { }

bool CHloTests::ExGATEWAYTest(SOCKET PrimeSkt, SOCKET StreamSkt)
{
	// Note that socket connections have been made before this method is called and it simply handles messaging over sockets.

	/////////////////////////////////////////////////////////
	// Read the xGATEWAY Metadata. 
	/////////////////////////////////////////////////////////

	// Indicate Read is starting.
	printf("xGw HaLow Read Metadata ...");

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
		printf("Error executing xGATEWAY HaLow test. Metadata Read cmd processing error.\r\n");
		return false;
	}

	// Validate response length and status byte.
	if ((MetaReadRespBytes[1] != 0x12) || (MetaReadRespBytes[2] != DW_SUCCESS))
	{
		printf("Error executing xGATEWAY HaLow test. Metadata Read response has bad length of %d instead of 18 or status %d instead of 0\r\n",
			MetaReadRespBytes[1], MetaReadRespBytes[2]);
		return false;
	}

	// Indicate Read is done.
	printf("completed.\r\n");

	// Display meta.
	printf("xGw Metadata ...\r\n");
	U32 xGwConfigUnixTime = ((U32)MetaReadRespBytes[3] << 24) & 0xFF000000;
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


	///////////////////////////////////////////////////////////////////////
	// Write an xGATEWAY Metadata update. 
	// Re-execute read to validate any changed vars like Config Unixtime.
	///////////////////////////////////////////////////////////////////////

	// Indicate Write is starting.
	printf("xGw Write Metadata ...");

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
		printf("Error executing xGATEWAY HaLow test. Metadata Write cmd processing error.\r\n");
		return false;
	}

	// Validate response length and status byte.
	if ((MetaWriteRespBytes[1] != 3) || (MetaWriteRespBytes[2] != DW_SUCCESS))
	{
		printf("Error executing xGATEWAY HaLow test. Metadata Write response has bad length of %d instead of 3 or status %d instead of 0\r\n",
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
			printf("Stream socket disconnected during xGATEWAY HaLow Test.\n");	// Should never see this as we manage disconnect.
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
				printf("Socket recieve failed with error %d during xGATEWAY HaLow Test\n", WSAGetLastError());
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
					printf("Socket recieved partial heartbeat with length %d during xGATEWAY HaLow Test\n", WSAGetLastError());
					return false;
				}

				// Print out heartbeat data.
				printf("xGw Heartbeat ...\r\n");

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
			}
		}
	} // while(true)

	return true;
}

bool CHloTests::ExTAGVibTest(SOCKET PrimeSkt, SOCKET StreamSkt)
{
	// Note that socket connections have been made before this method is called and it simply handles messaging over sockets.

	////////////////////////////////////////////////////////////
	// Read xTAG HaLow Metadata.
	////////////////////////////////////////////////////////////

	// We need the user to specify an xTAG ID - otherwise we will not be able to 
	// preset a heartbeat rate, etc. before the first heartbeat comes in (i.e. if
	// we simply react to the first xTAG's heartbeat). Scan in format AA:BB:CC:DD:EE:FF
	// with capital hex ID numbers.
	printf("Enter a valid xTAG ID for testing with capital hexadecimal numbers (e.g. 01:2A:0B:04:02:34):");
	U8 xTAG_ID[6];
	scanf_s("%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX", &(xTAG_ID[0]), &(xTAG_ID[1]), &(xTAG_ID[2]), &(xTAG_ID[3]), &(xTAG_ID[4]), &(xTAG_ID[5]));

	// Indicate Read is starting.
	printf("xTAG %02X:%02X:%02X:%02X:%02X:%02X HaLow Vib Read Metadata ... ",
		xTAG_ID[0], xTAG_ID[1], xTAG_ID[2], xTAG_ID[3], xTAG_ID[4], xTAG_ID[5]);

	// Setup cmd bytes.
	const U8 ReadxTgMetaMsgLen = 8;
	U8 ReadxTgMetaCmdBytes[] = { XGWHLO_XTAG_META_READ_CMD, ReadxTgMetaMsgLen,
		xTAG_ID[0], xTAG_ID[1], xTAG_ID[2], xTAG_ID[3], xTAG_ID[4], xTAG_ID[5] };

	// Process the cmd and provide an array for response.
	U8 RespBytes[PRIMARY_REC_BUFLEN];
	bool CmdExResult = false;
	CmdExResult = CUtility::ProcCmdForResp(PrimeSkt, ReadxTgMetaCmdBytes, ReadxTgMetaMsgLen, RespBytes, PRIMARY_REC_BUFLEN, 1000); // 1000 mSec timeout.

	// Handle gen error.
	if (CmdExResult == false)
	{
		printf("Error executing xTAG HaLow Vib test. Metadata Read cmd processing error.\r\n");
		return false;
	}

	// Validate response length.
	if ((RespBytes[1] != 12) && (RespBytes[1] != 3))
	{
		printf("Error executing xTAG HaLow Vib test. Metadata Read response has bad length of %d instead of 3 or 12\r\n", RespBytes[1]);
		return false;
	}

	// Check response status.
	if (RespBytes[2] == DW_ERR_CMD_PROCESS) { printf("New xTAG record will be created as nothing in DB... "); }
	else if (RespBytes[2] != DW_SUCCESS)
	{
		printf("Error executing xTAG HaLow Vib test. Metadata Read response has %d instead of 0\r\n", RespBytes[2]);
		return false;
	}

	// Indicate Read is done.
	printf("completed.\r\n");

	// Display meta.
	printf("xTg Metadata: ..................................................................\r\n");
	U32 xTgConfigUnixTime = ((U32)RespBytes[3] << 24) & 0xFF000000;
	xTgConfigUnixTime += ((U32)RespBytes[4] << 16) & 0x00FF0000;
	xTgConfigUnixTime += ((U32)RespBytes[5] << 8) & 0x0000FF00;
	xTgConfigUnixTime += ((U32)RespBytes[6] << 0) & 0x000000FF;
	printf("Config unix time: %d\r\n", xTgConfigUnixTime);
	printf("Type (must be 0x10): 0x%02X\r\n", RespBytes[7]);
	U8 TxPwr = RespBytes[8];
	printf("Tx Power: %d\r\n", TxPwr);
	U16 xTgHeartbeatFreq = ((U16)RespBytes[9] << 8) & 0xFF00;
	xTgHeartbeatFreq += ((U16)RespBytes[10] << 0) & 0x00FF;
	printf("Heartbeat Freq in min: %d\r\n", xTgHeartbeatFreq);
	U8 HBRetries = RespBytes[11];
	printf("Heartbeat Retries: %d\r\n", HBRetries);

	printf("................................................................................\r\n");

	// Replace some values for testing.
	TxPwr = xTAG_TEST_TXPOWER;
	xTgHeartbeatFreq = xTAG_TEST_HBPERIOD;
	HBRetries = xTAG_TEST_HBRETRIES;

	////////////////////////////////////////////////////////////
	// Write xTAG HaLow Metadata.
	////////////////////////////////////////////////////////////

	// Indicate Write is starting.
	printf("xTAG HaLow Vib Write Metadata ...");

	// Setup cmd bytes.
	const U8 WritexTgMetaMsgLen = 0x11;
	U8 WritexTgMetaCmdBytes[WritexTgMetaMsgLen];
	WritexTgMetaCmdBytes[0] = XGWHLO_XTAG_META_WRITE_CMD;
	WritexTgMetaCmdBytes[1] = WritexTgMetaMsgLen;
	WritexTgMetaCmdBytes[2] = xTAG_ID[0];
	WritexTgMetaCmdBytes[3] = xTAG_ID[1];
	WritexTgMetaCmdBytes[4] = xTAG_ID[2];
	WritexTgMetaCmdBytes[5] = xTAG_ID[3];
	WritexTgMetaCmdBytes[6] = xTAG_ID[4];
	WritexTgMetaCmdBytes[7] = xTAG_ID[5];
	xTgConfigUnixTime = CUtility::GetCurrentUnixTimeUTC();
	WritexTgMetaCmdBytes[8] = (U8)((xTgConfigUnixTime >> 24) & 0x000000FF);
	WritexTgMetaCmdBytes[9] = (U8)((xTgConfigUnixTime >> 16) & 0x000000FF);
	WritexTgMetaCmdBytes[10] = (U8)((xTgConfigUnixTime >> 8) & 0x000000FF);
	WritexTgMetaCmdBytes[11] = (U8)((xTgConfigUnixTime >> 0) & 0x000000FF);
	WritexTgMetaCmdBytes[12] = 0x10;	// Type always acc (0x10)
	WritexTgMetaCmdBytes[13] = TxPwr;									
	WritexTgMetaCmdBytes[14] = (U8)((xTgHeartbeatFreq >> 8) & 0x00FF);	
	WritexTgMetaCmdBytes[15] = (U8)((xTgHeartbeatFreq >> 0) & 0x00FF);
	WritexTgMetaCmdBytes[16] = HBRetries;								

	// Process the cmd and provide an array for response.
	CmdExResult = CUtility::ProcCmdForResp(PrimeSkt, WritexTgMetaCmdBytes, WritexTgMetaMsgLen, RespBytes, PRIMARY_REC_BUFLEN, 1000); // 1000 mSec timeout.

	// Handle gen error.
	if (CmdExResult == false)
	{
		printf("Error executing xTAG HaLow Vib test. Metadata Write cmd processing error.\r\n");
		return false;
	}

	// Validate response length and status byte.
	if ((RespBytes[1] != 3) || (RespBytes[2] != DW_SUCCESS))
	{
		printf("Error executing xTAG HaLow Vib test. Metadata Write response has bad length of %d instead of 3 or status %d instead of 0\r\n",
			RespBytes[1], RespBytes[2]);
		return false;
	}

	// Indicate Read is done.
	printf("completed.\r\n");
	printf("Updated some test values like TxPower:%d, Heartbeat Freq in min:%d and retries:%d\r\n",
		TxPwr, xTgHeartbeatFreq, HBRetries);
	printf("Updates applied when the next Heartbeat comes in from the xTAG (pre update values displayed in output above).\r\n");

	/////////////////////////////////////////////////////////
	// Read acc config data for xTAG HaLow Sensor.
	/////////////////////////////////////////////////////////

	printf("Read xTAG HaLow Sensor Acc Config Data from xGATEWAY ... ");

	// Setup cmd bytes.
	const U8 ReadxTgAccCfgMsgLen = 8;
	U8 ReadxTgAccCfgCmdBytes[] = { XTAG_VIB_CONFIG_READ_CMD, ReadxTgMetaMsgLen,
		xTAG_ID[0], xTAG_ID[1], xTAG_ID[2], xTAG_ID[3], xTAG_ID[4], xTAG_ID[5] };

	// Process the cmd and provide an array for response.
	CmdExResult = CUtility::ProcCmdForResp(PrimeSkt, ReadxTgAccCfgCmdBytes, ReadxTgAccCfgMsgLen, RespBytes, PRIMARY_REC_BUFLEN, 1000); // 1000 mSec timeout.

	// Handle gen error.
	if (CmdExResult == false)
	{
		printf("Error executing xTAG HaLow Vib test. Acc Config Read cmd processing error.\r\n");
		return false;
	}

	// Validate response length and status byte.
	if ((RespBytes[1] != 7) || (RespBytes[2] != DW_SUCCESS))
	{
		printf("Error executing xTAG HaLow Vib test. Acc Config Read response with bad length %d instead of 7 or status %d instead of 0\r\n", 
			RespBytes[1], RespBytes[2]);
		return false;
	}

	// Indicate Read is done.
	printf("completed.\r\n");

	// Print Config.
	printf("xTAG HaLow Sensor Acc Config data.................................................\r\n");
	printf("xTAG Acc Range Byte ('3'-2G, '5'-4G, '8'-8G, '12'-16G): %d\r\n", RespBytes[3]);
	printf("xTAG Acc Samples/sec ('6'-25, '7'-50, '8'-100, '9'-200, '10'-400, '11'-800, '12'-1600): %d\r\n", RespBytes[4]);
	printf("xTAG Acc Oversample Byte ('0'-4x, '1'-2X, '2'-1X or No Oversample): %d\r\n", RespBytes[5]);
	printf("xTAG Acc Stream Seconds: %d\r\n", RespBytes[6]);
	printf("................................................................................\r\n");

	/////////////////////////////////////////////////////////
	// Write acc config data for xTAG HaLow Sensor.
	/////////////////////////////////////////////////////////

	printf("Write xTAG HaLow Sensor Acc Config Data to xGATEWAY\r\n");
	printf("Will set range of 4G, 50 samples/s, 1x oversample and 5 sec duration ... ");

	// Setup cmd bytes.
	const U8 WritexTgAccCfgMsgLen = 0x10;
	U8 WritexTgAccCfgCmdBytes[WritexTgAccCfgMsgLen];
	WritexTgAccCfgCmdBytes[0] = XTAG_VIB_CONFIG_WRITE_CMD;
	WritexTgAccCfgCmdBytes[1] = WritexTgAccCfgMsgLen;
	WritexTgAccCfgCmdBytes[2] = xTAG_ID[0];
	WritexTgAccCfgCmdBytes[3] = xTAG_ID[1];
	WritexTgAccCfgCmdBytes[4] = xTAG_ID[2];
	WritexTgAccCfgCmdBytes[5] = xTAG_ID[3];
	WritexTgAccCfgCmdBytes[6] = xTAG_ID[4];
	WritexTgAccCfgCmdBytes[7] = xTAG_ID[5];
	xTgConfigUnixTime = CUtility::GetCurrentUnixTimeUTC();
	WritexTgAccCfgCmdBytes[8] = (U8)((xTgConfigUnixTime >> 24) & 0x000000FF);
	WritexTgAccCfgCmdBytes[9] = (U8)((xTgConfigUnixTime >> 16) & 0x000000FF);
	WritexTgAccCfgCmdBytes[10] = (U8)((xTgConfigUnixTime >> 8) & 0x000000FF);
	WritexTgAccCfgCmdBytes[11] = (U8)((xTgConfigUnixTime >> 0) & 0x000000FF);
	WritexTgAccCfgCmdBytes[12] = 5;	// GRange 4G
	WritexTgAccCfgCmdBytes[13] = 7;	// 50 samples/sec
	WritexTgAccCfgCmdBytes[14] = 2; // 1X Oversample
	WritexTgAccCfgCmdBytes[15] = 5;	// 5 sec.

	// Process the cmd and provide an array for response.
	CmdExResult = CUtility::ProcCmdForResp(PrimeSkt, WritexTgAccCfgCmdBytes, WritexTgAccCfgMsgLen, RespBytes, PRIMARY_REC_BUFLEN, 1000); // 1000 mSec timeout.

	// Handle gen error.
	if (CmdExResult == false)
	{
		printf("Error executing xTAG HaLow Vib test. Acc Config Write cmd processing error.\r\n");
		return false;
	}

	// Validate response length and status byte.
	if ((RespBytes[1] != 3) || (RespBytes[2] != DW_SUCCESS))
	{
		printf("Error executing xTAG HaLow Vib test. Acc Config Write response has bad length of %d instead of 3 or status %d instead of 0\r\n",
			RespBytes[1], RespBytes[2]);
		return false;
	}

	// Indicate Read is done.
	printf("completed.\r\n");

	/////////////////////////////////////////////////////////
	// Process xTAG Heartbeats and Vib Stream Data. 
	/////////////////////////////////////////////////////////

	// Indicate waiting for an xTAG to send a Heartbeat to the connected xGW.
	printf("Waiting on incoming xTAG Heartbeats and then acc data stream. Hit spacebar at any time to abort ...\r\n");

	U8 HbStreamBuf[STREAM_REC_BUFLEN];
	U32 NumBytesRecieved = 0;
	const U8 xTgHeartbeatLen = 0x1E;
	U32 MaxAccSampleRead = 9 + 100 * 6;
	U16 NumSamplesRecieved = 0;
	float floatDataX, floatDataY, floatDataZ;
	S16 rawDataX, rawDataY, rawDataZ;
	while (true)
	{
		// Look for a spacebar key press and exit accordingly.
		if (GetKeyState(VK_SPACE) & 0x8000) { break; }

		// Read into HbStreamBuf
		int NumBytesRecieved = recv(StreamSkt, (char*)HbStreamBuf, STREAM_REC_BUFLEN, 0);
		if (NumBytesRecieved == 0)
		{
			printf("Stream socket disconnected during xTAG HaLow Vib Test.\n");	// Should never see this as we manage disconnect.
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
				printf("Socket recieve failed with error %d during xTAG HaLow Vib Test\n", WSAGetLastError());
				return false;
			}
		}
		else if (NumBytesRecieved > 0)
		{
			// Check if message is a heartbeat.
			if (HbStreamBuf[0] == XTAGHLO_HBEAT_MSG)
			{
				// If there are backed up heartbeats or other messages (more bytes recieved than expected in 1 heartbeat) just continue.
				if (NumBytesRecieved > (U32)xTgHeartbeatLen) { continue; }

				// Confirm length
				if (NumBytesRecieved != (U32)xTgHeartbeatLen)
				{
					// Must be < xGwHeartbeatLen
					printf("Socket recieved partial heartbeat with length %d during xTAG HaLow Vib Test\n", WSAGetLastError());
					return false;
				}

				// Print out heartbeat data.
				printf("xTAG Heartbeat: ................................................................\r\n");

				printf("xTAG Type ('16'-Acc '17'-Beacon '18'-EnvTPH '19'-EnvTH '32'-CO2): %d\r\n", HbStreamBuf[3]);

				printf("xTAG ID: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
					xTAG_ID[0], xTAG_ID[1], xTAG_ID[2], xTAG_ID[3], xTAG_ID[4], xTAG_ID[5]);

				printf("xTAG battery count (0-255 for 2.2-3.0 Vdc):%d\r\n", HbStreamBuf[10]);

				S16 xTgRSSI = ((S16)HbStreamBuf[11] << 8) & 0xFF00;
				xTgRSSI += ((S16)HbStreamBuf[12] << 0) & 0x00FF;
				printf("xTAG RSSI: %d\r\n", xTgRSSI);

				U32 xTgRunTime = ((U32)HbStreamBuf[13] << 24) & 0xFF000000;
				xTgRunTime += ((U32)HbStreamBuf[14] << 16) & 0x00FF0000;
				xTgRunTime += ((U32)HbStreamBuf[15] << 8) & 0x0000FF00;
				xTgRunTime += ((U32)HbStreamBuf[16] << 0) & 0x000000FF;
				printf("xTAG Run time in sec: %d\r\n", xTgRunTime);

				U32 xTgUnixTime = ((U32)HbStreamBuf[17] << 24) & 0xFF000000;
				xTgUnixTime += ((U32)HbStreamBuf[18] << 16) & 0x00FF0000;
				xTgUnixTime += ((U32)HbStreamBuf[19] << 8) & 0x0000FF00;
				xTgUnixTime += ((U32)HbStreamBuf[20] << 0) & 0x000000FF;
				printf("Current xTAG Unix time in sec: %d\r\n", xTgUnixTime);

				U32 xTgCfgTime = ((U32)HbStreamBuf[21] << 24) & 0xFF000000;
				xTgCfgTime += ((U32)HbStreamBuf[22] << 16) & 0x00FF0000;
				xTgCfgTime += ((U32)HbStreamBuf[23] << 8) & 0x0000FF00;
				xTgCfgTime += ((U32)HbStreamBuf[24] << 0) & 0x000000FF;
				printf("Current xTAG Cfg time in sec: %d\r\n", xTgCfgTime);

				U16 xTgHWRev = ((U16)HbStreamBuf[25] << 8) & 0xFF00;
				xTgHWRev += ((U16)HbStreamBuf[26] << 0) & 0x00FF;
				printf("xTAG Hardware Rev with 2 implied decimals: %d\r\n", xTgHWRev);

				U16 xTgSWRev = ((U16)HbStreamBuf[27] << 8) & 0xFF00;
				xTgSWRev += ((U16)HbStreamBuf[28] << 0) & 0x00FF;
				printf("xTAG Software Rev with 2 implied decimals: %d\r\n", xTgSWRev);

				printf("xTAG Tx Power: %d\r\n", HbStreamBuf[29]);
				printf("................................................................................\r\n");

				printf("xTAG Updates applied from xGATEWAY Now! (seen in next Heartbeat).\r\n");
				printf("Waiting on incoming xTAG Heartbeats and/or acc stream data. Hit spacebar at any time to abort ...\r\n");
			}
			else if (HbStreamBuf[0] == XTAG_VIB_ACQ_STREAM_MSG)	// Acc data stream.
			{
				// If there are backed up data (more bytes recieved than expected in 100 samples) just continue.
				if ((U32) NumBytesRecieved > MaxAccSampleRead) { continue; }

				// Confirm correct number of bytes recieved and stream msg type. 
				// Subtract 9 bytes for the header and then there should be a multiple of 6 bytes
				// for each sample... so modulus (%) 6 of sample bytes (after header bytes) should be 0.
				if (((NumBytesRecieved - 9) % 6) != 0)
				{
					printf("xTAG HaLow Vib sensor stream data size or type error. Got %d bytes in msg\r\n", NumBytesRecieved);
					return false;
				}

				// Calc NumSamplesRecieved
				NumSamplesRecieved = (NumBytesRecieved - 9) / 6;

				printf("Streamed %d xTAG Acc samples....................................................\r\n", NumSamplesRecieved);

				for (int x = 0; x < NumSamplesRecieved; x++)
				{
					// Get raw signed data (-32767 - 32767).
					rawDataX = ((S16)(HbStreamBuf[(x * 6) + 9]) << 0) & 0x00FF;	// LSByte
					rawDataX += ((S16)(HbStreamBuf[(x * 6) + 10]) << 8) & 0xFF00;	// MSByte
					rawDataY = ((S16)(HbStreamBuf[(x * 6) + 11]) << 0) & 0x00FF;	// LSByte
					rawDataY += ((S16)(HbStreamBuf[(x * 6) + 12]) << 8) & 0xFF00;	// MSByte
					rawDataZ = ((S16)(HbStreamBuf[(x * 6) + 13]) << 0) & 0x00FF;	// LSByte
					rawDataZ += ((S16)(HbStreamBuf[(x * 6) + 14]) << 8) & 0xFF00;	// MSByte

					// Get float by mult raw * percent of range.
					floatDataX = (((float)rawDataX) / 32767.0f) * 4;	// 4 G configured for this eg.
					floatDataY = (((float)rawDataY) / 32767.0f) * 4;	// 4 G configured for this eg.
					floatDataZ = (((float)rawDataZ) / 32767.0f) * 4;	// 4 G configured for this eg.

					//printf("xTAG Acc raw sample: x=%d y=%d z=%d\r\n", rawDataX, rawDataX, rawDataX);
					printf("xTAG Acc sample: x = %d %2.6f y = %d %2.6f z = %d %2.6f\r\n", 
						rawDataX, floatDataX, rawDataY, floatDataY, rawDataZ, floatDataZ);
				}
				
			}
		}
	}


	return true;
}

bool CHloTests::ExTAGEnvTest(SOCKET PrimeSkt, SOCKET StreamSkt)
{
	// Note that socket connections have been made before this method is called and it simply handles messaging over sockets.

	////////////////////////////////////////////////////////////
	// Read xTAG HaLow Metadata.
	////////////////////////////////////////////////////////////

	// We need the user to specify an xTAG ID - otherwise we will not be able to 
	// preset a heartbeat rate, etc. before the first heartbeat comes in (i.e. if
	// we simply react to the first xTAG's heartbeat). Scan in format AA:BB:CC:DD:EE:FF
	// with capital hex ID numbers.
	printf("Enter a valid xTAG ID for testing with capital hexadecimal numbers (e.g. 01:2A:0B:04:02:34):");
	U8 xTAG_ID[6];
	scanf_s("%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX", &(xTAG_ID[0]), &(xTAG_ID[1]), &(xTAG_ID[2]), &(xTAG_ID[3]), &(xTAG_ID[4]), &(xTAG_ID[5]));

	// Indicate Read is starting.
	printf("xTAG %02X:%02X:%02X:%02X:%02X:%02X HaLow Env Read Metadata ... ",
		xTAG_ID[0], xTAG_ID[1], xTAG_ID[2], xTAG_ID[3], xTAG_ID[4], xTAG_ID[5]);

	// Setup cmd bytes.
	const U8 ReadxTgMetaMsgLen = 8;
	U8 ReadxTgMetaCmdBytes[] = { XGWHLO_XTAG_META_READ_CMD, ReadxTgMetaMsgLen,
		xTAG_ID[0], xTAG_ID[1], xTAG_ID[2], xTAG_ID[3], xTAG_ID[4], xTAG_ID[5] };

	// Process the cmd and provide an array for response.
	U8 RespBytes[PRIMARY_REC_BUFLEN];
	bool CmdExResult = false;
	CmdExResult = CUtility::ProcCmdForResp(PrimeSkt, ReadxTgMetaCmdBytes, ReadxTgMetaMsgLen, RespBytes, PRIMARY_REC_BUFLEN, 1000); // 1000 mSec timeout.

	// Handle gen error.
	if (CmdExResult == false)
	{
		printf("Error executing xTAG HaLow Env test. Metadata Read cmd processing error.\r\n");
		return false;
	}

	// Validate response length.
	if ((RespBytes[1] != 12) && (RespBytes[1] != 3))
	{
		printf("Error executing xTAG HaLow Env test. Metadata Read response has bad length of %d instead of 3 or 12\r\n", RespBytes[1]);
		return false;
	}

	// Check response status.
	if (RespBytes[2] == DW_ERR_CMD_PROCESS) { printf("New xTAG record will be created as nothing in DB... "); }
	else if (RespBytes[2] != DW_SUCCESS)
	{
		printf("Error executing xTAG HaLow Env test. Metadata Read response has %d instead of 0\r\n", RespBytes[2]);
		return false;
	}

	// Indicate Read is done.
	printf("completed.\r\n");

	// Display meta.
	printf("xTg Metadata: ..................................................................\r\n");
	U32 xTgConfigUnixTime = ((U32)RespBytes[3] << 24) & 0xFF000000;
	xTgConfigUnixTime += ((U32)RespBytes[4] << 16) & 0x00FF0000;
	xTgConfigUnixTime += ((U32)RespBytes[5] << 8) & 0x0000FF00;
	xTgConfigUnixTime += ((U32)RespBytes[6] << 0) & 0x000000FF;
	printf("Config unix time: %d\r\n", xTgConfigUnixTime);
	printf("Type (must be 0x12): 0x%02X\r\n", RespBytes[7]);
	U8 TxPwr = RespBytes[8];
	printf("Tx Power: %d\r\n", TxPwr);
	U16 xTgHeartbeatFreq = ((U16)RespBytes[9] << 8) & 0xFF00;
	xTgHeartbeatFreq += ((U16)RespBytes[10] << 0) & 0x00FF;
	printf("Heartbeat Freq in min: %d\r\n", xTgHeartbeatFreq);
	U8 HBRetries = RespBytes[11];
	printf("Heartbeat Retries: %d\r\n", HBRetries);

	printf("................................................................................\r\n");

	// Replace some values for testing.
	TxPwr = xTAG_TEST_TXPOWER;
	xTgHeartbeatFreq = xTAG_TEST_HBPERIOD;
	HBRetries = xTAG_TEST_HBRETRIES;

	////////////////////////////////////////////////////////////
	// Write xTAG HaLow Metadata.
	////////////////////////////////////////////////////////////

	// Indicate Write is starting.
	printf("xTAG HaLow Env Write Metadata ...");

	// Setup cmd bytes.
	const U8 WritexTgMetaMsgLen = 0x11;
	U8 WritexTgMetaCmdBytes[WritexTgMetaMsgLen];
	WritexTgMetaCmdBytes[0] = XGWHLO_XTAG_META_WRITE_CMD;
	WritexTgMetaCmdBytes[1] = WritexTgMetaMsgLen;
	WritexTgMetaCmdBytes[2] = xTAG_ID[0];
	WritexTgMetaCmdBytes[3] = xTAG_ID[1];
	WritexTgMetaCmdBytes[4] = xTAG_ID[2];
	WritexTgMetaCmdBytes[5] = xTAG_ID[3];
	WritexTgMetaCmdBytes[6] = xTAG_ID[4];
	WritexTgMetaCmdBytes[7] = xTAG_ID[5];
	xTgConfigUnixTime = CUtility::GetCurrentUnixTimeUTC();
	WritexTgMetaCmdBytes[8] = (U8)((xTgConfigUnixTime >> 24) & 0x000000FF);
	WritexTgMetaCmdBytes[9] = (U8)((xTgConfigUnixTime >> 16) & 0x000000FF);
	WritexTgMetaCmdBytes[10] = (U8)((xTgConfigUnixTime >> 8) & 0x000000FF);
	WritexTgMetaCmdBytes[11] = (U8)((xTgConfigUnixTime >> 0) & 0x000000FF);
	WritexTgMetaCmdBytes[12] = 0x12;	// Type always env (0x12)
	WritexTgMetaCmdBytes[13] = TxPwr;
	WritexTgMetaCmdBytes[14] = (U8)((xTgHeartbeatFreq >> 8) & 0x00FF);
	WritexTgMetaCmdBytes[15] = (U8)((xTgHeartbeatFreq >> 0) & 0x00FF);
	WritexTgMetaCmdBytes[16] = HBRetries;

	// Process the cmd and provide an array for response.
	CmdExResult = CUtility::ProcCmdForResp(PrimeSkt, WritexTgMetaCmdBytes, WritexTgMetaMsgLen, RespBytes, PRIMARY_REC_BUFLEN, 1000); // 1000 mSec timeout.

	// Handle gen error.
	if (CmdExResult == false)
	{
		printf("Error executing xTAG HaLow Env test. Metadata Write cmd processing error.\r\n");
		return false;
	}

	// Validate response length and status byte.
	if ((RespBytes[1] != 3) || (RespBytes[2] != DW_SUCCESS))
	{
		printf("Error executing xTAG HaLow Env test. Metadata Write response has bad length of %d instead of 3 or status %d instead of 0\r\n",
			RespBytes[1], RespBytes[2]);
		return false;
	}

	// Indicate Read is done.
	printf("completed.\r\n");
	printf("Updated some test values like TxPower:%d, Heartbeat Freq in min:%d and retries:%d\r\n",
		TxPwr, xTgHeartbeatFreq, HBRetries);
	printf("Updates applied when the next Heartbeat comes in from the xTAG (pre update values displayed in output above).\r\n");

	/////////////////////////////////////////////////////////
	// Read Env config data for xTAG HaLow Sensor.
	/////////////////////////////////////////////////////////

	printf("Read xTAG HaLow Sensor Env Config Data from xGATEWAY ... ");

	// Setup cmd bytes.
	const U8 ReadxTgEnvCfgMsgLen = 8;
	U8 ReadxTgEnvCfgCmdBytes[] = { XTAG_ENV688_CONFIG_READ_CMD, ReadxTgEnvCfgMsgLen,
		xTAG_ID[0], xTAG_ID[1], xTAG_ID[2], xTAG_ID[3], xTAG_ID[4], xTAG_ID[5] };

	// Process the cmd and provide an array for response.
	CmdExResult = CUtility::ProcCmdForResp(PrimeSkt, ReadxTgEnvCfgCmdBytes, ReadxTgEnvCfgMsgLen, RespBytes, PRIMARY_REC_BUFLEN, 1000); // 1000 mSec timeout.

	// Handle gen error.
	if (CmdExResult == false)
	{
		printf("Error executing xTAG HaLow Env test. Env Config Read cmd processing error.\r\n");
		return false;
	} 

	// Validate response length and status byte.
	if ((RespBytes[1] != 7) || (RespBytes[2] != DW_SUCCESS))
	{
		printf("Error executing xTAG HaLow Env test. Env Config Read response with bad length %d instead of 7 or status %d instead of 0\r\n",
			RespBytes[1], RespBytes[2]);
		return false;
	}

	// Indicate Read is done.
	printf("completed.\r\n");

	// Print Config.
	printf("xTAG HaLow Sensor Env Config data.................................................\r\n");
	printf("xTAG TP IIR Filter: %d\r\n", RespBytes[3]);
	printf("xTAG Temperature OSR: %d\r\n", RespBytes[4]);
	printf("xTAG Pressure OSR: %d\r\n", RespBytes[5]);
	printf("xTAG Humidity OSR: %d\r\n", RespBytes[6]);
	printf("................................................................................\r\n");

	/////////////////////////////////////////////////////////
	// Write Env config data for xTAG HaLow Sensor.
	/////////////////////////////////////////////////////////

	printf("Write xTAG HaLow Sensor Env Config Data to xGATEWAY\r\n");
	printf("Will set TP IIR of 0, Temp OSR of 1, Press OSR of 2 and Humidity OSR of 3 ... ");

	// Setup cmd bytes.
	const U8 WritexTgEnvCfgMsgLen = 0x10;
	U8 WritexTgEnvCfgCmdBytes[WritexTgEnvCfgMsgLen];
	WritexTgEnvCfgCmdBytes[0] = XTAG_ENV688_CONFIG_WRITE_CMD;
	WritexTgEnvCfgCmdBytes[1] = WritexTgEnvCfgMsgLen;
	WritexTgEnvCfgCmdBytes[2] = xTAG_ID[0];
	WritexTgEnvCfgCmdBytes[3] = xTAG_ID[1];
	WritexTgEnvCfgCmdBytes[4] = xTAG_ID[2];
	WritexTgEnvCfgCmdBytes[5] = xTAG_ID[3];
	WritexTgEnvCfgCmdBytes[6] = xTAG_ID[4];
	WritexTgEnvCfgCmdBytes[7] = xTAG_ID[5];
	xTgConfigUnixTime = CUtility::GetCurrentUnixTimeUTC();
	WritexTgEnvCfgCmdBytes[8] = (U8)((xTgConfigUnixTime >> 24) & 0x000000FF);
	WritexTgEnvCfgCmdBytes[9] = (U8)((xTgConfigUnixTime >> 16) & 0x000000FF);
	WritexTgEnvCfgCmdBytes[10] = (U8)((xTgConfigUnixTime >> 8) & 0x000000FF);
	WritexTgEnvCfgCmdBytes[11] = (U8)((xTgConfigUnixTime >> 0) & 0x000000FF);
	WritexTgEnvCfgCmdBytes[12] = 0;
	WritexTgEnvCfgCmdBytes[13] = 1;
	WritexTgEnvCfgCmdBytes[14] = 2;
	WritexTgEnvCfgCmdBytes[15] = 4;

	// Process the cmd and provide an array for response.
	CmdExResult = CUtility::ProcCmdForResp(PrimeSkt, WritexTgEnvCfgCmdBytes, WritexTgEnvCfgMsgLen, RespBytes, PRIMARY_REC_BUFLEN, 1000); // 1000 mSec timeout.

	// Handle gen error.
	if (CmdExResult == false)
	{
		printf("Error executing xTAG HaLow Env test. Env Config Write cmd processing error.\r\n");
		return false;
	}

	// Validate response length and status byte.
	if ((RespBytes[1] != 3) || (RespBytes[2] != DW_SUCCESS))
	{
		printf("Error executing xTAG HaLow Env test. Env Config Write response has bad length of %d instead of 3 or status %d instead of 0\r\n",
			RespBytes[1], RespBytes[2]);
		return false;
	}

	// Indicate Read is done.
	printf("completed.\r\n");

	/////////////////////////////////////////////////////////////////////
	// Signal xTAG HaLow Env Sensor to Start Sampling on next Heartbeat.
	/////////////////////////////////////////////////////////////////////

	printf("Send xGATEWAY a command to start env sampling every 60 sec.... ");

	// Setup cmd bytes.
	const U8 WritexTgEnvStartMsgLen = 0x0D;
	U8 WritexTgEnvStartCmdBytes[WritexTgEnvStartMsgLen];
	WritexTgEnvStartCmdBytes[0] = XTAG_ACQ_START_CMD;
	WritexTgEnvStartCmdBytes[1] = WritexTgEnvStartMsgLen;
	WritexTgEnvStartCmdBytes[2] = xTAG_ID[0];
	WritexTgEnvStartCmdBytes[3] = xTAG_ID[1];
	WritexTgEnvStartCmdBytes[4] = xTAG_ID[2];
	WritexTgEnvStartCmdBytes[5] = xTAG_ID[3];
	WritexTgEnvStartCmdBytes[6] = xTAG_ID[4];
	WritexTgEnvStartCmdBytes[7] = xTAG_ID[5];
	WritexTgEnvStartCmdBytes[8] = 0; // No shock event processing required.
	U32 SampleFreq = 60; // sec.
	WritexTgEnvStartCmdBytes[9] = (U8)((SampleFreq >> 24) & 0x000000FF);	// Overkill for small value, but shows process.
	WritexTgEnvStartCmdBytes[10] = (U8)((SampleFreq >> 16) & 0x000000FF);	// Overkill for small value, but shows process.
	WritexTgEnvStartCmdBytes[11] = (U8)((SampleFreq >> 8) & 0x000000FF);	// Overkill for small value, but shows process.
	WritexTgEnvStartCmdBytes[12] = (U8)((SampleFreq >> 0) & 0x000000FF);	// Overkill for small value, but shows process.

	// Process the cmd and provide an array for response.
	CmdExResult = CUtility::ProcCmdForResp(PrimeSkt, WritexTgEnvStartCmdBytes, WritexTgEnvStartMsgLen, RespBytes, PRIMARY_REC_BUFLEN, 1000); // 1000 mSec timeout.

	// Handle gen error.
	if (CmdExResult == false)
	{
		printf("Error executing xTAG HaLow Env test. Env sample start cmd processing error.\r\n");
		return false;
	}

	// Validate response length and status byte.
	if ((RespBytes[1] != 3) || (RespBytes[2] != DW_SUCCESS))
	{
		printf("Error executing xTAG HaLow Env test. Env start sampling response has bad length of %d instead of 3 or status %d instead of 0\r\n",
			RespBytes[1], RespBytes[2]);
		return false;
	}

	// Indicate Read is done.
	printf("completed.\r\n");

	// Indicate waiting for an xTAG to send a Heartbeat to the connected xGW.
	printf("Reading stored xTAG samples from xGATEWAY (after the are transferred from xTAGs). Hit spacebar at any time to abort ...\r\n");

	U16 NumSamples;
	while (true)
	{
		// Look for a spacebar key press and exit accordingly.
		if (GetKeyState(VK_SPACE) & 0x8000) { break; }

		// Sleep a sec
		Sleep(1000);

		// Send a command to read samples.
		U8 ReadSamplesCmdLen = 2;
		U8 ReadSamplesCmd[2] = { XTAG_ENV_ACQ_READ_CMD, ReadSamplesCmdLen };

		// Process the cmd and provide an array for response.
		CmdExResult = CUtility::ProcCmdForResp(PrimeSkt, ReadSamplesCmd, ReadSamplesCmdLen, RespBytes, PRIMARY_REC_BUFLEN, 1000); // 1000 mSec timeout.

		// Handle gen error.
		if (CmdExResult == false)
		{
			printf("Error executing xTAG HaLow Env test. Env sample read cmd processing error.\r\n");
			return false;
		}

		// Check status/error.
		if (RespBytes[2] != DW_SUCCESS)
		{
			printf("Error executing xTAG HaLow Env test. Env sample read cmd error response %d.\r\n", RespBytes[2]);
			return false;
		}

		// Check length to see of there are any samples. DW_SUCCESS can be returned with no samples if there are none.
		if (RespBytes[1] > 3)
		{
			// Get the number of samples as  total msg length minus header divided by 20 bytes per sammple.
			NumSamples = (RespBytes[1] - 3) / 20;

			printf("Read %d samples.\r\n", NumSamples);

			// Print sample data.
			for (int x = 0; x < NumSamples; x++)
			{
				printf("Sample for ID: %02X:%02X:%02X:%02X:%02X:%02X............................................\r\n", 
					RespBytes[(20 * x) + 3], RespBytes[(20 * x) + 4], RespBytes[(20 * x) + 5], 
					RespBytes[(20 * x) + 6], RespBytes[(20 * x) + 7], RespBytes[(20 * x) + 8]);

				U32 SampleTime = ((U32)RespBytes[(20 * x) + 9] << 24) & 0xFF000000;
				SampleTime += ((U32)RespBytes[(20 * x) + 10] << 16) & 0x00FF0000;
				SampleTime += ((U32)RespBytes[(20 * x) + 11] << 8) & 0x0000FF00;
				SampleTime += ((U32)RespBytes[(20 * x) + 12] << 0) & 0x000000FF;
				printf("Unix time: %d\r\n", SampleTime);

				S16 SampleTemp = ((S16)RespBytes[(20 * x) + 13] << 8) & 0xFF00;
				SampleTemp += ((S16)RespBytes[(20 * x) + 14] << 0) & 0x00FF;
				printf("Temp (2 imp decimals): %d\r\n", SampleTemp);

				U32 SamplePress = ((U32)RespBytes[(20 * x) + 15] << 24) & 0xFF000000;
				SamplePress += ((U32)RespBytes[(20 * x) + 16] << 16) & 0x00FF0000;
				SamplePress += ((U32)RespBytes[(20 * x) + 17] << 8) & 0x0000FF00;
				SamplePress += ((U32)RespBytes[(20 * x) + 18] << 0) & 0x000000FF;
				printf("Pressure (3 imp decimals): %d\r\n", SamplePress);

				U32 SampleHumid = ((U32)RespBytes[(20 * x) + 19] << 24) & 0xFF000000;
				SampleHumid += ((U32)RespBytes[(20 * x) + 20] << 16) & 0x00FF0000;
				SampleHumid += ((U32)RespBytes[(20 * x) + 21] << 8) & 0x0000FF00;
				SampleHumid += ((U32)RespBytes[(20 * x) + 22] << 0) & 0x000000FF;
				printf("Humidity (3 imp decimals): %d\r\n", SampleHumid);
				printf("................................................................................\r\n");
			}
		}
	}

	// Send a command to stop sampling.
	printf("Stopping sampling....");

	const U8 SampleStopCmdLen = 0x08;
	U8 SampleStopCmd[SampleStopCmdLen];
	SampleStopCmd[0] = XTAG_ACQ_STOP_CMD;
	SampleStopCmd[1] = SampleStopCmdLen;
	SampleStopCmd[2] = xTAG_ID[0];
	SampleStopCmd[3] = xTAG_ID[1];
	SampleStopCmd[4] = xTAG_ID[2];
	SampleStopCmd[5] = xTAG_ID[3];
	SampleStopCmd[6] = xTAG_ID[4];
	SampleStopCmd[7] = xTAG_ID[5];

	// Process the cmd and provide an array for response.
	CmdExResult = CUtility::ProcCmdForResp(PrimeSkt, SampleStopCmd, SampleStopCmdLen, RespBytes, PRIMARY_REC_BUFLEN, 1000); // 1000 mSec timeout.

	// Handle gen error.
	if (CmdExResult == false)
	{
		printf("Error executing xTAG HaLow Env test. Env sample stop cmd processing error.\r\n");
		return false;
	}

	// Validate response length and status byte.
	if ((RespBytes[1] != 3) || (RespBytes[2] != DW_SUCCESS))
	{
		printf("Error executing xTAG HaLow Env test. Env stop sampling response has bad length of %d instead of 3 or status %d instead of 0\r\n",
			RespBytes[1], RespBytes[2]);
		return false;
	}

	printf("completed.");

	return true;
}

bool CHloTests::ExTAGCo2Test(SOCKET PrimeSkt, SOCKET StreamSkt) // Implemented test and daemon code, but untested.
{
	// Note that socket connections have been made before this method is called and it simply handles messaging over sockets.

	////////////////////////////////////////////////////////////
	// Read xTAG HaLow Metadata.
	////////////////////////////////////////////////////////////

	// We need the user to specify an xTAG ID - otherwise we will not be able to 
	// preset a heartbeat rate, etc. before the first heartbeat comes in (i.e. if
	// we simply react to the first xTAG's heartbeat). Scan in format AA:BB:CC:DD:EE:FF
	// with capital hex ID numbers.
	printf("Enter a valid xTAG ID for testing with capital hexadecimal numbers (e.g. 01:2A:0B:04:02:34):");
	U8 xTAG_ID[6];
	scanf_s("%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX", &(xTAG_ID[0]), &(xTAG_ID[1]), &(xTAG_ID[2]), &(xTAG_ID[3]), &(xTAG_ID[4]), &(xTAG_ID[5]));

	// Indicate Read is starting.
	printf("xTAG %02X:%02X:%02X:%02X:%02X:%02X HaLow CO2 Read Metadata ... ",
		xTAG_ID[0], xTAG_ID[1], xTAG_ID[2], xTAG_ID[3], xTAG_ID[4], xTAG_ID[5]);

	// Setup cmd bytes.
	const U8 ReadxTgMetaMsgLen = 8;
	U8 ReadxTgMetaCmdBytes[] = { XGWHLO_XTAG_META_READ_CMD, ReadxTgMetaMsgLen,
		xTAG_ID[0], xTAG_ID[1], xTAG_ID[2], xTAG_ID[3], xTAG_ID[4], xTAG_ID[5] };

	// Process the cmd and provide an array for response.
	U8 RespBytes[PRIMARY_REC_BUFLEN];
	bool CmdExResult = false;
	CmdExResult = CUtility::ProcCmdForResp(PrimeSkt, ReadxTgMetaCmdBytes, ReadxTgMetaMsgLen, RespBytes, PRIMARY_REC_BUFLEN, 1000); // 1000 mSec timeout.

	// Handle gen error.
	if (CmdExResult == false)
	{
		printf("Error executing xTAG HaLow CO2 test. Metadata Read cmd processing error.\r\n");
		return false;
	}

	// Validate response length.
	if ((RespBytes[1] != 12) && (RespBytes[1] != 3))
	{
		printf("Error executing xTAG HaLow CO2 test. Metadata Read response has bad length of %d instead of 3 or 12\r\n", RespBytes[1]);
		return false;
	}

	// Check response status.
	if (RespBytes[2] == DW_ERR_CMD_PROCESS) { printf("New xTAG record will be created as nothing in DB... "); }
	else if (RespBytes[2] != DW_SUCCESS)
	{
		printf("Error executing xTAG HaLow CO2 test. Metadata Read response has %d instead of 0\r\n", RespBytes[2]);
		return false;
	}

	// Indicate Read is done.
	printf("completed.\r\n");

	// Display meta.
	printf("xTg Metadata: ..................................................................\r\n");
	U32 xTgConfigUnixTime = ((U32)RespBytes[3] << 24) & 0xFF000000;
	xTgConfigUnixTime += ((U32)RespBytes[4] << 16) & 0x00FF0000;
	xTgConfigUnixTime += ((U32)RespBytes[5] << 8) & 0x0000FF00;
	xTgConfigUnixTime += ((U32)RespBytes[6] << 0) & 0x000000FF;
	printf("Config unix time: %d\r\n", xTgConfigUnixTime);
	printf("Type (must be 0x12): 0x%02X\r\n", RespBytes[7]);
	U8 TxPwr = RespBytes[8];
	printf("Tx Power: %d\r\n", TxPwr);
	U16 xTgHeartbeatFreq = ((U16)RespBytes[9] << 8) & 0xFF00;
	xTgHeartbeatFreq += ((U16)RespBytes[10] << 0) & 0x00FF;
	printf("Heartbeat Freq in min: %d\r\n", xTgHeartbeatFreq);
	U8 HBRetries = RespBytes[11];
	printf("Heartbeat Retries: %d\r\n", HBRetries);

	printf("................................................................................\r\n");

	// Replace some values for testing.
	TxPwr = xTAG_TEST_TXPOWER;
	xTgHeartbeatFreq = xTAG_TEST_HBPERIOD;
	HBRetries = xTAG_TEST_HBRETRIES;

	////////////////////////////////////////////////////////////
	// Write xTAG HaLow Metadata.
	////////////////////////////////////////////////////////////

	// Indicate Write is starting.
	printf("xTAG HaLow CO2 Write Metadata ...");

	// Setup cmd bytes.
	const U8 WritexTgMetaMsgLen = 0x11;
	U8 WritexTgMetaCmdBytes[WritexTgMetaMsgLen];
	WritexTgMetaCmdBytes[0] = XGWHLO_XTAG_META_WRITE_CMD;
	WritexTgMetaCmdBytes[1] = WritexTgMetaMsgLen;
	WritexTgMetaCmdBytes[2] = xTAG_ID[0];
	WritexTgMetaCmdBytes[3] = xTAG_ID[1];
	WritexTgMetaCmdBytes[4] = xTAG_ID[2];
	WritexTgMetaCmdBytes[5] = xTAG_ID[3];
	WritexTgMetaCmdBytes[6] = xTAG_ID[4];
	WritexTgMetaCmdBytes[7] = xTAG_ID[5];
	xTgConfigUnixTime = CUtility::GetCurrentUnixTimeUTC();
	WritexTgMetaCmdBytes[8] = (U8)((xTgConfigUnixTime >> 24) & 0x000000FF);
	WritexTgMetaCmdBytes[9] = (U8)((xTgConfigUnixTime >> 16) & 0x000000FF);
	WritexTgMetaCmdBytes[10] = (U8)((xTgConfigUnixTime >> 8) & 0x000000FF);
	WritexTgMetaCmdBytes[11] = (U8)((xTgConfigUnixTime >> 0) & 0x000000FF);
	WritexTgMetaCmdBytes[12] = 0x12;	// Type always env (0x12)
	WritexTgMetaCmdBytes[13] = TxPwr;
	WritexTgMetaCmdBytes[14] = (U8)((xTgHeartbeatFreq >> 8) & 0x00FF);
	WritexTgMetaCmdBytes[15] = (U8)((xTgHeartbeatFreq >> 0) & 0x00FF);
	WritexTgMetaCmdBytes[16] = HBRetries;

	// Process the cmd and provide an array for response.
	CmdExResult = CUtility::ProcCmdForResp(PrimeSkt, WritexTgMetaCmdBytes, WritexTgMetaMsgLen, RespBytes, PRIMARY_REC_BUFLEN, 1000); // 1000 mSec timeout.

	// Handle gen error.
	if (CmdExResult == false)
	{
		printf("Error executing xTAG HaLow CO2 test. Metadata Write cmd processing error.\r\n");
		return false;
	}

	// Validate response length and status byte.
	if ((RespBytes[1] != 3) || (RespBytes[2] != DW_SUCCESS))
	{
		printf("Error executing xTAG HaLow Env test. Metadata Write response has bad length of %d instead of 3 or status %d instead of 0\r\n",
			RespBytes[1], RespBytes[2]);
		return false;
	}

	// Indicate Read is done.
	printf("completed.\r\n");
	printf("Updated some test values like TxPower:%d, Heartbeat Freq in min:%d and retries:%d\r\n",
		TxPwr, xTgHeartbeatFreq, HBRetries);
	printf("Updates applied when the next Heartbeat comes in from the xTAG (pre update values displayed in output above).\r\n");

	/////////////////////////////////////////////////////////
	// Read CO2 config data for xTAG HaLow Sensor.
	/////////////////////////////////////////////////////////

	printf("Read xTAG HaLow Sensor Env Config Data from xGATEWAY ... ");

	// Setup cmd bytes.
	const U8 ReadxTgEnvCfgMsgLen = 8;
	U8 ReadxTgEnvCfgCmdBytes[] = { XTAG_CO2_CONFIG_READ, ReadxTgEnvCfgMsgLen,
		xTAG_ID[0], xTAG_ID[1], xTAG_ID[2], xTAG_ID[3], xTAG_ID[4], xTAG_ID[5] };

	// Process the cmd and provide an array for response.
	CmdExResult = CUtility::ProcCmdForResp(PrimeSkt, ReadxTgEnvCfgCmdBytes, ReadxTgEnvCfgMsgLen, RespBytes, PRIMARY_REC_BUFLEN, 1000); // 1000 mSec timeout.

	// Handle gen error.
	if (CmdExResult == false)
	{
		printf("Error executing xTAG HaLow CO2 test. Env Config Read cmd processing error.\r\n");
		return false;
	}

	// Validate response length and status byte.
	if ((RespBytes[1] != 9) || (RespBytes[2] != DW_SUCCESS))
	{
		printf("Error executing xTAG HaLow CO2 test. Env Config Read response with bad length %d instead of 9 or status %d instead of 0\r\n",
			RespBytes[1], RespBytes[2]);
		return false;
	}

	// Indicate Read is done.
	printf("completed.\r\n");

	// Print Config.
	printf("xTAG HaLow Sensor CO2 Config data.................................................\r\n");
	U16 CO2Altitude = ((U16)RespBytes[3] << 8) & 0xFF00;
	CO2Altitude += ((U16)RespBytes[4] << 0) & 0x00FF;
	printf("xTAG Altitude in m: %d\r\n", CO2Altitude);
	U16 CO2Pressure = ((U16)RespBytes[5] << 8) & 0xFF00;
	CO2Pressure += ((U16)RespBytes[6] << 0) & 0x00FF;
	printf("xTAG Pressure in kPa (1 implied decimal): %d\r\n", CO2Pressure);
	S16 CO2TempOffset = ((S16)RespBytes[7] << 8) & 0xFF00;
	CO2TempOffset += ((S16)RespBytes[8] << 0) & 0x00FF;
	printf("xTAG Temperature Offset in deg C (3 implied decimals): %d\r\n", CO2TempOffset);
	printf("................................................................................\r\n");

	/////////////////////////////////////////////////////////
	// Write CO2 config data for xTAG HaLow Sensor.
	/////////////////////////////////////////////////////////

	printf("Write xTAG HaLow Sensor CO2 Config Data to xGATEWAY\r\n");
	printf("Will set Alt of 0, Pressure of 101.5 kpa and Temp offset of 0 ... ");
	CO2Altitude = 0;
	CO2Pressure = 1015;
	CO2TempOffset = 0;

	// Setup cmd bytes.
	const U8 WritexTgCO2CfgMsgLen = 0x12;
	U8 WritexTgCO2CfgCmdBytes[WritexTgCO2CfgMsgLen];
	WritexTgCO2CfgCmdBytes[0] = XTAG_CO2_CONFIG_READ;
	WritexTgCO2CfgCmdBytes[1] = WritexTgCO2CfgMsgLen;
	WritexTgCO2CfgCmdBytes[2] = xTAG_ID[0];
	WritexTgCO2CfgCmdBytes[3] = xTAG_ID[1];
	WritexTgCO2CfgCmdBytes[4] = xTAG_ID[2];
	WritexTgCO2CfgCmdBytes[5] = xTAG_ID[3];
	WritexTgCO2CfgCmdBytes[6] = xTAG_ID[4];
	WritexTgCO2CfgCmdBytes[7] = xTAG_ID[5];
	xTgConfigUnixTime = CUtility::GetCurrentUnixTimeUTC();
	WritexTgCO2CfgCmdBytes[8] = (U8)((xTgConfigUnixTime >> 24) & 0x000000FF);
	WritexTgCO2CfgCmdBytes[9] = (U8)((xTgConfigUnixTime >> 16) & 0x000000FF);
	WritexTgCO2CfgCmdBytes[10] = (U8)((xTgConfigUnixTime >> 8) & 0x000000FF);
	WritexTgCO2CfgCmdBytes[11] = (U8)((xTgConfigUnixTime >> 0) & 0x000000FF);
	WritexTgCO2CfgCmdBytes[12] = (U8)((CO2Altitude >> 8) & 0x00FF);
	WritexTgCO2CfgCmdBytes[13] = (U8)((CO2Altitude >> 0) & 0x00FF);
	WritexTgCO2CfgCmdBytes[14] = (U8)((CO2Pressure >> 8) & 0x00FF);
	WritexTgCO2CfgCmdBytes[15] = (U8)((CO2Pressure >> 0) & 0x00FF);
	WritexTgCO2CfgCmdBytes[16] = (U8)((CO2TempOffset >> 8) & 0x00FF);
	WritexTgCO2CfgCmdBytes[17] = (U8)((CO2TempOffset >> 0) & 0x00FF);

	// Process the cmd and provide an array for response.
	CmdExResult = CUtility::ProcCmdForResp(PrimeSkt, WritexTgCO2CfgCmdBytes, WritexTgCO2CfgMsgLen, RespBytes, PRIMARY_REC_BUFLEN, 1000); // 1000 mSec timeout.

	// Handle gen error.
	if (CmdExResult == false)
	{
		printf("Error executing xTAG HaLow Env test. CO2 Config Write cmd processing error.\r\n");
		return false;
	}

	// Validate response length and status byte.
	if ((RespBytes[1] != 3) || (RespBytes[2] != DW_SUCCESS))
	{
		printf("Error executing xTAG HaLow CO2 test. Env Config Write response has bad length of %d instead of 3 or status %d instead of 0\r\n",
			RespBytes[1], RespBytes[2]);
		return false;
	}

	// Indicate Read is done.
	printf("completed.\r\n");

	/////////////////////////////////////////////////////////////////////
	// Signal xTAG HaLow CO2 Sensor to Start Sampling on next Heartbeat.
	/////////////////////////////////////////////////////////////////////

	printf("Send xGATEWAY a command to start env sampling every 60 sec.... ");

	// Setup cmd bytes.
	const U8 WritexTgCO2StartMsgLen = 0x0D;
	U8 WritexTgCO2StartCmdBytes[WritexTgCO2StartMsgLen];
	WritexTgCO2StartCmdBytes[0] = XTAG_ACQ_START_CMD;
	WritexTgCO2StartCmdBytes[1] = WritexTgCO2StartMsgLen;
	WritexTgCO2StartCmdBytes[2] = xTAG_ID[0];
	WritexTgCO2StartCmdBytes[3] = xTAG_ID[1];
	WritexTgCO2StartCmdBytes[4] = xTAG_ID[2];
	WritexTgCO2StartCmdBytes[5] = xTAG_ID[3];
	WritexTgCO2StartCmdBytes[6] = xTAG_ID[4];
	WritexTgCO2StartCmdBytes[7] = xTAG_ID[5];
	WritexTgCO2StartCmdBytes[8] = 0; // No shock event processing required.
	U32 SampleFreq = 60; // sec.
	WritexTgCO2StartCmdBytes[9] = (U8)((SampleFreq >> 24) & 0x000000FF);	// Overkill for small value, but shows process.
	WritexTgCO2StartCmdBytes[10] = (U8)((SampleFreq >> 16) & 0x000000FF);	// Overkill for small value, but shows process.
	WritexTgCO2StartCmdBytes[11] = (U8)((SampleFreq >> 8) & 0x000000FF);	// Overkill for small value, but shows process.
	WritexTgCO2StartCmdBytes[12] = (U8)((SampleFreq >> 0) & 0x000000FF);	// Overkill for small value, but shows process.

	// Process the cmd and provide an array for response.
	CmdExResult = CUtility::ProcCmdForResp(PrimeSkt, WritexTgCO2StartCmdBytes, WritexTgCO2StartMsgLen, RespBytes, PRIMARY_REC_BUFLEN, 1000); // 1000 mSec timeout.

	// Handle gen error.
	if (CmdExResult == false)
	{
		printf("Error executing xTAG HaLow Env test. CO2 sample start cmd processing error.\r\n");
		return false;
	}

	// Validate response length and status byte.
	if ((RespBytes[1] != 3) || (RespBytes[2] != DW_SUCCESS))
	{
		printf("Error executing xTAG HaLow Env test. CO2 start sampling response has bad length of %d instead of 3 or status %d instead of 0\r\n",
			RespBytes[1], RespBytes[2]);
		return false;
	}

	// Indicate Read is done.
	printf("completed.\r\n");

	// Indicate waiting for an xTAG to send a Heartbeat to the connected xGW.
	printf("Reading stored xTAG samples from xGATEWAY (after the are transferred from xTAGs). Hit spacebar at any time to abort ...\r\n");

	U16 NumSamples;
	while (true)
	{
		// Look for a spacebar key press and exit accordingly.
		if (GetKeyState(VK_SPACE) & 0x8000) { break; }

		// Sleep a sec
		Sleep(1000);

		// Send a command to read samples.
		U8 ReadSamplesCmdLen = 2;
		U8 ReadSamplesCmd[2] = { XTAG_CO2_ACQ_READ_CMD, ReadSamplesCmdLen };

		// Process the cmd and provide an array for response.
		CmdExResult = CUtility::ProcCmdForResp(PrimeSkt, ReadSamplesCmd, ReadSamplesCmdLen, RespBytes, PRIMARY_REC_BUFLEN, 1000); // 1000 mSec timeout.

		// Handle gen error.
		if (CmdExResult == false)
		{
			printf("Error executing xTAG HaLow CO2 test. CO2 sample read cmd processing error.\r\n");
			return false;
		}

		// Check status/error.
		if (RespBytes[2] != DW_SUCCESS)
		{
			printf("Error executing xTAG HaLow CO2 test. CO2 sample read cmd error response %d.\r\n", RespBytes[2]);
			return false;
		}

		// Check length to see of there are any samples. DW_SUCCESS can be returned with no samples if there are none.
		if (RespBytes[1] > 3)
		{
			// Get the number of samples as  total msg length minus header divided by 20 bytes per sammple.
			NumSamples = (RespBytes[1] - 3) / 20;

			printf("Read %d samples.\r\n", NumSamples);

			// Print sample data.
			for (int x = 0; x < NumSamples; x++)
			{
				printf("Sample for ID: %02X:%02X:%02X:%02X:%02X:%02X............................................\r\n",
					RespBytes[(20 * x) + 3], RespBytes[(20 * x) + 4], RespBytes[(20 * x) + 5],
					RespBytes[(20 * x) + 6], RespBytes[(20 * x) + 7], RespBytes[(20 * x) + 8]);

				U32 SampleTime = ((U32)RespBytes[(20 * x) + 9] << 24) & 0xFF000000;
				SampleTime += ((U32)RespBytes[(20 * x) + 10] << 16) & 0x00FF0000;
				SampleTime += ((U32)RespBytes[(20 * x) + 11] << 8) & 0x0000FF00;
				SampleTime += ((U32)RespBytes[(20 * x) + 12] << 0) & 0x000000FF;
				printf("Unix time: %d\r\n", SampleTime);

				U16 SampleCO2 = ((U16)RespBytes[(20 * x) + 13] << 8) & 0xFF00;
				SampleCO2 += ((U16)RespBytes[(20 * x) + 14] << 0) & 0x00FF;
				printf("CO2 (ppm): %d\r\n", SampleCO2);

				S32 SampleTemp = ((S32)RespBytes[(20 * x) + 15] << 24) & 0xFF000000;
				SampleTemp += ((S32)RespBytes[(20 * x) + 16] << 16) & 0x00FF0000;
				SampleTemp += ((S32)RespBytes[(20 * x) + 17] << 8) & 0x0000FF00;
				SampleTemp += ((S32)RespBytes[(20 * x) + 18] << 0) & 0x000000FF;
				printf("Temperature (3 imp decimals): %d\r\n", SampleTemp);

				U32 SampleHumid = ((U32)RespBytes[(20 * x) + 19] << 24) & 0xFF000000;
				SampleHumid += ((U32)RespBytes[(20 * x) + 20] << 16) & 0x00FF0000;
				SampleHumid += ((U32)RespBytes[(20 * x) + 21] << 8) & 0x0000FF00;
				SampleHumid += ((U32)RespBytes[(20 * x) + 22] << 0) & 0x000000FF;
				printf("Humidity (3 imp decimals): %d\r\n", SampleHumid);
				printf("................................................................................\r\n");
			}
		}
	}

	// Send a command to stop sampling.
	printf("Stopping sampling....");

	const U8 SampleStopCmdLen = 0x08;
	U8 SampleStopCmd[SampleStopCmdLen];
	SampleStopCmd[0] = XTAG_ACQ_STOP_CMD;
	SampleStopCmd[1] = SampleStopCmdLen;
	SampleStopCmd[2] = xTAG_ID[0];
	SampleStopCmd[3] = xTAG_ID[1];
	SampleStopCmd[4] = xTAG_ID[2];
	SampleStopCmd[5] = xTAG_ID[3];
	SampleStopCmd[6] = xTAG_ID[4];
	SampleStopCmd[7] = xTAG_ID[5];

	// Process the cmd and provide an array for response.
	CmdExResult = CUtility::ProcCmdForResp(PrimeSkt, SampleStopCmd, SampleStopCmdLen, RespBytes, PRIMARY_REC_BUFLEN, 1000); // 1000 mSec timeout.

	// Handle gen error.
	if (CmdExResult == false)
	{
		printf("Error executing xTAG HaLow CO2 test. CO2 sample stop cmd processing error.\r\n");
		return false;
	}

	// Validate response length and status byte.
	if ((RespBytes[1] != 3) || (RespBytes[2] != DW_SUCCESS))
	{
		printf("Error executing xTAG HaLow CO2 test. CO2 stop sampling response has bad length of %d instead of 3 or status %d instead of 0\r\n",
			RespBytes[1], RespBytes[2]);
		return false;
	}

	printf("completed.");
	return true;
}

bool CHloTests::ExTAGFOTATest(SOCKET PrimeSkt, SOCKET StreamSkt)
{
	// Note that socket connections have been made before this method is called and it simply handles messaging over sockets.

	////////////////////////////////////////////////////////////
	// Read xTAG HaLow Metadata.
	////////////////////////////////////////////////////////////

	// We need the user to specify an xTAG ID - otherwise we will not be able to 
	// preset a heartbeat rate, etc. before the first heartbeat comes in (i.e. if
	// we simply react to the first xTAG's heartbeat). Scan in format AA:BB:CC:DD:EE:FF
	// with capital hex ID numbers.
	printf("Enter a valid xTAG ID for testing with capital hexadecimal numbers (e.g. 01:2A:0B:04:02:34):");
	U8 xTAG_ID[6];
	scanf_s("%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX", &(xTAG_ID[0]), &(xTAG_ID[1]), &(xTAG_ID[2]), &(xTAG_ID[3]), &(xTAG_ID[4]), &(xTAG_ID[5]));

	// Set the below to match the provided CRC for a text file. 
	U32 CRC = 0x7C68B969;
	U16 Rev = 10606;

	// Indicate test is starting.
	printf("xTAG %02X:%02X:%02X:%02X:%02X:%02X HaLow FOTA update. Will use xtaghlo_%05d-%06X.bin\r\n",
		xTAG_ID[0], xTAG_ID[1], xTAG_ID[2], xTAG_ID[3], xTAG_ID[4], xTAG_ID[5], Rev, CRC);

	// Setup cmd bytes.
	const U8 FOTAMsgLen = 0x0E;
	U8 FOTACmdBytes[FOTAMsgLen];
	FOTACmdBytes[0] = XGWHLO_XTAG_FOTA_CMD;
	FOTACmdBytes[1] = FOTAMsgLen;
	FOTACmdBytes[2] = xTAG_ID[0];
	FOTACmdBytes[3] = xTAG_ID[1];
	FOTACmdBytes[4] = xTAG_ID[2];
	FOTACmdBytes[5] = xTAG_ID[3];
	FOTACmdBytes[6] = xTAG_ID[4];
	FOTACmdBytes[7] = xTAG_ID[5];
	FOTACmdBytes[8] = (U8)((Rev >> 8) & 0x00FF);
	FOTACmdBytes[9] = (U8)((Rev >> 0) & 0x00FF);
	FOTACmdBytes[10] = (U8)((CRC >> 24) & 0x000000FF);
	FOTACmdBytes[11] = (U8)((CRC >> 16) & 0x000000FF);
	FOTACmdBytes[12] = (U8)((CRC >> 8) & 0x000000FF);
	FOTACmdBytes[13] = (U8)((CRC >> 0) & 0x000000FF);

	// Process the cmd and provide an array for response.
	// Proccessing of this cmd may require fetching approx 800kB from the DWorx cloud - taking 2-5 sec on a slower link. Use long timeout.
	U8 RespBytes[PRIMARY_REC_BUFLEN] = { 0 };
	bool CmdExResult = false;
	CmdExResult = CUtility::ProcCmdForResp(PrimeSkt, FOTACmdBytes, FOTAMsgLen, RespBytes, PRIMARY_REC_BUFLEN, 6000); // 6000 mSec timeout.

	// Handle gen error.
	if (CmdExResult == false)
	{
		printf("Error executing xTAG HaLow FOTA test. Write cmd processing error.\r\n");
		return false;
	}

	// Validate response length and status byte.
	if ((RespBytes[1] != 3) || (RespBytes[2] != DW_SUCCESS))
	{
		printf("Error executing xTAG HaLow FOTA test. Write response has bad length of %d instead of 3 or status %d instead of 0\r\n",
			RespBytes[1], RespBytes[2]);
		return false;
	}

	printf("xGATEWAY setup to update the selected xTAG upon its next check-in!\r\n");

	return true;
}
