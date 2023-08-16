// xGwTestApp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <time.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

// Se3lect testing for xtagusbd or xtagbled. MUST COMMENT OUT ONE OF THE DEFINES BELOW.
//#define XTAGBLED
//#define XTAGUSBD
#define SERIALD

#define STREAM_REC_BUFLEN           8192    // Default max for winsock. Set SO_SNDBUF for larger. Note xGw max is 200k.
#define PRIMARY_REC_BUFLEN          1024    // Cmd responses are smaller.

#ifdef XTAGBLED
#define DEFAULT_PRIME_PORT          "3240"
#define DEFAULT_STREAM_PORT         "3241"
#elif defined XTAGUSBD
#define DEFAULT_PRIME_PORT          "3242"
#define DEFAULT_STREAM_PORT         "3243"
#elif defined SERIALD
#define DEFAULT_PRIME_PORT          "3244"
#endif

#define DEFAULT_SERVER              "192.168.50.204"

#define SOCKET_RECV_SLEEP_MSEC    100

BYTE scannedXTAGArr[20][7];
int scannedxTAGCount = 0;

typedef struct ThreadData {
    SOCKET streamSocket;
    SOCKET primeSocket;
    UINT32  stopUnixTime;
    int* pNumSamples;
} THREADDATA, * PTHREADDATA;

// Return unix time in sec.
UINT32 getCurrentUnixTimeUTC(void)
{
    time_t curUnixTime;
    time(&curUnixTime);
    return (UINT32)curUnixTime;
}

bool procCmdForResp(SOCKET Sckt, char* pCmd, int cmdLen, char* pResp, int respCapacity, UINT32 timeOutmSec)
{
    // Clear out the response.
    memset(pResp, 0, respCapacity);

    int iRes = send(Sckt, pCmd, cmdLen, 0);
    if (iRes == SOCKET_ERROR) 
    {
        printf("send failed with error: %d\n", WSAGetLastError());
        return false; 
    }

    // printf("Bytes Sent: %ld\n", iRes);
    
    UINT32 funcTimeLeft = timeOutmSec;
    while (funcTimeLeft > 0)
    {
        // Non-block read will return immediatly if no bytes available.
        iRes = recv(Sckt, pResp, respCapacity, 0);

        if (iRes > 0)  // Read something. Safe to assome all bytes for each message will come in within internal timeout.
        {    
            // DEBUG
            printf("Cmd proc with %d mSec left. Resp: \n", funcTimeLeft);
            for (int x = 0; x < iRes; x++) { printf("%02X ", pResp[x] & 0x000000FF); }
            printf("\n");

            if (iRes > 8) // DEBUG looking for 1024 byte resp
            {
                printf("BIG BACK\n");
                return false;
            }

            return true;
        }
        else if (iRes == 0) 
        { 
            //printf("Connection closed in cmd proc.\n");
            return false;
        }
        else // < 0
        { 
            if(WSAGetLastError() == WSAEWOULDBLOCK) // Try again - unless timeout.
            {
                Sleep(SOCKET_RECV_SLEEP_MSEC);
                funcTimeLeft -= SOCKET_RECV_SLEEP_MSEC;
                continue;
            }
            else
            {
                printf("recv failed with error: %d\n", WSAGetLastError());
                return false;
            }
        }
    }
       
    // Timeout.
    printf("Socket timeout while processing cmd\n");
    return false;
}

DWORD WINAPI StreamThreadFunction(LPVOID lpParam)
{
    UINT8 recvbuf[STREAM_REC_BUFLEN];
    int numBytesToProc = 0, numBytesInMsg = 0;
    int numMsgs = 0;
    DWORD curTickCnt, prevTickCnt = 0;

    // Get thread data.
    PTHREADDATA pThreadDataIn = (PTHREADDATA)lpParam;
    SOCKET StreamSocket = pThreadDataIn->streamSocket;
    SOCKET PrimeSocket = pThreadDataIn->primeSocket;
    UINT32 UnixThreadStopTime = pThreadDataIn->stopUnixTime + 2;  // Stream stop cmds out at pThreadDataIn->stopUnixTime - wait for late coming stream msgs.
    int* pSampleCnt = pThreadDataIn->pNumSamples;

    // Only save (for now) for incoming data from this device.
    //UINT8 Id[] = { 0xEF, 0xAD, 0xCA, 0x75, 0x26, 0xBC };
    //UINT8 Id[] = { 0xCA, 0x29, 0xEF, 0x23, 0x84, 0x24 };
    //UINT8 Id[] = { 0xD0, 0xFD, 0x5D, 0x82, 0x98, 0x95 };
    //UINT8 Id[] = { 0xCC, 0xB5, 0x34, 0x50, 0x5A, 0x5A };
    //UINT8 Id[] = { 0xDE, 0x17, 0x61, 0x30, 0x6B, 0xCF };
    //UINT8 Id[] = { 0xF7, 0x34, 0x47, 0x2B, 0x7A, 0xBD };
    //UINT8 Id[] = { 0xF6, 0xD6, 0xFB, 0x3D, 0x5D, 0x0F };
    //UINT8 Id[] = { 0xF9, 0xEA, 0xCA, 0x25, 0x41, 0xC0 };
    //UINT8 Id[] = { 0xF1, 0xDB, 0x8A, 0xE3, 0x83, 0xEE };
    //UINT8 Id[] = { 0xDF, 0x36, 0x1D, 0x3F, 0x63, 0xC5 };
    UINT8 Id[] = { 0xDF4, 0x1C, 0x95, 0x1A, 0x0C, 0xCC };

    //printf("Starting thread with UnixStop in %d sec.\n", UnixThreadStopTime - getCurrentUnixTimeUTC());
    WCHAR fileName[64];
    swprintf_s(fileName, 64, L"%02X_%02X_%02X_%02X_%02X_%02X_%d_xgw.csv", Id[0], Id[1], Id[2], Id[3], Id[4], Id[5], getCurrentUnixTimeUTC());
    HANDLE csvHandle = CreateFile(fileName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (csvHandle == INVALID_HANDLE_VALUE)
    {
        printf("Error %d when creating a file. Cannot continue.\n", GetLastError());
        return -1;
    }

    float xFloat_GValue, yFloat_GValue,zFloat_GValue;
    WCHAR csvRowStr[64];
    DWORD dwBytesOut;

    // Reset.
    *pSampleCnt = 0;

    // Stop 2 sec after the end time (to support shutdown).
    while (getCurrentUnixTimeUTC() < UnixThreadStopTime)
    {
        // Read into recvbuf at position numBytesToProc - to support reading msgs in pieces.
        int iRes = recv(StreamSocket, (char*) &(recvbuf[numBytesToProc]), (size_t)(STREAM_REC_BUFLEN - numBytesToProc), 0);
        if (iRes == 0)
        {
            printf("Connection closure error during xTAG stream. Aborting thread.\n");
            return -1;
        }
        else if (iRes < 0)
        {
            if (WSAGetLastError() == WSAEWOULDBLOCK) // Try again - unless timeout.
            {
                Sleep(10);  // Give other threads some attention.
                continue;
            }
            else
            {
                printf("recv failed with error: %d\n", WSAGetLastError());
                continue;
            }
        }
        else if (iRes > 0)  // iRes Bytes read....
        {
            // Add the number of bytes to process - may have partial msg contents, so adding req'd.
            numBytesToProc += iRes;

            // If 2+ bytes read, we can get the number of expected bytes from the msg.
            if (numBytesToProc >= 2) { numBytesInMsg = recvbuf[1] & 0x000000FF; }
            else { continue; } // Need more bytes to be read before we can determine msg length.

            // If we have all the msg bytes (sometimes more), process.
            while (numBytesToProc >= numBytesInMsg)
            {
                // Handle cmd responses differently.
                switch (recvbuf[0])
                {
                    case 0x17:  // Stream data.
                    {
                        curTickCnt = GetTickCount();
                        if (recvbuf[2] == 0x00)
                        {
                            printf("%02d: Processing %d incoming stream bytes from %02X %02X %02X %02X %02X %02X\n", curTickCnt - prevTickCnt, numBytesInMsg,
                                recvbuf[3] & 0x000000FF, recvbuf[4] & 0x000000FF, recvbuf[5] & 0x000000FF,
                                recvbuf[6] & 0x000000FF, recvbuf[7] & 0x000000FF, recvbuf[8] & 0x000000FF);
                        }
                        else if(recvbuf[2] == 0x05)
                        {
                            printf("%02d: Plugged stream. %u bytes removed from %02X %02X %02X %02X %02X %02X\n", curTickCnt - prevTickCnt, recvbuf[9] & 0x000000FF,
                                recvbuf[3] & 0x000000FF, recvbuf[4] & 0x000000FF, recvbuf[5] & 0x000000FF,
                                recvbuf[6] & 0x000000FF, recvbuf[7] & 0x000000FF, recvbuf[8] & 0x000000FF);
                        }
                        else
                        {
                            printf("% 02d: Bad stream data status %d\n", curTickCnt - prevTickCnt, recvbuf[2]);
                            break;
                        }
                        prevTickCnt = curTickCnt;
                        /*
                        // Setup the cmd to stop - using the xTAG addr.
                        char sendCmd[] = { (char)0x18, (char)0x08, recvbuf[3], recvbuf[4], recvbuf[5], recvbuf[6], recvbuf[7], recvbuf[8] };
                        char recBuf[16];
                        procCmdForResp(PrimeSocket, sendCmd, (size_t)sendCmd[1], recBuf, (size_t)PRIMARY_REC_BUFLEN, 3000);
                        printf("Sending stream stop cmd to xTAG");

                        Sleep(3000);

                        char sendCmd2[] = { (char)0x16, (char)0x0A, recvbuf[3], recvbuf[4], recvbuf[5], recvbuf[6], recvbuf[7], recvbuf[8], (char)0x64, (char)0x00 };
                        procCmdForResp(PrimeSocket, sendCmd2, (size_t)sendCmd[1], recBuf, (size_t)PRIMARY_REC_BUFLEN, 3000);
                        printf("Sending stream start cmd to xTAG\n");
                        */
                        // TODO - When interested in which xTAG is the source, get the 6-byte ID from 
                        // recvbuf[msgStartIdx + 3] to recvbuf[msgStartIdx + 8] inclusive.

                        // TODO - if saving to a file for each sensor, open or create that file as required based on the incoming msg id.

                        // Loop through sample sets of 6 bytes each.
                        INT16 xRaw, yRaw, zRaw;
                        for (int x = 9; x < numBytesInMsg; x += 6)
                        {
                            xRaw = (recvbuf[x + 1] << 8) & 0xFF00;
                            xRaw += (recvbuf[x] << 0) & 0x00FF;
                            yRaw = (recvbuf[x + 3] << 8) & 0xFF00;
                            yRaw += (recvbuf[x + 2] << 0) & 0x00FF;
                            zRaw = (recvbuf[x + 5] << 8) & 0xFF00;
                            zRaw += (recvbuf[x + 4] << 0) & 0x00FF;

                            if ((xRaw == 0xFFFF) || (yRaw == 0xFFFF) || (zRaw == 0xFFFF))
                            {
                                printf("Bad raw data.\n");
                            }

                            // TODO - If interested in G values corresponding to raw values, divide them by 32767, cast to float and then 
                            // mult by the g range equivalent float value returned from the stream start command response G Range byte. 
                            // (e.g. G Range byte of 0x05 denotes range of -/+ 4g, so mult the division result by 4f).
                            // xFloat_GValue = ((float) (xRaw / 32767)) * 4f;

                            
                            xFloat_GValue = ((float) xRaw / 32767.0f) * 4.0f;
                            yFloat_GValue = ((float) yRaw / 32767.0f) * 4.0f;
                            zFloat_GValue = ((float) zRaw / 32767.0f) * 4.0f;

                            if (xRaw == yRaw)
                            {
                                printf("Match data %04X %04X %04X device %02X %02X %02X %02X %02X %02X\n", xRaw & 0x0000FFFF, yRaw & 0x0000FFFF, zRaw & 0x0000FFFF,
                                    recvbuf[3] & 0x000000FF, recvbuf[4] & 0x000000FF, recvbuf[5] & 0x000000FF,
                                    recvbuf[6] & 0x000000FF, recvbuf[7] & 0x000000FF, recvbuf[8] & 0x000000FF);
                            }
                            

                            // TODO - Write a sample row to a data file with the data in separate columns (ie. x, y and z columns in a csv file).
                            // For now, just write the the file if the Ids match that spec'd in source.
                            if ((recvbuf[3] == Id[0]) && (recvbuf[4] == Id[1]) && (recvbuf[5] == Id[2]) &&
                                (recvbuf[6] == Id[3]) && (recvbuf[7] == Id[4]) && (recvbuf[8] == Id[5]))
                            {
                                memset(csvRowStr, 0, (sizeof(WCHAR) * 64));
                                swprintf_s(csvRowStr, 64, L"%02.4f,%02.4f,%02.4f\r\n", xFloat_GValue, yFloat_GValue, zFloat_GValue);
                                size_t len = wcslen(csvRowStr);
                                WriteFile(
                                    csvHandle,            // Handle to the file
                                    csvRowStr,  // Buffer to write
                                    len * sizeof(WCHAR),   // Buffer size
                                    &dwBytesOut,    // Bytes written
                                    0L);         // Overlapped
                            }

                            // Count samples
                            *pSampleCnt += 1;
                        }
                        numMsgs++; // DEBUG
                        break;
                    }

                    default:

                        printf("Strange response during streaming. Cmd reflection byte: %02X\n", recvbuf[0] & 0x000000FF);
                        break;

                }   // switch (recvbuf[0])

                // If we get more than 1 complete msg ....
                if (numBytesToProc > numBytesInMsg)
                {
                    // DEBUG
                    // printf("Multi-msg. Response is %d bytes, just processed first %d byte msg\n", numBytesToProc, numBytesInMsg);

                    // Update remaining bytes to process.
                    numBytesToProc -= numBytesInMsg;

                    // Move bytes up to the start of recvbuf.
                    memcpy(&(recvbuf[0]), &(recvbuf[numBytesInMsg]), numBytesToProc);

                    // Confirm that there is at least 2 bytes remaining to proc.
                    // If so, update numBytesInMsg for next while (numBytesToProc >= numBytesInMsg) iteration.
                    if (numBytesToProc >= 2) { numBytesInMsg = recvbuf[1] & 0x000000FF; }
                    else {
                        break; 
                    } // Need more bytes.        
                }
                else // numBytesToProc == numBytesInMsg ... an exact msg processed.
                {
                    // Nothing left to process.
                    numBytesToProc = 0;
                    numBytesInMsg = 0;

                    // Clear out the recvbuf for cleanliness - not really req'd.
                    memset(recvbuf, 0, STREAM_REC_BUFLEN);

                    // Get out of the while loop as condition will be met when numBytesToProc == numBytesInMsg == 0
                    break;
                }
            }       // while (numBytesToProc >= numBytesInMsg)
        }           // else if (iRes > 0)
    }

    //printf("Stream thread timeout and shutdown.\n");

    CloseHandle(csvHandle);

    return 0;
}

int main()
{
	printf("xGATEWAY Test App Starting ...\n");

    if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL))
    {
        printf("Error setting thread priority. Error %d\n", GetLastError());
    }

    // Display thread priority
    int thrdPriority = GetThreadPriority(GetCurrentThread());
    if (thrdPriority == 0x0F) { printf("Thread priority bumped to realtime.\n");  }
    else { printf("Current thread priority is not real time, but rather 0x%02X\n", thrdPriority); }

    WSADATA wsaData;
    SOCKET PrimeConnectSocket = INVALID_SOCKET;
    SOCKET StreamConnectSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;
  
    char recvbuf[PRIMARY_REC_BUFLEN];
    int recvbufdatalen = 0;

    // Initialize Winsock
    int iResult;
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Prime socket connection below. Must be before stream socket connection that follows.
    //////////////////////////////////////////////////////////////////////////////////////////

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server prime address and port
    iResult = getaddrinfo(DEFAULT_SERVER, DEFAULT_PRIME_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo for prime server failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address on the prime server until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        // Create a SOCKET for connecting to server
        PrimeConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (PrimeConnectSocket == INVALID_SOCKET) {
            printf("Prime socket create failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect(PrimeConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(PrimeConnectSocket);
            PrimeConnectSocket = INVALID_SOCKET;
            continue;
        }

        printf("Primary socket connection completed.\n");
        break;
    }

    freeaddrinfo(result);

    if (PrimeConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to prime server!\n");
        WSACleanup();
        return 1;
    }

    // Setup the socket for non-blocking.
    u_long iMode = 1;
    iResult = ioctlsocket(PrimeConnectSocket, FIONBIO, &iMode);
    if (iResult != NO_ERROR) {
        printf("Unable to reset prime socket to non-blocking.\n");
        WSACleanup();
        return 1;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Stream socket connection below. Must be after prime socket connection preceeding.
    //////////////////////////////////////////////////////////////////////////////////////////

#if defined XTAGBLED || defined XTAGUSBD

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server stream address and port
    iResult = getaddrinfo(DEFAULT_SERVER, DEFAULT_STREAM_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo for stream server failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address on the prime server until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        // Create a SOCKET for connecting to server
        StreamConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (StreamConnectSocket == INVALID_SOCKET) {
            printf("Stream socket create failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect(StreamConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(StreamConnectSocket);
            StreamConnectSocket = INVALID_SOCKET;
            continue;
        }

        printf("Stream socket connection completed.\n");
        break;
    }

    freeaddrinfo(result);

    if (StreamConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to stream server!\n");
        WSACleanup();
        return 1;
    }

    // Setup the socket for non-blocking.
    iMode = 1;
    iResult = ioctlsocket(StreamConnectSocket, FIONBIO, &iMode);
    if (iResult != NO_ERROR) {
        printf("Unable to reset stream socket to non-blocking.\n");
        WSACleanup();
        return 1;
    }

#endif   

    char inChars[2];
    while (true)
    {
        printf("Enter test number or 'l' to list tests and 'x' to exit: ");
        scanf_s("%s", &inChars, 2);
        // printf("\r\n");

        // Check for eXit.
        if ((inChars[0] == 'x') || (inChars[0] == 'X')) 
        {
            closesocket(PrimeConnectSocket);
            closesocket(StreamConnectSocket);
            WSACleanup(); 
            return 0;  
        }

        if ((inChars[0] == 'l') || (inChars[0] == 'L')) 
        { 
            printf("1 - Shutdown Test.\r\n");
            printf("2 - Get GW Meta Test.\r\n");
            printf("3 - List xTAGs Test.\r\n");
            printf("4 - Connect listed xTAGs Test.\r\n");
            printf("5 - Disconnect connected xTAGs Test.\r\n");
            printf("6 - Get xTAG Meta Test.\r\n");
            printf("7 - Set xTAG Meta Test.\r\n");
            printf("8 - Calibrate Test.\r\n");
            printf("9 - Config xTAG Acc Test.\r\n");
            printf("A - 30 Sec Stream from Connected xTAGs Test.\r\n");
            printf("M - seraild Connect and Modbus Command Test.\r\n");
            continue;
        }

        // Check which test to run.
        switch (inChars[0])
        {
            case '1': // Test 1
            {
                // Send the shutdown cmd.
                char sendCmd[] = { (char)0xFF, (char)0x02 };
                if (!procCmdForResp(PrimeConnectSocket, sendCmd, (size_t)sendCmd[1], recvbuf, (size_t)PRIMARY_REC_BUFLEN, 1000))
                {
                    closesocket(PrimeConnectSocket);
                    closesocket(StreamConnectSocket);
                    WSACleanup();
                    return -1;
                }

                // Qualify return.
                if ((recvbuf[0] != (char)0xFF) || (recvbuf[1] != (char)0x03) || (recvbuf[2] != (char)0x00))
                {
                    printf("Bad response to shutdown cmd test. Cmd 0x%02X  Len 0x%02X Stat 0x%02X\n", recvbuf[0] & 0x000000FF, recvbuf[1] & 0x000000FF, recvbuf[2] & 0x000000FF);
                    break;
                }

                // Must shutdown as the xtagd is gone anyway.
                printf("xtagd shutdown - so this app will as well.\n");
                closesocket(PrimeConnectSocket);
                closesocket(StreamConnectSocket);
                WSACleanup();

                // Time for output ...
                Sleep(1000);
                
                return 0;
            }
            case '2': // Test 2
            {
                // Send the gw meta read cmd.
                char sendCmd[] = { (char)0x01, (char)0x02 };
                if (!procCmdForResp(PrimeConnectSocket, sendCmd, (size_t)sendCmd[1], recvbuf, (size_t)PRIMARY_REC_BUFLEN, 1000))
                {
                    closesocket(PrimeConnectSocket);
                    closesocket(StreamConnectSocket);
                    WSACleanup();
                    return -1;
                }

                // Qualify return.
                if ((recvbuf[0] != (char)0x01) || (recvbuf[1] != (char)0x09) || (recvbuf[2] != (char)0x00))
                {
                    printf("Bad response to xGW Meta Read cmd test. Cmd 0x%02X  Len 0x%02X Stat 0x%02X\n", recvbuf[0] & 0x000000FF, recvbuf[1] & 0x000000FF, recvbuf[2] & 0x000000FF);
                    break;
                }
               
                long runSec = (long)((recvbuf[3] << 24) & 0xFF000000);
                runSec += (long)((recvbuf[4] << 16) & 0x00FF0000);
                runSec += (long)((recvbuf[5] << 8) & 0x0000FF00);
                runSec += (long)(recvbuf[6] & 0x000000FF);

                int swVer = (int)((recvbuf[7] << 8) & 0xFF00);
                swVer += (int)(recvbuf[8] & 0x00FF);

                printf("Response. GW Run Sec: %d GW SW Ver: %d\n", runSec, swVer);
              
                break;
            }

            case '3': // Test 3
            {   
                // Long cmd proc required.
                printf("working ...\n");

                // Send the list xtags cmd.
                char sendCmd[] = { (char)0x02, (char)0x03, (char)0x0A }; // Test timeout 0x0F or 15 sec.
                if (!procCmdForResp(PrimeConnectSocket, sendCmd, (size_t)sendCmd[1], recvbuf, (size_t)PRIMARY_REC_BUFLEN, 21000))
                {
                    closesocket(PrimeConnectSocket);
                    closesocket(StreamConnectSocket);
                    WSACleanup();
                    return -1;
                }

                // Qualify return.
                if ((recvbuf[0] != (char)0x02) || (recvbuf[2] != (char)0x00))
                {
                    printf("Bad response to the list xtags cmd test. Cmd 0x%02X  Stat 0x%02X\n", recvbuf[0] & 0x000000FF, recvbuf[2] & 0x000000FF);
                    break;
                }
                
                // Store xtags. 
                // Remove 3-byte header and then divide payload by 6 (6 bytes per xTAG address).
                int payload_len = recvbuf[1] - 3;
                scannedxTAGCount = payload_len / 7;
                printf("List xTAGS responses has %d xtags.\n", scannedxTAGCount);

                // Copy scanned bytes into local storage.
                for (int x = 0; x < scannedxTAGCount; x++)
                {
                    scannedXTAGArr[x][0] = (BYTE)(recvbuf[(7 * x) + 3] & 0x000000FF);   // Connect stat (>0 for connect and 0 for disconnect)
                    scannedXTAGArr[x][1] = (BYTE)(recvbuf[(7 * x) + 4] & 0x000000FF);   // MS Addr byte
                    scannedXTAGArr[x][2] = (BYTE)(recvbuf[(7 * x) + 5] & 0x000000FF);
                    scannedXTAGArr[x][3] = (BYTE)(recvbuf[(7 * x) + 6] & 0x000000FF);
                    scannedXTAGArr[x][4] = (BYTE)(recvbuf[(7 * x) + 7] & 0x000000FF);
                    scannedXTAGArr[x][5] = (BYTE)(recvbuf[(7 * x) + 8] & 0x000000FF);
                    scannedXTAGArr[x][6] = (BYTE)(recvbuf[(7 * x) + 9] & 0x000000FF);   // LS Addr byte
                }

                // Print xtags. 
                char CONNECTSTRING[] = "Connected:Oui";     // Better alignment en francais
                char DISCONNECTSTRING[] = "Connected:Non";
                char* connstring;
                for (int x = 0; x < scannedxTAGCount; x++)
                {
                    if (scannedXTAGArr[x][0] > 0) { connstring = CONNECTSTRING; }
                    else { connstring = DISCONNECTSTRING; }

                    printf("%d: %s %02X:%02X:%02X:%02X:%02X:%02X\n", x, connstring,
                        scannedXTAGArr[x][1], scannedXTAGArr[x][2], scannedXTAGArr[x][3],
                        scannedXTAGArr[x][4], scannedXTAGArr[x][5], scannedXTAGArr[x][6]);
                }
                

                break; 
            }

            case '4': // Test 4
            {
                // Validate xTAGs scanned.
                if (scannedxTAGCount == 0) 
                { 
                    printf("No scanned xTAGs. Execute scan test first.\n"); 
                    break; 
                }

                printf("Enter the index of the xTAG to connect or b to go back to execute test 3 to list: ");
                scanf_s("%s", &inChars, 2);
                if((inChars[0] == 'b') || (inChars[0] == 'B')) { break; }

                // Validate correct idx
                inChars[1] = '\0';
                int idx = atoi(inChars);
                if (idx >= scannedxTAGCount)
                {
                    printf("Bad index provided. Re-execute test 3 to list.\n");
                    break;
                }

                // Long cmd proc required.
                printf("working ...\n");

                // Send the connect cmd for the xTAG.
                char sendCmd[] = {  (char)0x03, (char)0x08,  
                                    (char) scannedXTAGArr[idx][1], (char) scannedXTAGArr[idx][2], (char) scannedXTAGArr[idx][3],
                                    (char) scannedXTAGArr[idx][4], (char) scannedXTAGArr[idx][5], (char) scannedXTAGArr[idx][6]  };

                printf("Sending connect for xTAG at idx %d ... ", idx);
                if (!procCmdForResp(PrimeConnectSocket, sendCmd, (size_t)sendCmd[1], recvbuf, (size_t)PRIMARY_REC_BUFLEN, 20000))
                {
                    closesocket(PrimeConnectSocket);
                    closesocket(StreamConnectSocket);
                    WSACleanup();
                    return -1;
                }

                // Qualify return.
                if ((recvbuf[0] != (char)0x03) || (recvbuf[2] != (char)0x00))
                {
                    printf("Bad response to connect xtag cmd test. Cmd 0x%02X  Stat 0x%02X\n", recvbuf[0] & 0x000000FF, recvbuf[2] & 0x000000FF);
                    break;
                } 

                // Update the local status.
                scannedXTAGArr[idx][0] = 1;

                printf("done!\n");

                break;
            }

            case '5': // Test 5
            {
                // Validate xTAGs scanned.
                if (scannedxTAGCount == 0)
                {
                    printf("No scanned xTAGs. Execute scan test first.\n");
                    break;
                }

                printf("Enter the index of the xTAG to disconnect or b to go back to execute test 3 to list: ");
                scanf_s("%s", &inChars, 2);
                if ((inChars[0] == 'b') || (inChars[0] == 'B')) { break; }

                // Validate correct idx
                inChars[1] = '\0';
                int idx = atoi(inChars);
                if (idx >= scannedxTAGCount)
                {
                    printf("Bad index provided. Re-execute test 3 to list.\n");
                    break;
                }

                // Long cmd proc required.
                printf("working ...\n");

                // Send the disconnect cmd for the xTAG.
                char sendCmd[] = {  (char)0x04, (char)0x08,
                                    (char)scannedXTAGArr[idx][1], (char)scannedXTAGArr[idx][2], (char)scannedXTAGArr[idx][3],
                                    (char)scannedXTAGArr[idx][4], (char)scannedXTAGArr[idx][5], (char)scannedXTAGArr[idx][6] };

                printf("Sending disconnect for xTAG at idx %d ... ", idx);
                if (!procCmdForResp(PrimeConnectSocket, sendCmd, (size_t)sendCmd[1], recvbuf, (size_t)PRIMARY_REC_BUFLEN, 3000))
                {
                    closesocket(PrimeConnectSocket);
                    closesocket(StreamConnectSocket);
                    WSACleanup();
                    return -1;
                }

                printf("done!\n");

                // Update the local connection status for the xTAG.
                scannedXTAGArr[idx][0] = 0;

                break;
            }
            case '6': // Test 6
            {
                // Validate xTAGs scanned.
                if (scannedxTAGCount == 0)
                {
                    printf("No scanned xTAGs. Execute scan test first.\n");
                    break;
                }

                printf("Enter the index of the xTAG to read Meta data from or b to go back to execute test 3 to list: ");
                scanf_s("%s", &inChars, 2);
                if ((inChars[0] == 'b') || (inChars[0] == 'B')) { break; }

                // Validate correct idx
                inChars[1] = '\0';
                int idx = atoi(inChars);
                if (idx >= scannedxTAGCount)
                {
                    printf("Bad index provided. Re-execute test 3 to list.\n");
                    break;
                }

                // Long cmd proc required.
                printf("working ...\n");

                // Send the read xtag meta cmd for the indexed xTAG.
                char sendCmd[] = { (char)0x05, (char)0x08,
                                    (char)scannedXTAGArr[idx][1], (char)scannedXTAGArr[idx][2], (char)scannedXTAGArr[idx][3],
                                    (char)scannedXTAGArr[idx][4], (char)scannedXTAGArr[idx][5], (char)scannedXTAGArr[idx][6] };
                printf("Sending read meta cmd to xTAG at idx %d ... ", idx);
                if (!procCmdForResp(PrimeConnectSocket, sendCmd, (size_t)sendCmd[1], recvbuf, (size_t)PRIMARY_REC_BUFLEN, 2000))
                {
                    closesocket(PrimeConnectSocket);
                    closesocket(StreamConnectSocket);
                    WSACleanup();
                    return -1;
                }

                printf("done!\n");

                // Qualify return.
                if ((recvbuf[0] != (char)0x05) || (recvbuf[1] != (char)0x18) || (recvbuf[2] != (char)0x00))
                {
                    printf("Bad response to the read xtag meta data cmd test. Cmd 0x%02X Len 0x%02X Stat 0x%02X\n", recvbuf[0] & 0x000000FF, recvbuf[1] & 0x000000FF, recvbuf[2] & 0x000000FF);
                    break;
                }
                
                char typeStr[4] = { 0 };
                if (recvbuf[9] == 0x00) { sprintf_s(typeStr, 4, "Acc"); }
                else { sprintf_s(typeStr, 4, "???"); }

                int battVoltsRaw = 0;
                battVoltsRaw = battVoltsRaw | (recvbuf[10] & 0x000000FF);

                float battVolts = (((float) battVoltsRaw / 255.0f) * 0.8f) + 2.2f; // 0-255 = 2.2-3.0 Vdc.

                long runSec = (long)((recvbuf[12] << 24) & 0xFF000000);
                runSec += (long)((recvbuf[13] << 16) & 0x00FF0000);
                runSec += (long)((recvbuf[14] << 8) & 0x0000FF00);
                runSec += (long)(recvbuf[15] & 0x000000FF);

                long unixTime = (long)((recvbuf[16] << 24) & 0xFF000000);
                unixTime += (long)((recvbuf[17] << 16) & 0x00FF0000);
                unixTime += (long)((recvbuf[18] << 8) & 0x0000FF00);
                unixTime += (long)(recvbuf[19] & 0x000000FF);

                int hwVer = (int)((recvbuf[20] << 8) & 0xFF00);
                hwVer += (int)(recvbuf[21] & 0x00FF);

                int swVer = (int)((recvbuf[22] << 8) & 0xFF00);
                swVer += (int)(recvbuf[23] & 0x00FF);

                printf("Response. xTAG %02X%02X%02X%02X%02X%02X Type: %s Volts: %f SUP Byte 0x%02X Run Sec: %d Unix Time: %d HW Ver: %d GW SW Ver: %d\n",
                    recvbuf[3] & 0x000000FF, recvbuf[4] & 0x000000FF, recvbuf[5] & 0x000000FF,
                    recvbuf[6] & 0x000000FF, recvbuf[7] & 0x000000FF, recvbuf[8] & 0x000000FF,
                    typeStr, battVolts, (recvbuf[11] & 0x000000FF), runSec, unixTime, hwVer, swVer);

                break;
            }

            case '7': // Test 7
            {
                // Validate xTAGs scanned.
                if (scannedxTAGCount == 0)
                {
                    printf("No scanned xTAGs. Execute scan test first.\n");
                    break;
                }

                printf("Enter the index of the xTAG to write Meta data to or b to go back to execute test 3 to list: ");
                scanf_s("%s", &inChars, 2);
                if ((inChars[0] == 'b') || (inChars[0] == 'B')) { break; }

                // Validate correct idx
                inChars[1] = '\0';
                int idx = atoi(inChars);
                if (idx >= scannedxTAGCount)
                {
                    printf("Bad index provided. Re-execute test 3 to list.\n");
                    break;
                }

                // Get the user to provide a SUP byte in hex.
                unsigned int inSUP = 0;
                printf("Enter a hex SUP byte value (0 - ff) to write. This machines unix time will be used for writes: ");
                scanf_s("%x", &inSUP);
                char inSUPByte = (char)(inSUP & 0x000000FF);

                // Get current unix time.
                UINT32 inUnixTime = getCurrentUnixTimeUTC();

                // Long cmd proc required.
                printf("working ...\n");

                // Send the write xtag meta cmd for the indexed xTAG.
                char sendCmd[] = { (char)0x06, (char)0x0D,
                                    (char)scannedXTAGArr[idx][1], (char)scannedXTAGArr[idx][2], (char)scannedXTAGArr[idx][3],
                                    (char)scannedXTAGArr[idx][4], (char)scannedXTAGArr[idx][5], (char)scannedXTAGArr[idx][6],
                                    inSUPByte,  (char)(inUnixTime >> 24) & 0x000000FF, (char)(inUnixTime >> 16) & 0x000000FF,
                                                (char)(inUnixTime >> 8) & 0x000000FF, (char)(inUnixTime & 0x000000FF) };
                printf("Sending write meta cmd to xTAG at idx %d ... ", idx);
                if (!procCmdForResp(PrimeConnectSocket, sendCmd, (size_t)sendCmd[1], recvbuf, (size_t)PRIMARY_REC_BUFLEN, 2000))
                {
                    closesocket(PrimeConnectSocket);
                    closesocket(StreamConnectSocket);
                    WSACleanup();
                    return -1;
                }

                printf("done!\n");

                // Qualify return.
                if ((recvbuf[0] != (char)0x06) || (recvbuf[1] != (char)0x09) || (recvbuf[2] != (char)0x00))
                {
                    printf("Bad response to the write xtag meta data cmd test. Cmd 0x%02X Len 0x%02X Stat 0x%02X\n", recvbuf[0] & 0x000000FF, recvbuf[1] & 0x000000FF, recvbuf[2] & 0x000000FF);
                    break;
                }


                break;
            }

            case '8': // Test 8
            {
                // Validate xTAGs scanned.
                if (scannedxTAGCount == 0)
                {
                    printf("No scanned xTAGs. Execute scan test first.\n");
                    break;
                }

                printf("Enter the index of the xTAG to calibrate or b to go back to execute test 3 to list: ");
                scanf_s("%s", &inChars, 2);
                if ((inChars[0] == 'b') || (inChars[0] == 'B')) { break; }

                // Validate correct idx
                inChars[1] = '\0';
                int idx = atoi(inChars);
                if (idx >= scannedxTAGCount)
                {
                    printf("Bad index provided. Re-execute test 3 to list.\n");
                    break;
                }

                // Get the dim.
                char dimCmdByte = 0;
                char dimChar[2];
                printf("Enter a dim value (x, y or z): ");
                scanf_s("%s", &dimChar, 2);
                if ((dimChar[0] == 'x') || (dimChar[0] == 'X')) { dimCmdByte = 0x01; }
                else if ((dimChar[0] == 'y') || (dimChar[0] == 'Y')) { dimCmdByte = 0x02; }
                else if ((dimChar[0] == 'z') || (dimChar[0] == 'Z')) { dimCmdByte = 0x03; }
                else { printf("Bad dim provided. Will default to x.\n"); dimCmdByte = 0x01; }

                // Long cmd proc required.
                printf("working ...\n");

                // Send the caibrate start in dim X cmd for the indexed xTAG.
                char sendCmd[] = { (char)0x11, (char)0x09,
                                    (char)scannedXTAGArr[idx][1], (char)scannedXTAGArr[idx][2], (char)scannedXTAGArr[idx][3],
                                    (char)scannedXTAGArr[idx][4], (char)scannedXTAGArr[idx][5], (char)scannedXTAGArr[idx][6],
                                    dimCmdByte };

                if(dimCmdByte == 0x01 ) { printf("Sending calibrate cmd in dim x to xTAG at idx %d ... ", idx); }
                else if (dimCmdByte == 0x02) { printf("Sending calibrate cmd in dim y to xTAG at idx %d ... ", idx); }
                else { printf("Sending calibrate cmd in dim z to xTAG at idx %d ... ", idx); }

                if (!procCmdForResp(PrimeConnectSocket, sendCmd, (size_t)sendCmd[1], recvbuf, (size_t)PRIMARY_REC_BUFLEN, 2000))
                {
                    closesocket(PrimeConnectSocket);
                    closesocket(StreamConnectSocket);
                    WSACleanup();
                    return -1;
                }

                printf("done!\n");

                // Qualify return.
                if ((recvbuf[0] != (char)0x11) || (recvbuf[1] != (char)0x09) || (recvbuf[2] != (char)0x00))
                {
                    printf("Bad response to the caibrate xTAG in dim X cmd test. Cmd 0x%02X Len 0x%02X Stat 0x%02X\n", recvbuf[0] & 0x000000FF, recvbuf[1] & 0x000000FF, recvbuf[2] & 0x000000FF);
                    break;
                }


                break;
            }

            case '9': // Test 9
            {
                // Validate xTAGs scanned.
                if (scannedxTAGCount == 0)
                {
                    printf("No scanned xTAGs. Execute scan test first.\n");
                    break;
                }

                printf("Enter the index of the xTAG to config its acc or b to go back to execute test 3 to list: ");
                scanf_s("%s", &inChars, 2);
                if ((inChars[0] == 'b') || (inChars[0] == 'B')) { break; }

                // Validate correct idx
                inChars[1] = '\0';
                int idx = atoi(inChars);
                if (idx >= scannedxTAGCount)
                {
                    printf("Bad index provided. Re-execute test 3 to list.\n");
                    break;
                }
               
                // Get the user to a g range.
                unsigned int gRange = 0;
                char gRangeByte;
                /*
                printf("Enter a g range value (2, 4, 8 or 16): ");
                scanf_s("%d", &gRange);
                if (gRange == 2) { gRangeByte = 0x03; }
                else if (gRange == 4) { gRangeByte = 0x05; }
                else if (gRange == 8) { gRangeByte = 0x08; }
                else if (gRange == 16) { gRangeByte = 0x0C; }
                else { printf("Bad g range provided. Will default to 8.\n"); gRangeByte = 0x08; }
                */
                gRangeByte = 0x05; // Hardcode for now

                // Always use sample rate of 100 samples / sec (0x08)
                // Always use normal filter (0x02) 

                // Long cmd proc required.
                printf("working ...\n");

                // Send the write xtag meta cmd for the indexed xTAG.
                char sendCmd[] = { (char)0x14, (char)0x0B,
                                    (char)scannedXTAGArr[idx][1], (char)scannedXTAGArr[idx][2], (char)scannedXTAGArr[idx][3],
                                    (char)scannedXTAGArr[idx][4], (char)scannedXTAGArr[idx][5], (char)scannedXTAGArr[idx][6],
                                    gRangeByte, 0x08, 0x02 }; // Second last byte 0x0C for 1600 s/sec or 0x08 fo 100 s/sec.

                printf("Sending write acc config cmd to xTAG at idx %d ... ", idx);
                if (!procCmdForResp(PrimeConnectSocket, sendCmd, (size_t)sendCmd[1], recvbuf, (size_t)PRIMARY_REC_BUFLEN, 2000))
                {
                    closesocket(PrimeConnectSocket);
                    closesocket(StreamConnectSocket);
                    WSACleanup();
                    return -1;
                }

                printf("done!\n");

                // Qualify return.
                if ((recvbuf[0] != (char)0x14) || (recvbuf[1] != (char)0x09) || (recvbuf[2] != (char)0x00))
                {
                    printf("Bad response to the write xtag acc config data cmd test. Cmd 0x%02X Len 0x%02X Stat 0x%02X\n", recvbuf[0] & 0x000000FF, recvbuf[1] & 0x000000FF, recvbuf[2] & 0x000000FF);
                    break;
                }

                break;
            }

            case 'A': // Test A
            case 'a':
            {
                // Validate xTAGs scanned.
                if (scannedxTAGCount == 0)
                {
                    printf("No scanned xTAGs. Execute scan test first.\n");
                    break;
                }

                // While running, count incoming samples.
                int numSamples = 0;

                // Setup the end time for the test (sec away).
                UINT32 testEndUnixTime = getCurrentUnixTimeUTC() + 10; // x sec

                // Set data to pass to a thread.
                THREADDATA ThrdData;
                ThrdData.pNumSamples = &numSamples;
                ThrdData.stopUnixTime = testEndUnixTime;
                ThrdData.streamSocket = StreamConnectSocket;
                ThrdData.primeSocket = PrimeConnectSocket;

                // Start a thread that will watch for incoming stream packets on the stream socket.
                DWORD threadID;
                HANDLE threadHandle = CreateThread(
                    NULL,                   // default security attributes
                    0,                      // use default stack size  
                    StreamThreadFunction,       // thread function name
                    &ThrdData,          // argument to thread function 
                    0,                      // use default creation flags 
                    &threadID);   // returns the thread identifier 

                // Loop through scanned xTAGs and send a stream start to each.
                int connectedxtags = 0;
                for (int x = 0; x < scannedxTAGCount; x++)
                {
                    if (scannedXTAGArr[x][0] > 0) // If connected.
                    { 
                        connectedxtags++;

                        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                        // Select 1 of the 3 sendCmd[] options below for immediate stream start, start after On, start when off.
                        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                        // Setup the cmd to start - using the xTAG addr.
                        
                        char sendCmd[] = { (char)0x16, (char)0x0A,
                                    (char)scannedXTAGArr[x][1], (char)scannedXTAGArr[x][2], (char)scannedXTAGArr[x][3],
                                    (char)scannedXTAGArr[x][4], (char)scannedXTAGArr[x][5], (char)scannedXTAGArr[x][6],
                                    (char)0x00, (char)0x00 };   // No On or Off required - stream immediatly.
                        
                        /*
                        char sendCmd[] = { (char)0x16, (char)0x0A,
                                    (char)scannedXTAGArr[x][1], (char)scannedXTAGArr[x][2], (char)scannedXTAGArr[x][3],
                                    (char)scannedXTAGArr[x][4], (char)scannedXTAGArr[x][5], (char)scannedXTAGArr[x][6],
                                    (char)64, (char)0x00 };   // Stream when "On" with On threshold at 6.75% of G Range (32/255 * 0.5 * G Range)
                        */
                        /*
                        char sendCmd[] = { (char)0x16, (char)0x0A,
                                    (char)scannedXTAGArr[x][1], (char)scannedXTAGArr[x][2], (char)scannedXTAGArr[x][3],
                                    (char)scannedXTAGArr[x][4], (char)scannedXTAGArr[x][5], (char)scannedXTAGArr[x][6],
                                    (char)0x00, (char)52 };   // Stream when "Off" with On threshold at 10% of G Range (52/255 * 0.5 * G Range)
                        */
                        printf("Sending stream start cmd to xTAG at idx %d\n", x);
                        if (!procCmdForResp(PrimeConnectSocket, sendCmd, (size_t)sendCmd[1], recvbuf, (size_t)PRIMARY_REC_BUFLEN, 2000))
                        {
                            closesocket(PrimeConnectSocket);
                            closesocket(StreamConnectSocket);
                            WSACleanup();
                            return -1;
                        }

                        // Qualify return.
                        if ((recvbuf[0] != (char)0x16) || (recvbuf[1] != (char)0x0C) || (recvbuf[2] != (char)0x00))
                        {
                            printf("Bad response to the start xtag stream test for xTAG at idx %d. Cmd 0x%02X Len 0x%02X Stat 0x%02X\n",
                                x, recvbuf[0] & 0x000000FF, recvbuf[1] & 0x000000FF, recvbuf[2] & 0x000000FF);
                            break;
                        }

                        Sleep(50);     // Avoid multiple incoming cmds at once within xtagd - unsupported.
                    }
                }

                // Cannot continue if there were no connected xTAGs.
                if (connectedxtags == 0)
                {
                    printf("No connected xTAGs. Cannot run test.\n");
                    break;
                }

                // Send cmd to stop threading at testEndUnixTime. Wait here until then.
                // The thread will shut itself down at testEndUnixTime + 2 or when the stop cmd response is received.
                // The thread will stop getting stream msgs before it shuts down.
                while (getCurrentUnixTimeUTC() < testEndUnixTime) { Sleep(50); }  // Give other threads some attention.

                // Loop through scanned xTAGs and send a stream stop to each.
                for (int x = 0; x < scannedxTAGCount; x++)
                {
                    if (scannedXTAGArr[x][0] > 0) // If connected.
                    {
                        // Setup the cmd to stop - using the xTAG addr.
                        char sendCmd[] = { (char)0x18, (char)0x08,
                            (char)scannedXTAGArr[x][1], (char)scannedXTAGArr[x][2],
                            (char)scannedXTAGArr[x][3], (char)scannedXTAGArr[x][4],
                            (char)scannedXTAGArr[x][5], (char)scannedXTAGArr[x][6] };

                        printf("Sending stream stop cmd to xTAG at idx %d\n", x);
                        
                        if (!procCmdForResp(PrimeConnectSocket, sendCmd, (size_t)sendCmd[1], recvbuf, (size_t)PRIMARY_REC_BUFLEN, 3000))
                        {
                            closesocket(PrimeConnectSocket);
                            closesocket(StreamConnectSocket);
                            WSACleanup();
                            return -1;
                        }

                        // Qualify return.
                        if ((recvbuf[0] != (char)0x18) || (recvbuf[1] != (char)0x09) || (recvbuf[2] != (char)0x00))
                        {
                            printf("Bad response to the stop xtag stream test for xTAG at idx %d. Cmd 0x%02X Len 0x%02X Stat 0x%02X\n",
                                x, recvbuf[0] & 0x000000FF, recvbuf[1] & 0x000000FF, recvbuf[2] & 0x000000FF);
                        }
                        else { printf("Good response to the stop xtag stream test on primary port for xTAG at idx %d.\n", x); }
                        

                        Sleep(50);     // Avoid multiple incoming cmds at once within xtagd - unsupported.
                    }
                }

                // Wait the 2 sec for the thread to complete.
                UINT32 resultsOutUnixTime = getCurrentUnixTimeUTC() + 2;
                while (getCurrentUnixTimeUTC() < resultsOutUnixTime) { Sleep(50); }  // Give other threads some attention.

                // Send out the results.
                printf("Test A num samples from all xTAGs: %d\n", numSamples);

                break;
            }

            case 'M': // Test M
            case 'm':
            {
                // Send a command to setup the serial connection within seriald.
                char sendCmd[] = { (char)0x69, (char)0x88, (char)0x89,  // 'D','W','X' preamble.
                                    (char)8,                            // Baudrate index. See CPort.h.
                                    (char)8,                            // Data bits - 5 to 8
                                    (char)2,                            // Parity. 0-even, 1-odd, 2-none
                                    (char)0,                            // Stop bits. 0 for 1 stop bit, 1 for 1.5 or 2 stop bits(depending on dat bit length)
                                    (char)0,                            // Port index. 0 - 9 denoting /dev/ttymxc0 - /dev/ttymxc9
                                    (char)0x03, (char)0xE8 };           // Timeout 0x03E8 denotes 1000 mSec.

                printf("Sending seriald connect command\n");
                if (!procCmdForResp(PrimeConnectSocket, sendCmd, 10, recvbuf, (size_t)PRIMARY_REC_BUFLEN, 2000))
                {
                    printf("seriald connect error\n");
                    closesocket(PrimeConnectSocket);
                    WSACleanup();
                    return -1;
                }

                // Check response.
                if (recvbuf[0] != 0)
                {
                    printf("seriald connect response error. Returned %d\n", recvbuf[0]);
                    closesocket(PrimeConnectSocket);
                    WSACleanup();
                    return -1;
                }
                else { printf("Good seriald connect response. Send Modbus commands ....\n"); }

                // Loop through modbus cmds for scope review of timimg.
                for (int x = 0; x < 3700; x++)
                {
                    Sleep(950); // mSec

                    // Read Modbus 
                    // Send a cmd to read the first holding register. Delta using only holding registers.
                    //[01] [03] [00 00] [00 01] [crclsb crcmsb]. Get CRC from fcn in modbusd project or online here:
                    // https://crccalc.com/ when specifying the CRC-16/MODBUS option. Note that modbus device expects CRC ls byte (0xD5) first.
                    char modbuscmd[] = { 0x01, 0x03, 0x10, 0x01, 0x00, 0x01, 0xD1, 0x0A };
                    char fullCmdLen = sizeof(modbuscmd);    // With crc.

                    if (!procCmdForResp(PrimeConnectSocket, modbuscmd, (size_t)fullCmdLen, recvbuf, (size_t)PRIMARY_REC_BUFLEN, 2000))
                    {
                        printf("seriald modbus command processing error.\n");
                        closesocket(PrimeConnectSocket);
                        WSACleanup();
                        return -1;
                    }

                    if ((recvbuf[0] != 0x01) || (recvbuf[1] != 0x03) || (recvbuf[2] != 0x02) || (recvbuf[3] != 0x01))
                    {
                        printf("seriald modbus command bad resp.\n");
                        closesocket(PrimeConnectSocket);
                        WSACleanup();
                        return -1;
                    }

                    if ((recvbuf[0] != 0x01) || (recvbuf[1] != 0x03) || (recvbuf[2] != 0x02) || (recvbuf[3] != 0x01))
                    {
                        printf("seriald modbus command bad resp.\n");
                        closesocket(PrimeConnectSocket);
                        WSACleanup();
                        return -1;
                    }

                    printf("seriald modbus test resp %d.\n",x);

                }

                printf("seriald modbus test complete.\n");
            }

            default:

                printf("Invalid test num. Select 'l' to see valid numbers.\r\n");
                break;
        }
    }

    // cleanup
    closesocket(PrimeConnectSocket);
    closesocket(StreamConnectSocket);
    WSACleanup();
   
	return 0;
}

