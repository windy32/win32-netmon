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

#include "stdafx.h"
#include "VirtualSource.h"

bool VirtualSource::Initialize()
{
    return true;
}

VirtualSource::~VirtualSource()
{
}

int VirtualSource::EnumDevices()
{
    return 1;
}

void VirtualSource::GetDeviceName(int index, TCHAR *buf, int cchLen)
{
    _tcscpy_s(buf, cchLen, TEXT("Virtual Adapter #1"));
}

bool VirtualSource::SelectDevice(int index)
{
    return true;
}

bool VirtualSource::Capture(PacketInfo *pi, bool *timeout)
{
    /*
    static int s_day = 0;
    static int s_byte = 0;
    static int s_limit = 0;

    if (s_limit == 0)
    {
        s_limit = rand() * 100;
    }
    */

    // Sleep(1);

    // Generate a packet
    pi->size = 1460; //14600;
    pi->time_s = (int)time(0); //(int)time(0) + 86400 * s_day;
    pi->time_us = 0;

    pi->dir = DIR_DOWN;
    pi->networkProtocol = NET_IPv4;
    pi->trasportProtocol = TRA_TCP;
    pi->remote_port = 80;
    pi->local_port  = 8000;

    // Update statistics
    /*
    s_byte += 14600;
    if (s_byte > s_limit)
    {
        s_byte = 0;
        s_limit = 0;
        s_day += 1;
    }
    */

    *timeout = false;
    return true;
}
