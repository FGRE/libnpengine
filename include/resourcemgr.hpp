/* 
 * libnpengine: Nitroplus script interpreter
 * Copyright (C) 2013-2014 Mislav Blažević <krofnica996@gmail.com>
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
#ifndef RESOURCE_MGR_HPP
#define RESOURCE_MGR_HPP

#include "npaiterator.hpp"

#include <vector>
#include <map>
#include <utility>
#include <algorithm>
#include <cstring>

typedef std::map<std::string, uint32_t> Registry;

class NpaFile;

struct MapDeleter
{
    template <class T> void operator() (T Data) { delete Data.second; }
};

template <class T>
struct CacheHolder
{
    typedef typename std::map<std::string, T*>::iterator CacheIter;

    static CacheIter ReadFirstMatch(const std::string& Key)
    {
        auto i = Cache.begin();
        if (i == Cache.end())
            return i;
        else if (CheckWildcard(Key, i))
            return i;
        else
            return ReadNextMatch(Key, i);
        return i;
    }

    static CacheIter ReadNextMatch(const std::string& Key, CacheIter i)
    {
        while (++i != Cache.end())
            if (CheckWildcard(Key, i))
                return i;
        return i;
    }

    static bool CheckWildcard(const std::string& Key, const CacheIter& i)
    {
        if (Key.size() - 1 <= i->first.size())
            if (std::memcmp(Key.c_str(), i->first.c_str(), Key.size() - 1) == 0)
                return true;
        return false;
    }

    static void Clear()
    {
        std::for_each(Cache.begin(), Cache.end(), MapDeleter());
        Cache.clear();
    }

    static T* Read(const std::string& Path)
    {
        auto iter = Cache.find(Path);
        if (iter != Cache.end())
            return iter->second;
        return nullptr;
    }

    static void Write(const std::string& Path, T* Data)
    {
        Cache[Path] = Data;
    }

    static std::map<std::string, T*> Cache;
};

template <class T>
std::map<std::string, T*> CacheHolder<T>::Cache;

class ResourceMgr
{
public:
    ResourceMgr(const std::vector<std::string>& AchieveFileNames);
    ~ResourceMgr();

    void ClearCache();

    NpaIterator GetFile(std::string Path);
    char* Read(std::string Path, uint32_t* Size);
    template <class T> T* GetResource(const std::string& Path);

private:
    std::map<std::string, NpaIterator> FileRegistry;
    std::vector<NpaFile*> Achieves;
};

template <class T> T* ResourceMgr::GetResource(const std::string& Path)
{
    // Check cache
    if (T* pCache = CacheHolder<T>::Read(Path))
        return pCache;

    uint32_t Size;

    // Check achieves
    if (char* Data = Read(Path, &Size))
    {
        T* pScript = new T(Path, Data, Size);
        CacheHolder<T>::Write(Path, pScript);
        return pScript;
    }

    return nullptr;
}

extern ResourceMgr* sResourceMgr;

#endif
