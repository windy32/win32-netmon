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
    Sleep(10);

    pi->size = 1460;
    pi->time_s = (int)time(0); 
    pi->time_us = 0;

    pi->dir = DIR_DOWN;
    pi->networkProtocol = NET_IPv4;
    pi->trasportProtocol = TRA_TCP;
    pi->remote_port = 80;
    pi->local_port  = 50000;

    *timeout = false;
    return true;
}
