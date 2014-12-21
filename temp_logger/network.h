/*
 *  network.h
 *
 *  Created on: Dec 21, 2014
 *      Author: Mitchel Pigsley
 */

#ifndef NETWORK_H_
#define NETWORK_H_

#define APPLICATION_NAME        "WLAN STATION"
#define APPLICATION_VERSION     "1.1.0"
#define HOST_NAME               "www.ti.com"
#define PING_INTERVAL       	1000    /* In msecs */
#define PING_TIMEOUT        	3000    /* In msecs */
#define PING_PKT_SIZE       	20      /* In bytes */
#define NO_OF_ATTEMPTS      	3
#define OSI_STACK_SIZE      	2048

// Application specific status/error codes
typedef enum {
    // Choosing -0x7D0 to avoid overlap w/ host-driver's error codes
    LAN_CONNECTION_FAILED = -0x7D0,
    INTERNET_CONNECTION_FAILED = LAN_CONNECTION_FAILED - 1,
    DEVICE_NOT_IN_STATION_MODE = INTERNET_CONNECTION_FAILED - 1,
    STATUS_CODE_MAX = -0xBB8
} e_AppStatusCodes;

void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvent);
void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent);
void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pHttpEvent,
                                  SlHttpServerResponse_t *pHttpResponse);
void SimpleLinkGeneralEventHandler(SlDeviceEvent_t *pDevEvent);
void SimpleLinkSockEventHandler(SlSockEvent_t *pSock);
void SimpleLinkPingReport(SlPingReport_t *pPingReport);

void InitializeNetwork();
long ConfigureSimpleLinkToDefaultState();
long CheckLanConnection();
long CheckInternetConnection();
long WlanConnect();

#endif /* NETWORK_H_ */
