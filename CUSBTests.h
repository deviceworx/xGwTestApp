#pragma once

/*
 * File:   CUSBTests.h
 * Author: mjanke
 *
 * Created on Aug 30, 2023
 * 
 * Class including methods that implement Deviceworx xGATEWAY Socket API tests (for xtagusbd interaction). 
 */

#ifndef CUSBTESTS_H
#define CUSBTESTS_H

#include "Defines.h"    // Win32 Type defines and Dworx Cmd, Response, Msg defines.
#include "CUtility.h"   // Misc utility methods.

class CUSBTests
{
public:

    CUSBTests();
    ~CUSBTests();

    // Executible tests.
    static bool ExGATEWAYTest(SOCKET PrimeSkt, SOCKET StreamSkt);
    static bool ExTAGVibTest(SOCKET PrimeSkt, SOCKET StreamSkt);
    static bool ExTAGEnvTest(SOCKET PrimeSkt, SOCKET StreamSkt);
    static bool ExTAGCo2Test(SOCKET PrimeSkt, SOCKET StreamSkt);

private:

};

#endif /* CUSBTESTS_H */