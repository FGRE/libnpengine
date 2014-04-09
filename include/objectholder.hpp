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
#ifndef OBJECT_HOLDER_HPP
#define OBJECT_HOLDER_HPP

#include "resourcemgr.hpp"

struct ObjectHolder : private CacheHolder<Object>
{
    template <class F> static void ForeachWildcard(const std::string& Key, std::function<void(F*)> Func, std::string& HandleName)
    {
        if (Key[0] == '@')
        {
            for (auto i = Aliases.begin(); i != Aliases.end(); ++i)
            {
                if (CheckWildcard(Key, i->first))
                {
                    auto j = Cache.find(i->second);
                    if (j != Cache.end())
                    {
                        if (F* pF = dynamic_cast<F*>(j->second))
                        {
                            HandleName = j->first;
                            Func(pF);
                        }
                    }
                }
            }
        }

        for (auto i = Cache.begin(); i != Cache.end(); ++i)
        {
            if (CheckWildcard(Key, i->first))
            {
                if (F* pF = dynamic_cast<F*>(i->second))
                {
                    HandleName = i->first;
                    Func(pF);
                }
            }
        }
    }

    static bool CheckWildcard(const std::string& Key, const std::string& Value)
    {
        if (Key.size() - 1 <= Value.size())
            if (std::memcmp(Key.c_str(), Value.c_str(), Key.size() - 1) == 0)
                return true;
        return false;
    }

    static Object* Read(std::string& HandleName)
    {
        if (Object* pObject = CacheHolder::Read(HandleName))
            return pObject;

        // Check aliases
        auto aiter = Aliases.find(HandleName);
        if (aiter != Aliases.end())
        {
            auto iter = Cache.find(aiter->second);
            if (iter != Cache.end())
            {
                HandleName = iter->first;
                return iter->second;
            }
        }
        return nullptr;
    }

    static void Write(const std::string& Path, Object* pObject)
    {
        CacheHolder::Write(Path, pObject);
    }

    static std::map<std::string, std::string> Aliases;
};

#endif
