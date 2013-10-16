#include "resourcemgr.hpp"
#include "npafile.hpp"

ResourceMgr::ResourceMgr(const std::vector<std::string>& AchieveFileNames)
{
    FileRegistry.resize(AchieveFileNames.size());
    for (uint32_t i = 0; i < AchieveFileNames.size(); ++i)
    {
        NpaFile* pAchieve = new NpaFile(AchieveFileNames[i], NPA_READ);
        FileRegistry[i].first = pAchieve;
        for (NpaIterator File = pAchieve->Begin(); File != pAchieve->End(); ++File)
            FileRegistry[i].second[File.GetFileName()] = File.GetOffset();
    }
}

void ResourceMgr::ClearCache()
{
}

NsbFile* ResourceMgr::GetScript(const std::string& Path)
{
    return nullptr;
}
