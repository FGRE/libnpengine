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

char* ResourceMgr::Read(const std::string& Path, uint32_t* Size)
{
    auto iter = FileRegistry.find(Path);
    if (iter != FileRegistry.end())
    {
        *Size = iter->second.GetFileSize();
        return iter->second.GetFileData();
    }
    *Size = 0;
    return nullptr;
}
