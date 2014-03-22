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
#include "npafile.hpp"
#include "scriptfile.hpp"

#include <memory>
#include <boost/locale.hpp>
using namespace boost::locale;
using namespace boost::locale::conv;

ResourceMgr* sResourceMgr;

ResourceMgr::ResourceMgr(const std::vector<std::string>& AchieveFileNames)
{
    std::locale loc = generator().generate("ja_JP.SHIFT-JIS");
    Achieves.resize(AchieveFileNames.size());
    for (uint32_t i = 0; i < AchieveFileNames.size(); ++i)
    {
        NpaFile* pAchieve = new NpaFile(AchieveFileNames[i], NPA_READ);
        Achieves[i] = pAchieve;
        for (NpaIterator File = pAchieve->Begin(); File != pAchieve->End(); ++File)
            FileRegistry.insert(std::pair<std::string, NpaIterator>(to_utf<char>(File.GetFileNameRaw(), File.GetFileNameRaw() + File.GetFileNameSize(), loc), File));
    }
}

ResourceMgr::~ResourceMgr()
{
    std::for_each(Achieves.begin(), Achieves.end(), std::default_delete<NpaFile>());
    CacheHolder<ScriptFile>::Clear();
}

char* ResourceMgr::Read(const std::string& Path, uint32_t* Size)
{
    NpaIterator File = GetFile(Path);
    if (File)
    {
        *Size = File.GetFileSize();
        return File.GetFileData();
    }
    *Size = 0;
    return nullptr;
}

NpaIterator ResourceMgr::GetFile(std::string Path)
{
    std::transform(Path.begin(), Path.end(), Path.begin(), ::tolower);
    auto iter = FileRegistry.find(Path);
    if (iter != FileRegistry.end())
        return iter->second;
    return NpaIterator();
}

ScriptFile* ResourceMgr::GetScriptFile(const std::string& Path)
{
    // Check cache
    if (ScriptFile* pCache = CacheHolder<ScriptFile>::Read(Path))
        return pCache;

    std::string MapPath(Path, 0, Path.size() - 3);
    MapPath += "map";

    // Check achieves
    uint32_t NsbSize;
    char* NsbData = Read(Path, &NsbSize);
    uint32_t MapSize;
    char* MapData = Read(MapPath, &MapSize);

    // Both files found
    if (NsbData && MapData)
    {
        ScriptFile* pScript = new ScriptFile(Path, NsbData, NsbSize, MapData, MapSize);
        CacheHolder<ScriptFile>::Write(Path, pScript);
        return pScript;
    }

    // Either file not found
    delete NsbData;
    delete MapData;
    return nullptr;
}
