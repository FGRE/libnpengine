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
    Variable() : Type("STRING") {}
    Variable(const std::string& Type, const std::string& Value) : Type(Type), Value(Value) {}

    std::string Type;
    std::string Value;
};

struct FuncReturn
{
    NsbFile* pScript;
    uint32_t SourceLine;
};

class NsbInterpreter
{
public:
    NsbInterpreter(Game* pGame, ResourceMgr* pResourceMgr, const std::string& InitScript);
    ~NsbInterpreter();

    void Run();
    void CallScript(const std::string& FileName);
    void LoadScript(const std::string& FileName);

private:
    bool Boolify(const std::string& String);

    template <class T> T* GetHandle(const std::string& Identifier);
    template <class T> T GetVariable(const std::string& Identifier);
    void SetVariable(const std::string& Identifier, const Variable& Var);
    bool CallFunction(NsbFile* pDestScript, const char* FuncName); // Obsolete?

    void LoadMovie(const std::string& HandleName, int32_t Priority, int32_t x,
                   int32_t y, bool Loop, bool unk0, const std::string& File, bool unk1);

    void NsbAssert(bool expr, const char* fmt);
    void NsbAssert(const char* fmt);
    template<typename T, typename... A> void NsbAssert(bool expr, const char* fmt, T value, A... args);

    Game* pGame;
    ResourceMgr* pResourceMgr;
    NsbFile* pScript;
    bool EndHack;

    std::stack<FuncReturn> Returns;
    std::vector<NsbFile*> LoadedScripts;
    std::map<std::string, Variable> Variables;
    std::map<std::string, void*> Handles;
    std::vector<Variable> Params;
};

#endif
