#include "nsbfile.hpp"
#include "movie.hpp"
#include "game.hpp"
#include "resourcemgr.hpp"
#include "nsbmagic.hpp"

#include <iostream>
#include <boost/lexical_cast.hpp>

NsbInterpreter::NsbInterpreter(Game* pGame, ResourceMgr* pResourceMgr, const std::string& InitScript) :
pGame(pGame),
pResourceMgr(pResourceMgr),
EndHack(false)
{
    CallScript(InitScript);

    // TODO: from .map file
    LoadScript("nss/function_steinsgate.nsb");
}

NsbInterpreter::~NsbInterpreter()
{
    delete pResourceMgr;
}

void NsbInterpreter::Run()
{
    // This is... a hack.
    if (EndHack)
        return;

    NsbFile* pScript = ScriptStack.top();
    assert(pScript && "Interpreting null script");

    while (Line* pLine = pScript->GetNextLine())
    {
        switch (pLine->Magic)
        {
            case uint16_t(MAGIC_CALL_SCRIPT):
                // TODO: extract entry function & convert nss to nsb
                //CallScript(pLine->Params[0]);
                return;
            case uint16_t(MAGIC_CALL):
            {
                const char* FuncName = pLine->Params[0].c_str();
                if (CallFunction(pScript, FuncName))
                    return;

                for (uint32_t i = 0; i < LoadedScripts.size(); ++i)
                    if (CallFunction(LoadedScripts[i], FuncName))
                        return;
                std::cerr << "Attempted to call unknown function: " << FuncName << std::endl;
                break;
            }
            case uint16_t(MAGIC_UNK5):
                Params[0] = { "STRING", std::string() }; // Hack
                break;
            case uint16_t(MAGIC_END):
                ScriptStack.pop();
                pScript = Returns.top().pScript;
                pScript->SetSourceIter(Returns.top().SourceLine);
                Returns.pop();
                break;
            case uint16_t(MAGIC_SET):
                SetVariable(pLine->Params[0], Params[0]);
                break;
            case uint16_t(MAGIC_GET):
                Params.push_back(Variables[pLine->Params[0]]);
                break;
            case uint16_t(MAGIC_PARAM):
                Params.push_back({pLine->Params[0], pLine->Params[1]});
                break;
            case uint16_t(MAGIC_CONCAT):
            {
                uint32_t First = Params.size() - 2, Second = Params.size() - 1;
                assert(Params[First].Type == Params[Second].Type && "Concating params of different types");
                if (Params[First].Type == "STRING")
                    Params[First].Value += Params[Second].Value;
                else
                    assert(false && "Please tell krofna where did you find this");
                Params.resize(Second);
                break;
            }
            case uint16_t(MAGIC_LOAD_MOVIE):
                LoadMovie(pLine->Params[0],
                          boost::lexical_cast<int32_t>(pLine->Params[1]),
                          boost::lexical_cast<int32_t>(pLine->Params[2]),
                          boost::lexical_cast<int32_t>(pLine->Params[3]),
                          Boolify(pLine->Params[4]),
                          Boolify(pLine->Params[5]),
                          GetVariable<std::string>(pLine->Params[6]),
                          Boolify(pLine->Params[7]));
                break;
            case uint16_t(MAGIC_UNK12):
                EndHack = true;
                return;
            case uint16_t(MAGIC_UNK6):
                // Guess...
                Params.clear();
                return;
            case uint16_t(MAGIC_CALLBACK):
                pGame->RegisterCallback(static_cast<sf::Keyboard::Key>(pLine->Params[0][0] - 'A'), pLine->Params[1]);
                break;
            default:
                //std::cerr << "Unknown magic: " << std::hex << pLine->Magic << std::dec << std::endl;
                break;
        }
    }
    std::cerr << "Unexpected end of script at line: " << pScript->GetNextLineEntry() - 1 << std::endl;
}

template <class T> T NsbInterpreter::GetVariable(const std::string& Identifier)
{
    auto iter = Variables.find(Identifier);
    if (iter == Variables.end())
        return nullptr;
    return boost::lexical_cast<T>(iter->second.Value);
}

void NsbInterpreter::SetVariable(const std::string& Identifier, const Variable& Var)
{
    Variables.insert(std::pair<std::string, Variable>(Identifier, Var));
}

bool NsbInterpreter::Boolify(const std::string& String)
{
    if (String == "true")
        return true;
    else if (String == "false")
        return false;
    std::cerr << "Invalid boolification of string: " << String << std::endl;
    assert(false);
}

template <class T> T* NsbInterpreter::GetHandle(const std::string& Identifier)
{
    auto iter = Handles.find(Identifier);
    if (iter == Handles.end())
        return nullptr;
    return static_cast<T*>(iter->second);
}

void NsbInterpreter::LoadMovie(const std::string& HandleName, int32_t Priority, int32_t x,
                               int32_t y, bool Loop, bool unk0, const std::string& File, bool unk1)
{
    if (Movie* pOld = GetHandle<Movie>(HandleName))
        delete pOld;

    Movie* pMovie = new Movie(x, y, Loop, File);
    Handles[HandleName] = pMovie;
    pGame->AddDrawable({pMovie, Priority}); // Not sure about this...
}

void NsbInterpreter::LoadScript(const std::string& FileName)
{
    LoadedScripts.push_back(pResourceMgr->GetResource<NsbFile>(FileName));
}

void NsbInterpreter::CallScript(const std::string& FileName)
{
    ScriptStack.push(pResourceMgr->GetResource<NsbFile>(FileName));
}

bool NsbInterpreter::CallFunction(NsbFile* pScript, const char* FuncName)
{
    if (uint32_t FuncLine = pScript->GetFunctionLine(FuncName))
    {
        Returns.push({ScriptStack.top(), ScriptStack.top()->GetNextLineEntry()});
        ScriptStack.push(pScript);
        pScript->SetSourceIter(FuncLine);
        return true;
    }
    return false;
}
