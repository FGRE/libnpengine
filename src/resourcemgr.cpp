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
#include "resourcemgr.hpp"
#include "npafile.hpp"
#include "nsbfile.hpp"

#include <memory>
#include <algorithm>

ResourceMgr::ResourceMgr(const std::vector<std::string>& AchieveFileNames)
{
    Achieves.resize(AchieveFileNames.size());
    for (uint32_t i = 0; i < AchieveFileNames.size(); ++i)
    {
        NpaFile* pAchieve = new NpaFile(AchieveFileNames[i], NPA_READ);
        Achieves[i] = pAchieve;
        for (NpaIterator File = pAchieve->Begin(); File != pAchieve->End(); ++File)
            FileRegistry.insert(std::pair<std::string, NpaIterator>(File.GetFileName(), File));
    }
}

ResourceMgr::~ResourceMgr()
{
    std::for_each(Achieves.begin(), Achieves.end(), std::default_delete<NpaFile>());
    ClearCache();
}

void ResourceMgr::ClearCache()
{
    // TODO
}

char* ResourceMgr::Read(std::string Path, uint32_t* Size)
{
    std::transform(Path.begin(), Path.end(), Path.begin(), ::tolower);
    auto iter = FileRegistry.find(Path);
    if (iter != FileRegistry.end())
    {
        *Size = iter->second.GetFileSize();
        return iter->second.GetFileData();
    }
    *Size = 0;
    return nullptr;
}
