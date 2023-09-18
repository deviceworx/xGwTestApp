# xGwTestApp
<img src="https://www.deviceworx.com/img/xGw3RenderObliqueView.png" alt="Screenshot of xGATEWAY 3.0" width="300"/>

# Overview
A test app demonstrating how to use the socket interface exposed from Deviceworx xGATEWAY Platforms.

This app has been created to provide xGATEWAY customers with a reference that they can use to develop their own client that uses or consumes the xGATEWAY socket interface.

Note that the interface is documented within the xGATEWAY users guide. Consult the latest rev of the user's guide - availble for view or download from the [Deviceworx Downloads Page](https://www.deviceworx.com/downloads).
A specific link to this viewable doc: [xGATEWAY HaLow User's Guide](https://www.deviceworx.com/assets/xGATEWAYUsersGuideR_2_5.pdf).
# General Comments
The app includes the following elements:
- [xGwTestApp.cpp](xGwTestApp.cpp): Includes "main" and simply calls individual test methods.
- [Defines.h](Defines.h): Defines constants including command and other message IDs.
- [CUtility.h](CUtility.h)/[CUtility.cpp](CUtility.cpp): The CUtility class implements ... static utility functions used by other classes (e.g. a GetCurrentUnixTimeUTC() returns the current unix time at Greenwich).
- [CUSBTests.h](CUSBTests.h)/[CUSBTests.cpp](CUSBTests.cpp): Implements all interface test methods supporting an xGATEWAY communicating with xTAG USB Sensors.
- [CHloTests.h](CHloTests.h)/[CHloTests.cpp](CHloTests.cpp): Implements all interface test methods supporting an xGATEWAY communicating with xTAG HaLow Sensors.
# Project Support and Format
The app was developed Visual Studio Community Edition. Project and solution files for VS are included. Note that this supports simple migration to other dev environments as required.
For simplicty, the app has been developed as a Windows Console app with simple user menu selection driving which tests are executed. Users enter required test parameters including xTAG IDs (when testing with an indentifed xTAG is required).
Because the app was intended to function as a good reference, its code is not optimized for efficiency. It has been structured to clearly show steps required for interface consumers for specific usage (e.g. Configure and xTAG for data acquisition, aquire data and then terminate acquisition).
# Local or Remote Execution
Because a socket interface is used, the app can be easily ported to run on an xGATEWAY. Or, more simply, execute the app, as is, on a Windows laptop that resides on the same subnet as an xGATEWAY. To connect via a socket, the xGATEWAY IP address must be identified and plugged into code. Locally, use a loopback IP address (127.0.0.1). The following code, at the top of xGwTestApp.cpp, stipulates the xGATEWAY IP address:

`#define DEFAULT_SERVER              "192.168.0.121"`
# Typical Test Sequences
## HaLow Tests
1. Connect to the xGATEWAY via sockets.
2. Configure an xGATEWAY or ...
3. Configure an xTAG.
    - Signal acquisition to start whenever the next xTAG connection to the xGATEWAY is made.
    - Stream fast data (e.g. from Vibration sensors) or periodically read slow data whenever xTAGs connect (e.g. from Environmental sensors).
    - Signal acquisition stop upon next xTAG connection
7. Disconnect sockets.
## USB Tests
1. Connect to the xGATEWAY via sockets.
2. Configure an xGATEWAY or
3. List available xTAGs.
   - Connect to an xTAG.
   - Start acquisition.
   - Stream fast data (e.g. from Vibration sensors) or periodically read slow data (e.g. from Environmental sensors).
   - Stop acquisition and disconnect the xTAG.
9. Disconnect sockets.
## BLE Support
Note that the current interface does not support xTAG BLE sensor interaction as BLE sensors are almost exclusivly used with the xTAG Explorer Android app (i.e. no xGATEWAY installation required). WiFi HaLow is the best option when wirelessly connecting xTAG sensors given that HaLow supports 1000's of concurrent connections (vs about 10 BLE connections) and HaLow supports 3x - 5x the wireless range.
# Support
If you have any questions regarding this application, please reach out to [Deviceworx Support](mailto:support@deviceworx.com).

