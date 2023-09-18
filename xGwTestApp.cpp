// xGwTestApp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define WIN32_LEAN_AND_MEAN

// Commonly used headers.
#include <winsock2.h>
#include <time.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#include "Defines.h"
#include "CUtility.h"
#include "CUSBTests.h"
#include "CHloTests.h"

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

// Setup IP address of the target xGw.
#define DEFAULT_SERVER              "192.168.0.121"

enum class DeviceType { xGw, xTg };
enum class CommsType { USB, HLO};
enum class SenseType { Vib, Env, Co2 };

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

    // Get a test type (xGw or xTg)
    int inSel;
    DeviceType SelDeviceType = DeviceType::xGw; // Just a default selection.
    CommsType SelCommsType = CommsType::HLO;    // Just a default selection.
    SenseType SelSenseType = SenseType::Vib;    // Just a default selection.

    while (true)
    {
        printf("Select a test type.\r\n");
        printf("1 - xGATEWAY USB\r\n");
        printf("2 - xGATEWAY HaLow\r\n");
        printf("3 - xTAG USB Vibration Sensor\r\n");
        printf("4 - xTAG HaLow Vibration Sensor\r\n");
        printf("5 - xTAG USB Environmental Sensor\r\n");
        printf("6 - xTAG HaLow Environmental Sensor\r\n");
        printf("7 - xTAG USB CO2 Sensor\r\n");
        printf("8 - xTAG HaLow CO2 Sensor\r\n");
        printf("9 - xTAG HaLow FOTA (after ex 4, 6 or 8)\r\n");
        printf("Enter a selection or '0' to exit: ");
        scanf_s("%d", &inSel);

        // Check for eXit.
        if (inSel == 0) { return 0; }

        // Just set the device type.
        if(inSel == 1)
        {
            SelDeviceType = DeviceType::xGw;
            SelCommsType = CommsType::USB;
            break;
        }
        if(inSel == 2)
        {
            SelDeviceType = DeviceType::xGw;
            SelCommsType = CommsType::HLO;
            break;
        }
        if(inSel == 3)
        {
            SelDeviceType = DeviceType::xTg;
            SelCommsType = CommsType::USB;
            SelSenseType = SenseType::Vib;
            break;
        }
        if(inSel == 4)
        {
            SelDeviceType = DeviceType::xTg;
            SelCommsType = CommsType::HLO;
            SelSenseType = SenseType::Vib;
            break;
        }
        if(inSel == 5)
        {
            SelDeviceType = DeviceType::xTg;
            SelCommsType = CommsType::USB;
            SelSenseType = SenseType::Env;
            break;
        }
        if(inSel == 6)
        {
            SelDeviceType = DeviceType::xTg;
            SelCommsType = CommsType::HLO;
            SelSenseType = SenseType::Env;
            break;
        }
        if(inSel == 7)
        {
            SelDeviceType = DeviceType::xTg;
            SelCommsType = CommsType::USB;
            SelSenseType = SenseType::Co2;
            break;
        }
        if(inSel == 8)
        {
            SelDeviceType = DeviceType::xTg;
            SelCommsType = CommsType::HLO;
            SelSenseType = SenseType::Co2;
            break;
        }
        if (inSel == 9)
        {
            SelDeviceType = DeviceType::xTg;
            SelCommsType = CommsType::HLO;
            SelSenseType = SenseType::Vib;      // Same for all, but had to pick one to select ports.
            break;
        }
        else
        {
            printf("Invalid selection. Try again!\r\n");
            Sleep(2000);
            continue;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Winsock Setup.
    //////////////////////////////////////////////////////////////////////////////////////////

    WSADATA wsaData;
    SOCKET PrimeConnectSocket = INVALID_SOCKET;
    SOCKET StreamConnectSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;
  
    // char recvbuf[PRIMARY_REC_BUFLEN];
    int recvbufdatalen = 0;

    // Initialize Winsock
    int iResult;
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    // Set ports based on selected comms type.
    char PrimePortStr[5] = { 0 };
    char StreamPortStr[5] = { 0 };
    if (SelCommsType == CommsType::USB)
    {
        sprintf_s(PrimePortStr, "%d", 3242);
        sprintf_s(StreamPortStr, "%d", 3243);
    }
    else if(SelCommsType == CommsType::HLO)
    {
        sprintf_s(PrimePortStr, "%d", 3244);
        sprintf_s(StreamPortStr, "%d", 3245);
    }
    else
    {
        printf("Invalid comms type enum: %d\n", SelCommsType);
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
    iResult = getaddrinfo(DEFAULT_SERVER, PrimePortStr, &hints, &result);
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

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server stream address and port
    iResult = getaddrinfo(DEFAULT_SERVER, StreamPortStr, &hints, &result);
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

    // Pass sockets into specific test methods below. 
    // Note that test method code overlaps (i.e. the same code used in many tests, which is very inefficient)
    // This is intentional to support a simpler code reference.

    if(SelDeviceType == DeviceType::xGw)
    {
        if (SelCommsType == CommsType::USB)
        {
            if (CUSBTests::ExGATEWAYTest(PrimeConnectSocket, StreamConnectSocket))
            { printf("Successfully completed xGw USB test!\n"); }
            else
            { printf("xGw USB test failed!\n"); }
        }
        else if(SelCommsType == CommsType::HLO)
        {
            if (CHloTests::ExGATEWAYTest(PrimeConnectSocket, StreamConnectSocket))
            { printf("Successfully completed xGw HaLow test!\n"); }
            else
            { printf("xGw HaLow test failed!\n"); }
        }
        else { printf("Somehow, invalid comms type set. Test failed.\n"); }
    }
    else if (SelDeviceType == DeviceType::xTg)
    {
        if (SelCommsType == CommsType::USB)
        {
            if (SelSenseType == SenseType::Vib)
            {
                if (CUSBTests::ExTAGVibTest(PrimeConnectSocket, StreamConnectSocket))
                { printf("Successfully completed xTg USB Vibration test!\n"); }
                else
                { printf("xTg USB Vibration test failed!\n"); }
            }
            else if (SelSenseType == SenseType::Env)
            {
                if (CUSBTests::ExTAGEnvTest(PrimeConnectSocket, StreamConnectSocket))
                { printf("Successfully completed xTg USB Environmental test!\n"); }
                else
                { printf("xTg USB Environmental test failed!\n"); }
            }
            else if (SelSenseType == SenseType::Co2)
            {
                if (CUSBTests::ExTAGCo2Test(PrimeConnectSocket, StreamConnectSocket))
                { printf("Successfully completed xTg USB Co2 test!\n"); }
                else
                { printf("xTg USB Co2 test failed!\n"); }
            }
            else { printf("Somehow, invalid sensor type set. xTAG USB Test failed.\n"); }
        }
        else if (SelCommsType == CommsType::HLO)
        {
            // If inSel == 9, just execute a FOTA test.
            if (inSel == 9)
            {
                if (CHloTests::ExTAGFOTATest(PrimeConnectSocket, StreamConnectSocket))
                { printf("Successfully completed xTg HaLow FOTA test!\n"); }
                else
                { printf("xTg HaLow FOTA test failed!\n"); }
            }
            else if (SelSenseType == SenseType::Vib)
            {
                if (CHloTests::ExTAGVibTest(PrimeConnectSocket, StreamConnectSocket))
                { printf("Successfully completed xTg HaLow Vibration test!\n"); }
                else
                { printf("xTg HaLow Vibration test failed!\n"); }
            }
            else if (SelSenseType == SenseType::Env)
            {
                if (CHloTests::ExTAGEnvTest(PrimeConnectSocket, StreamConnectSocket))
                { printf("Successfully completed xTg HaLow Environmental test!\n"); }
                else
                { printf("xTg HaLow Environmental test failed!\n"); }
            }
            else if (SelSenseType == SenseType::Co2)
            {
                if (CHloTests::ExTAGCo2Test(PrimeConnectSocket, StreamConnectSocket))
                { printf("Successfully completed xTg HaLow Co2 test!\n"); }
                else
                { printf("xTg HaLow Co2 test failed!\n"); }
            }
            else { printf("Somehow, invalid sensor type set. xTAG HaLow Test failed.\n"); }
        }
        else { printf("Somehow, invalid comms type set. xTAG Test failed.\n"); }

    }
    else { printf("Somehow, invalid device type set. Test failed.\n"); }

    // Close sockets.
    closesocket(PrimeConnectSocket);
    printf("Primary socket disconnected.\n");
    closesocket(StreamConnectSocket);
    printf("Stream socket disconnected.\n");

    // Cleanup socket support.
    WSACleanup();

	return 0;
}

