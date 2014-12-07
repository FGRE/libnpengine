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
#include <regex>

class Window;
class Object
{
public:
    virtual ~Object() {}
    virtual void Request(int32_t State) { }
    virtual void Delete(Window* pWindow) { }
    virtual bool Action() { return false; }
};

struct ObjectHolder_t : private Holder<Object>
{
    Object* Read(const string& Handle)
    {
        string Leftover = Handle;
        string ObjHandle = ExtractObjHandle(Leftover);
        if (Leftover.empty())
            return Holder::Read(ObjHandle);
        return GetHolder(ObjHandle)->Read(Leftover);
    }

    void Write(const string& Handle, Object* pObject)
    {
        string Leftover = Handle;
        string ObjHandle = ExtractObjHandle(Leftover);
        if (Leftover.empty())
            Holder::Write(ObjHandle, pObject);
        else
            GetHolder(ObjHandle)->Write(Leftover, pObject);
    }

    string Regexify(const string& Wildcard)
    {
        return string("^" + Wildcard.substr(0, Wildcard.size() - 1) + ".*");
    }

    template <class F>
    void ExecuteSafe(const string& HolderHandle, const string& Handle, F Func)
    {
        if (ObjectHolder_t* pHolder = GetHolder(HolderHandle))
            pHolder->Execute(Handle, Func);
    }

    template <class F>
    void WildcardAlias(const string& Leftover, const string& ObjHandle, F Func)
    {
        std::regex Regex(Regexify(ObjHandle.substr(1)));
        for (auto i = Aliases.begin(); i != Aliases.end(); ++i)
            if (std::regex_match(i->first, Regex))
                Leftover.empty() ? Execute(i->second, Func) : ExecuteSafe(i->second, Leftover, Func);
    }

    template <class F>
    void WildcardCache(const string& Leftover, const string& ObjHandle, F Func)
    {
        std::regex Regex(Regexify(ObjHandle));
        for (auto i = Cache.begin(); i != Cache.end(); ++i)
            if (std::regex_match(i->first, Regex))
                Leftover.empty() ? Func(&i->second) : ExecuteSafe(i->first, Leftover, Func);
    }

    template <class F>
    void Wildcard(const string& Leftover, const string& ObjHandle, F Func)
    {
        if (ObjHandle.front() == '@')
            WildcardAlias(Leftover, ObjHandle, Func);
        else
            WildcardCache(Leftover, ObjHandle, Func);
    }

    template <class F>
    void Execute(const string& Handle, F Func)
    {
        string Leftover = Handle;
        string ObjHandle = ExtractObjHandle(Leftover);
        if (ObjHandle.back() == '*')
            Wildcard(Leftover, ObjHandle, Func);
        else
            Leftover.empty() ? Func(&Cache.find(ObjHandle)->second) : GetHolder(ObjHandle)->Execute(Leftover, Func);
    }

    void WriteAlias(const string& Handle, const string& Alias)
    {
        Aliases[Alias] = Handle;
    }

    string ExtractObjHandle(string& Handle)
    {
        // Name
        string ObjHandle;
        size_t Index = Handle.find('/');
        if (Index != string::npos)
        {
            ObjHandle = Handle.substr(0, Index);
            Handle = Handle.substr(Index + 1);
        }
        else
        {
            ObjHandle = Handle;
            Handle.clear();
        }
        // Alias
        if (ObjHandle.front() == '@' && ObjHandle.back() != '*')
        {
            Handle = Aliases[ObjHandle.substr(1)] + "/" + Handle;
            ObjHandle = ExtractObjHandle(Handle);
        }
        return ObjHandle;
    }

    ObjectHolder_t* GetHolder(const string& Handle)
    {
        return dynamic_cast<ObjectHolder_t*>(Holder::Read(Handle));
    }

    map<string, string> Aliases;
};

#endif
