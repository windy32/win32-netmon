#ifndef DETAIL_MODEL_H
#define DETAIL_MODEL_H

#include "NetModel.h"

class DetailModel : public NetModel
{
private:
    // Item Definition
    typedef struct tagDtModelItem
    {
        __int64 curPackets;
        __int64 prevPackets;

        struct tagDtModelItem()
        {
            curPackets = 0;
            prevPackets = 0;
        }
    } DtModelItem;

    // Items
    std::map<int, DtModelItem> _items;

private:
    void InitDatabase();

public:
    DetailModel();

    // Modify the Model
    void InsertPacket(PacketInfoEx *pi);
    void SetPrevPackets(int process, __int64 numPackets);
    void ClearPackets();

    // Export Model Info
    __int64 GetCurPackets(int process);
    __int64 GetPrevPackets(int process);

    // Save Model Into to Database
    void SaveDatabase();
};

#endif
