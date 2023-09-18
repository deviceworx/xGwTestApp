#pragma once

/*
 * File:   CUtility.h
 * Author: mjanke
 *
 * Created on Aug 31, 2023
 *
 * Class including methods that support gen functions like sending cmds and getting responses.
 */

#ifndef CUTILITY_H
#define CUTILITY_H

#include <winsock2.h>
#include <time.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#include "Defines.h"      // Win32 Type defines and Dworx Cmd, Response, Msg defines.

#define SOCKET_RECV_SLEEP_MSEC  100     // Used in ProcCmdForResp()
#define STREAM_REC_BUFLEN       8192    // Default max for winsock. Set SO_SNDBUF for larger.
#define PRIMARY_REC_BUFLEN      8192    // Cmd response useable capacity.

class CUtility
{
public:

    CUtility();
    ~CUtility();

    // Static methods supporting gen functionality.
    static U32 GetCurrentUnixTimeUTC(void);
    static bool ProcCmdForResp(SOCKET Sckt, U8* pCmd, U16 cmdLen, U8* pResp, U16 respCapacity, U32 timeOutmSec);

private:

};

#endif /* CUTILITY_H */

