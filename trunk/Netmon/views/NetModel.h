#ifndef NET_MODEL_H
#define NET_MODEL_H

#include "../utils/Packet.h"

// Base class of all concrete models
class NetModel
{
private:
    CRITICAL_SECTION _cs;

public:
    static const int PROCESS_ALL;

protected:
    void Lock();
    void Unlock();

public:
    NetModel();
    ~NetModel();
};

#endif
