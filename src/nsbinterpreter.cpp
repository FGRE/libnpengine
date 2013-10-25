#include "nsbfile.hpp"
#include "movie.hpp"
#include "game.hpp"
#include "resourcemgr.hpp"
#include "nsbmagic.hpp"

#include <iostream>
#include <boost/lexical_cast.hpp>

NsbInterpreter::NsbInterpreter(Game* pGame, ResourceMgr* pResourceMgr, const std::string& InitScript) :
pGame(pGame),
pResourceMgr(pResourceMgr)
{
    ScriptStack.push(pResourceMgr->GetResource<NsbFile>(InitScript));
}

NsbInterpreter::~NsbInterpreter()
{
    delete pResourceMgr;
}

void NsbInterpreter::Run()
{
    NsbFile* pScript = ScriptStack.top();
    while (Line* pLine = pScript->GetNextLine())
    {
        switch (pLine->Magic)
        {
            case uint16_t(MAGIC_CALL):
                if (uint32_t FuncLine = pScript->GetFunctionLine(pLine->Params[0].c_str()))
                {
                    ReturnLines.push(pScript->GetNextLineEntry());
                    pScript->SetSourceIter(FuncLine);
                }
                else
                    std::cout << "Attempted to call unknown function: " << pLine->Params[0] << std::endl;
                break;
            case uint16_t(MAGIC_END):
                pScript->SetSourceIter(ReturnLines.top());
                ReturnLines.pop();
                break;
            case uint16_t(MAGIC_LOAD_MOVIE):
                LoadMovie(pLine->Params[0],
                          boost::lexical_cast<int32_t>(pLine->Params[1]),
                          boost::lexical_cast<int32_t>(pLine->Params[2]),
                          boost::lexical_cast<int32_t>(pLine->Params[3]),
                          Boolify(pLine->Params[4]),
                          Boolify(pLine->Params[5]),
                          Boolify(pLine->Params[6]),
                          pLine->Params[7]);
                break;
            default:
                std::cout << "Unknown magic: " << std::hex << pLine->Magic << std::dec << std::endl;
        }
    }
    std::cout << "Unexpected end of script!" << std::endl;
}

template <class T> T* NsbInterpreter::GetVariable(const std::string& Identifier) const
{
    auto iter = Variables.find(Identifier);
    if (iter == Variables.end())
        return nullptr;
    return boost::any_cast<T*>(iter->second);
}

bool NsbInterpreter::Boolify(const std::string& String)
{
    if (String == "true")
        return true;
    else if (String == "false")
        return false;
    std::cout << "Invalid boolification of string: " << String << std::endl;
    return false;
}

void NsbInterpreter::LoadMovie(const std::string& HandleName, int32_t Priority, int32_t x,
                               int32_t y, bool Loop, bool unk0, bool unk1, const std::string& File)
{
    if (Movie* pOld = GetVariable<Movie>(HandleName))
        delete pOld;

    Movie* pMovie = new Movie(x, y, Loop, File);
    Variables[HandleName] = boost::any(pMovie);

    pGame->AddDrawable({pMovie, Priority}); // Not sure about this...
}
