// Copyright (C) 2012-2014 F32 (feng32tc@gmail.com)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 3 as
// published by the Free Software Foundation;
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 

#ifndef VIRTUAL_SOURCE_H
#define VIRTUAL_SOURCE_H

#include "TrafficSource.h"

class VirtualSource : public TrafficSource
{
public:
    bool Initialize();
    virtual ~VirtualSource();

    virtual int EnumDevices();
    virtual void GetDeviceName(int index, TCHAR *buf, int cchLen);
    virtual bool SelectDevice(int index);

    virtual bool Capture(PacketInfo *pi, bool *timeout);

    bool Reconnect(int index) { return true; };
};

#endif