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

#ifndef SINGLE_LOCK_H
#define SINGLE_LOCK_H

// Provide platform-specific implementation of locks in a simple interface
// Usage: Simply inherit this class, and then you can call Lock() and Unlock()
//        Code between Lock() and Unlock() cannot be executed simutaneously in different threads
class SingleLock
{
private:
    CRITICAL_SECTION _cs;

protected:
    void Lock();
    void Unlock();

public:
    SingleLock();
    ~SingleLock();
};

#endif
