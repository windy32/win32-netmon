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

#ifndef PROFILE_H
#define PROFILE_H

#include "ProfileItem.h"

// The Profile class is used to operate an ini file with only one section.
class Profile
{
private:
    TCHAR _filename[MAX_PATH];
    TCHAR _sectionName[MAX_PATH];

    std::map<std::tstring, ProfileValueItem *> _defaults;
    std::map<std::tstring, ProfileValueItem *> _values;

public:
    // Input some information of the profile
    void Init(const TCHAR *fileName, const TCHAR *sectionName);

    // Register default value
    void RegisterDefault(const TCHAR *option, ProfileValueItem *item);

    // Load
    void Load();

    // Get values
    ProfileBoolItem    *GetBool(const TCHAR *option);
    ProfileIntItem     *GetInt(const TCHAR *option);
    ProfileStringItem  *GetString(const TCHAR *option);
    ProfileIntListItem *GetIntList(const TCHAR *option);

    // Set value
    void SetValue(const TCHAR *option, ProfileValueItem *item);
};

#endif
