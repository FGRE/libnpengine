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

template <class T> T* ResourceMgr::GetResource(const std::string& Path)
{
    // Check cache
    if (T* pCache = CacheHolder<T>::Read(Path))
        return pCache;

    // Check achieves
    if (char* Data = FileRegistry[Path].GetFileData())
    {
        T* pScript = new T(Path, Data);
        CacheHolder<T>::Write(Path, pScript);
        return pScript;
    }

    return nullptr;
}
