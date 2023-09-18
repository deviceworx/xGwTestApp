#pragma once

/*
 * File:   CHloTests.h
 * Author: mjanke
 *
 * Created on Aug 30, 2023
 *
 * Class including methods that implement Deviceworx xGATEWAY Socket API tests (for xtaghlod interaction).
 */

#ifndef CHLOTESTS_H
#define CHLOTESTS_H

#include "Defines.h"    // Win32 Type defines and Dworx Cmd, Response, Msg defines.
#include "CUtility.h"   // Misc utility methods.

#define xTAG_TEST_TXPOWER   18  // dbm
#define xTAG_TEST_HBPERIOD  2   // Min
#define xTAG_TEST_HBRETRIES 3

class CHloTests
{
public:

    CHloTests();
    ~CHloTests();

    // Executible tests.
    static bool ExGATEWAYTest(SOCKET PrimeSkt, SOCKET StreamSkt);
    static bool ExTAGVibTest(SOCKET PrimeSkt, SOCKET StreamSkt);
    static bool ExTAGEnvTest(SOCKET PrimeSkt, SOCKET StreamSkt);
    static bool ExTAGCo2Test(SOCKET PrimeSkt, SOCKET StreamSkt);
    static bool ExTAGFOTATest(SOCKET PrimeSkt, SOCKET StreamSkt);

private:

};

#endif /* CHLOTESTS_H */

