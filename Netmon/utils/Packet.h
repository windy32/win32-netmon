#ifndef PACKET_H
#define PACKET_H

#define NET_IPv4  0x0800
#define NET_ARP   0x0806
#define NET_IPv6  0x86DD
#define NET_OTHER 0x0000

#define TRA_TCP   6
#define TRA_UDP   17
#define TRA_ICMP  1
#define TRA_OTHER 0

#define DIR_UP    1
#define DIR_DOWN  2

typedef struct tagPacketInfo
{
	int     networkProtocol;
	int     trasportProtocol;

	int     local_port;    // Only for TCP & UDP
	int     remote_port;

	int     dir;     
	int     size;    // In Bytes

	int     time_s;  // Time in Secode
	int     time_us; // Time in Microseconds
} PacketInfo;

typedef struct tagPacketInfoEx
{
	// Part 1 -----------------------------------
	int     networkProtocol;
	int     trasportProtocol;

	int     local_port;    // Only for TCP & UDP
	int     remote_port;

	int     dir;     
	int     size;    // In Bytes

	int     time_s;  // Time in Secode
	int     time_us; // Time in Microseconds

	// Part 2 -----------------------------------
	int     pid;
	int     puid;    // Process UID
	int     pauid;   // Process Activity UID
	TCHAR   name[MAX_PATH];
	TCHAR   fullPath[MAX_PATH];
} PacketInfoEx;

#endif