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
#include "ResourceMgr.hpp"
#include "scriptfile.hpp"
#include <memory>

ResourceMgr* sResourceMgr;

ResourceMgr::ResourceMgr()
{
}

ResourceMgr::~ResourceMgr()
{
    for_each(Archives.begin(), Archives.end(), default_delete<INpaFile>());
    CacheHolder<ScriptFile>::Clear();
}

char* ResourceMgr::Read(string Path, uint32_t& Size)
{
    transform(Path.begin(), Path.end(), Path.begin(), ::tolower);
    for (uint32_t i = 0; i < Archives.size(); ++i)
        if (char* pData = Archives[i]->ReadFile(Path, Size))
            return pData;
    Size = 0;
    return nullptr;
}

ScriptFile* ResourceMgr::GetScriptFile(const string& Path)
{
    if (ScriptFile* pCache = CacheHolder<ScriptFile>::Read(Path))
        return pCache;

    ScriptFile* pScript = ReadScriptFile(Path);
    if (!pScript)
        return nullptr;

    CacheHolder<ScriptFile>::Write(Path, pScript);
    return pScript;
}
