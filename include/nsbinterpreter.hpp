#ifndef NSB_INTERPRETER_HPP
#define NSB_INTERPRETER_HPP

#include <stack>
#include <cstdint>
#include <map>
#include <boost/any.hpp>

class NsbFile;
class Game;
class ResourceMgr;

class NsbInterpreter
{
public:
    NsbInterpreter(Game* pGame, ResourceMgr* pResourceMgr, const std::string& InitScript);
    ~NsbInterpreter();

    void Run();

private:
    bool Boolify(const std::string& String);
    template<class T> T* GetVariable(const std::string& Identifier) const;

    void LoadMovie(const std::string& HandleName, int32_t Priority, int32_t x,
                   int32_t y, bool Loop, bool unk0, const std::string& File, bool unk1);

    Game* pGame;
    ResourceMgr* pResourceMgr;
    bool EndHack;
    std::stack<uint32_t> ReturnLines;
    std::stack<NsbFile*> ScriptStack;
    std::map<std::string, boost::any> Variables;
    std::vector<std::string> Params;
};

#endif
