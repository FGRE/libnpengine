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

    char* Read(const std::string& Path);
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

    // Check achieves
    if (char* Data = Read(Path))
    {
        T* pScript = new T(Path, Data);
        CacheHolder<T>::Write(Path, pScript);
        return pScript;
    }

    return nullptr;
}

#endif
