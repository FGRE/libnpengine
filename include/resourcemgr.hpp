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
    static T* Read(const std::string& Path) { return Cache[Path]; }
    static void Write(const std::string& Path, T* Data) { Cache[Path] = Data; }
    static std::map<std::string, T*> Cache;
};

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
    template <class T> T* GetResource(const std::string& Path);

private:
    std::map<std::string, NpaIterator> FileRegistry;
    std::vector<NpaFile*> Achieves;
};

#endif
