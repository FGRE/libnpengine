#ifndef RESOURCE_MGR_HPP
#define RESOURCE_MGR_HPP

#include <vector>
#include <map>
#include <cstdint>
#include <string>
#include <utility>

typedef std::map<std::string, uint32_t> Registry;

class NpaFile;
class NsbFile;

class ResourceMgr
{
public:
    ResourceMgr(const std::vector<std::string>& AchieveFileNames);

    void ClearCache();
    NsbFile* GetScript(const std::string& Path);

private:
    std::map<std::string, char*> ResourceCache;
    std::vector<std::pair<NpaFile*, Registry>> FileRegistry;
};

#endif
