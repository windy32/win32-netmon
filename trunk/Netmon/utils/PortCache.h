#ifndef PORT_CACHE_H
#define PORT_CACHE_H

class PortCache
{
protected:
    // The port cache that makes looking up tcp / udp table much more faster.
    // The table is initialized to all zero in constructor, which means that
    // no map from port to pid is logged.
    //
    // PortCache class has a GetTcpPortPid() / GetUdpPortPid interface, which is used to replace
    // GetExtendedTcpTable / GetExtendedUdpTable for better performance.
    // 
    // If the map from a certain port to pid is loged in port table, 
    // PortCache will return pid quickly.
    // 
    // If not, PortCache calls GetExtendedTcpTable / GetExtendedUdpTable to update the port table, 
    // and then return the result.
    //
    // Return Value
    //
    //     GetTcpPortPid / GetPortPortPid returns the pid for the corresponding port.
    //     If the pid cannot be found, return value is 0.

    int _tcpPortTable[65536];
    int _udpPortTable[65536];

public:
    PortCache();

    int GetTcpPortPid(int port);
    int GetUdpPortPid(int port);
    void RebuildTcpTable();
    void RebuildUdpTable();
};

#endif
