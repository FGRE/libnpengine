/* 
 * libnpengine: Nitroplus script interpreter
 * Copyright (C) 2013 Mislav Blažević <krofnica996@gmail.com>
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

typedef std::map<std::string, uint32_t> Registry;

class NpaFile;

template <class T>
struct CacheHolder
{
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

struct MapDeleter
{
    template <class T> void operator() (T Data) { delete Data.second; }
};

class ResourceMgr
{
public:
    ResourceMgr(const std::vector<std::string>& AchieveFileNames);
    ~ResourceMgr();

    void ClearCache();

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

#endif
