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
#include "ProfileItem.h"

// Bool Item
ProfileBoolItem::ProfileBoolItem(bool value)
{
    this->value = value;
}

ProfileValueItem *ProfileBoolItem::Parse(const std::tstring &text)
{
    int value;
    if (_stscanf_s(text.data(), TEXT("%d"), &value) == 1) // Ok
    {
        return new ProfileBoolItem(value ? true : false);
    }

    return NULL;
}

std::tstring ProfileBoolItem::ToString()
{
    return value ? TEXT("1") : TEXT("0");
}

// Int Item
ProfileIntItem::ProfileIntItem(int value)
{
    this->value = value;
}

ProfileValueItem *ProfileIntItem::Parse(const std::tstring &text)
{
    int value;
    if (_stscanf_s(text.data(), TEXT("%d"), &value) == 1) // Ok
    {
        return new ProfileIntItem(value);
    }

    return NULL;
}

std::tstring ProfileIntItem::ToString()
{
    TCHAR buf[16];
    _stprintf_s(buf, 16, TEXT("%d"), value);
    return std::tstring(buf);
}

// String Item
ProfileStringItem::ProfileStringItem(const std::tstring &value)
{
    this->value = value;
}

ProfileValueItem *ProfileStringItem::Parse(const std::tstring &text)
{
    return new ProfileStringItem(text);
}

std::tstring ProfileStringItem::ToString()
{
    return value;
}

// Int List Item
ProfileIntListItem::ProfileIntListItem()
{
}

ProfileIntListItem::ProfileIntListItem(const std::vector<int> &value)
{
    this->value = value;
}

ProfileValueItem *ProfileIntListItem::Parse(const std::tstring &text)
{
    int puid;
    std::vector<int> puids;
    int offset = 0;

    while (_stscanf_s(text.data() + offset, TEXT("%d"), &puid) == 1)
    {
        puids.push_back(puid);

        // Update offset
        TCHAR buf[16];
        _stprintf_s(buf, 16, TEXT("%d"), puid);
        offset += _tcslen(buf) + 1;
    }

    return new ProfileIntListItem(puids);
}

std::tstring ProfileIntListItem::ToString()
{
    std::tstring str;
    TCHAR pid[16];

    // Generate String
    for (unsigned int i = 0; i < value.size(); i++)
    {
        if (i != value.size() - 1)
        {
            _stprintf_s(pid, 16, TEXT("%d "), value[i]);
        }
        else
        {
            _stprintf_s(pid, 16, TEXT("%d"), value[i]);
        }
        str.append(pid);
    }
    return str;
}
