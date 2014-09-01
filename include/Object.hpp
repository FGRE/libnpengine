/* 
 * libnpengine: Nitroplus script interpreter
 * Copyright (C) 2014 Mislav Blažević <krofnica996@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * */
#ifndef OBJECT_HPP
#define OBJECT_HPP

#include "ResourceMgr.hpp"

class Window;
class Object
{
public:
    virtual ~Object() {}
    virtual void Request(const string& State) { }
    virtual void Delete(Window* pWindow) { }
};

struct ObjectHolder : private CacheHolder<Object>
{
    static Object* Read(const string& Handle)
    {
        if (Handle.front() == '@')
            return CacheHolder::Read(Aliases[Handle.substr(1)]);
        return CacheHolder::Read(Handle);
    }

    static void Write(const string& Handle, Object* pObject)
    {
        CacheHolder::Write(Handle, pObject);
    }

    static void WriteAlias(const string& Handle, const string& Alias)
    {
        Aliases[Alias] = Handle;
    }

    static map<string, string> Aliases;
};

#endif
