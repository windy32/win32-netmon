#ifndef TRAFFIC_SOURCE_H
#define TRAFFIC_SOURCE_H

#include "../utils/Packet.h"

#pragma region Protocol Headers

#pragma pack(1)

// MACv2 Header
typedef struct tagMacHeader 
{
    unsigned char  dst[6];
    unsigned char  src[6];
    unsigned short protocol;
} MacHeader;

/*
00 26 18 b1 cb d3 dc d2 fc 98 64 f9 88 64 11 00 21 6c 05 ca 00 21
~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~ ~~~~~ ~~ ~~ ~~~~~ ~~~~~ ~~~~~
Destination Addr  Source Address    Ethernet Type           PPP Protocol: 0x0021* (IP)
                                    0x8862 / 0x8864*
---------- MAC Packet ---------------------+-- PPPoE Packet -+-----
                                          Version / Type: 0x11*
                                             Code: 0x00*
                                                Session ID
                                                      Length (1482)
*/

// PPPOE Header
typedef struct tagPppoeHeader
{
    unsigned ver      : 4;
    unsigned type     : 4;
    unsigned code     : 8;
    unsigned session  : 16;
    unsigned length   : 16;
    unsigned protocol : 16;
} PppoeHeader;

// IPv4 Header
typedef struct tagIpHeader
{
    unsigned ver      : 4;
    unsigned hdr_len  : 4;
    unsigned          : 8;
    unsigned len      : 16;

    unsigned          : 16;
    unsigned          : 3;
    unsigned          : 13;

    unsigned          : 8;
    unsigned protocol : 8;
    unsigned          : 16;

    unsigned src      : 32;
    unsigned dst      : 32;
} IpHeader;

// UDP Header
typedef struct tagUdpHeader
{
    unsigned short src_port;
    unsigned short dst_port;
    unsigned short len;
    unsigned short checksum;
} UdpHeader;

// TCP Header
typedef struct tagTcpHeader
{
    unsigned src_port      : 16;
    unsigned dst_port      : 16;

    unsigned               : 32;

    unsigned               : 32;

    unsigned data_offset   : 4;
    unsigned               : 6;
    unsigned flags         : 6;
    unsigned window        : 16;

    unsigned checksum      : 16;
    unsigned emergency_ptr : 16;
} TcpHeader;

#pragma pack()

#pragma endregion

class TrafficSource
{
public:
    virtual bool Initialize() = 0;
    virtual ~TrafficSource() {};

    virtual int EnumDevices() = 0;
    virtual void GetDeviceName(int index, TCHAR *buf, int cchLen) = 0;
    virtual bool SelectDevice(int index) = 0;

    // Return value
    // 1. Returns true and timeout = false, if a packet has been captured
    // 2. Returns false and timeout = true, if no packet captured in this period, should call again
    // 3. Returns false and timeout = false, if some error occurred
    virtual bool Capture(PacketInfo *pi, bool *timeout) = 0;

    //bool ReConnect(int i);
};

#endif
