#include "CUtility.h"

CUtility::CUtility(void) { }
CUtility::~CUtility() { }

U32 CUtility::GetCurrentUnixTimeUTC(void)
{
    time_t curUnixTime;
    time(&curUnixTime);
    return (U32)curUnixTime;
}

bool CUtility::ProcCmdForResp(SOCKET Sckt, U8* pCmd, U16 cmdLen, U8* pResp, U16 respCapacity, U32 timeOutmSec)
{
    // Clear out the response.
    memset(pResp, 0, respCapacity);

    int iRes = send(Sckt, (const char*) pCmd, cmdLen, 0);
    if (iRes == SOCKET_ERROR)
    {
        printf("Cmd send failed with error: %d\n", WSAGetLastError());
        return false;
    }

    // printf("Bytes Sent: %ld\n", iRes);

    UINT32 funcTimeLeft = timeOutmSec;
    while (funcTimeLeft > 0)
    {
        // Non-block read will return immediatly if no bytes available.
        iRes = recv(Sckt, (char*) pResp, respCapacity, 0);

        if (iRes > 0)  // Read something. Safe to assome all bytes for each message will come in within internal timeout.
        {
            // DEBUG
            /*
            printf("Cmd proc with %d mSec left. Resp: ", funcTimeLeft);
            for (int x = 0; x < iRes; x++) { printf("%02X ", pResp[x] & 0x000000FF); }
            printf("\n");
            */
            return true;
        }
        else if (iRes == 0)
        {
            //printf("Connection closed in cmd proc.\n");
            return false;
        }
        else // < 0
        {
            if (WSAGetLastError() == WSAEWOULDBLOCK) // Try again - unless timeout.
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