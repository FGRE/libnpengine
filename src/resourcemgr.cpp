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
#include "resourcemgr.hpp"
#include "scriptfile.hpp"

#include <memory>

ResourceMgr* sResourceMgr;

ResourceMgr::ResourceMgr(const std::vector<INpaFile*>& Achieves) :
Archives(Achieves)
{
    assert(!Archives.empty());
}

ResourceMgr::~ResourceMgr()
{
    std::for_each(Archives.begin(), Archives.end(), std::default_delete<INpaFile>());
    CacheHolder<ScriptFile>::Clear();
}

Resource ResourceMgr::GetResource(std::string Path)
{
    std::transform(Path.begin(), Path.end(), Path.begin(), ::tolower);
    for (uint32_t i = 0; i < Archives.size(); ++i)
    {
        auto File = Archives[i]->FindFile(Path);
        if (File != Archives[i]->End())
            return Resource(Archives[i], File);
    }
    return Resource(nullptr, Archives[0]->End());
}

char* ResourceMgr::Read(std::string Path, uint32_t& Size)
{
    std::transform(Path.begin(), Path.end(), Path.begin(), ::tolower);
    for (uint32_t i = 0; i < Archives.size(); ++i)
        if (char* pData = Archives[i]->ReadFile(Path, Size))
            return pData;
    Size = 0;
    return nullptr;
}

ScriptFile* ResourceMgr::GetScriptFile(const std::string& Path)
{
    // Check cache
    if (ScriptFile* pCache = CacheHolder<ScriptFile>::Read(Path))
        return pCache;

    ScriptFile* pScript = nullptr;
    std::string MapPath(Path, 0, Path.size() - 3);
    MapPath += "map";

    // Check Archives
    uint32_t NsbSize;
    char* NsbData = Read(Path, NsbSize);
    uint32_t MapSize;
    char* MapData = Read(MapPath, MapSize);

    // Both files found
    if (NsbData && MapData)
    {
        pScript = new ScriptFile(Path, NsbData, NsbSize, MapData, MapSize);
        CacheHolder<ScriptFile>::Write(Path, pScript);
    }

    delete[] NsbData;
    delete[] MapData;
    return pScript;
}
