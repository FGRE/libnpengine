#ifndef NSB_INTERPRETER_HPP
#define NSB_INTERPRETER_HPP

#include <stack>
#include <cstdint>
#include <map>

class NsbFile;
class Game;
class ResourceMgr;

struct Variable
{
    std::string Type;
    std::string Value;
};

class NsbInterpreter
{
public:
    NsbInterpreter(Game* pGame, ResourceMgr* pResourceMgr, const std::string& InitScript);
    ~NsbInterpreter();

    void Run();

private:
    bool Boolify(const std::string& String);

    template <class T> T* GetHandle(const std::string& Identifier);
    template <class T> T GetVariable(const std::string& Identifier);
    void SetVariable(const std::string& Identifier, const Variable& Var);

    void LoadMovie(const std::string& HandleName, int32_t Priority, int32_t x,
                   int32_t y, bool Loop, bool unk0, const std::string& File, bool unk1);

    Game* pGame;
    ResourceMgr* pResourceMgr;
    bool EndHack;
    std::stack<uint32_t> ReturnLines;
    std::stack<NsbFile*> ScriptStack;
    std::map<std::string, Variable> Variables;
    std::map<std::string, void*> Handles;
    std::vector<Variable> Params;
};

#endif
