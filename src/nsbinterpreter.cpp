/* 
 * libnpengine: Nitroplus script interpreter
 * Copyright (C) 2013 Mislav Blažević <krofnica996@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * */
#include "nsbfile.hpp"
#include "game.hpp"
#include "resourcemgr.hpp"
#include "nsbmagic.hpp"

#include <iostream>
#include <boost/lexical_cast.hpp>
#include <sfeMovie/Movie.hpp>
#include <SFML/Graphics/Sprite.hpp>

NsbInterpreter::NsbInterpreter(Game* pGame, ResourceMgr* pResourceMgr, const std::string& InitScript) :
pGame(pGame),
pResourceMgr(pResourceMgr),
EndHack(false)
{
    CallScript(InitScript);

    // Global variable (hack)
    SetVariable("OutRight", {"INT", "0"});

    // TODO: from .map file
    LoadScript("nss/function_steinsgate.nsb");
    LoadScript("nss/function.nsb");
    LoadScript("nss/extra_achievements.nsb");
    LoadScript("nss/function_select.nsb");
    //LoadScript("nss/function_stand.nsb");
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

    NsbAssert(pScript, "Interpreting null script");

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
                std::cout << "Calling function " << FuncName << " in " << pScript->GetName() << " at " << pScript->GetNextLineEntry() << std::endl;

                // Find function locally
                if (CallFunction(pScript, FuncName))
                    return;

                // Find function globally
                for (uint32_t i = 0; i < LoadedScripts.size(); ++i)
                    if (CallFunction(LoadedScripts[i], FuncName))
                        return;

                std::cerr << "Failed to lookup function symbol " << FuncName << std::endl;
                break;
            }
            case uint16_t(MAGIC_UNK5):
                Params[0] = {"STRING", std::string()}; // Hack
                break;
            case uint16_t(MAGIC_BEGIN):
                // Turn params into variables
                for (uint32_t i = 1; i < pLine->Params.size(); ++i)
                    SetVariable(pLine->Params[i], Params[i - 1]);
                break;
            case uint16_t(MAGIC_END):
                NsbAssert(!Returns.empty(), "Empty return stack");
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
                NsbAssert(Params[First].Type == Params[Second].Type,
                          "Concating params of different types (% and %) in % at %",
                          Params[First].Type,
                          Params[Second].Type,
                          pScript->GetName(),
                          pScript->GetNextLineEntry());
                NsbAssert(Params[First].Type == "STRING",
                          "Concating non-STRING params in % at %",
                          pScript->GetName(),
                          pScript->GetNextLineEntry());
                Params[First].Value += Params[Second].Value;
                Params.resize(Second);
                break;
            }
            case uint16_t(MAGIC_LOAD_MOVIE):
            {
                LoadMovie(GetVariable<std::string>(pLine->Params[0]),
                          GetVariable<int32_t>(pLine->Params[1]),
                          GetVariable<int32_t>(pLine->Params[2]),
                          GetVariable<int32_t>(pLine->Params[3]),
                          Boolify(GetVariable<std::string>(pLine->Params[4])),
                          Boolify(GetVariable<std::string>(pLine->Params[5])),
                          GetVariable<std::string>(pLine->Params[6]),
                          Boolify(GetVariable<std::string>(pLine->Params[7])));
                break;
            }
            case uint16_t(MAGIC_LOAD_TEXTURE):
            {
                int32_t unk1, unk2;
                if (Params[2].Type == "STRING")
                    unk1 = 0;
                else
                    unk1 = GetVariable<int32_t>(pLine->Params[2]);
                if (Params[3].Type == "STRING")
                    unk2 = 0;
                else
                    unk2 = GetVariable<int32_t>(pLine->Params[3]);

                LoadTexture(GetVariable<std::string>(pLine->Params[0]),
                            GetVariable<int32_t>(pLine->Params[1]),
                            unk1,
                            unk2,
                            GetVariable<std::string>(pLine->Params[4]));
                break;
            }
            case uint16_t(MAGIC_DISPLAY): break; // Doesn't seem right....
                Display(GetVariable<std::string>(pLine->Params[0]),
                        GetVariable<int32_t>(pLine->Params[1]),
                        GetVariable<int32_t>(pLine->Params[2]),
                        GetVariable<std::string>(pLine->Params[3]),
                        GetVariable<std::string>(pLine->Params[4]));
                break;
            case uint16_t(MAGIC_UNK12):
                return;
            case uint16_t(MAGIC_UNK3):
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
        return boost::lexical_cast<T>(Identifier);
    return boost::lexical_cast<T>(iter->second.Value);
}

void NsbInterpreter::SetVariable(const std::string& Identifier, const Variable& Var)
{
    Variables[Identifier] = Var;
}

bool NsbInterpreter::Boolify(const std::string& String)
{
    if (String == "true")
        return true;
    else if (String == "false")
        return false;
    NsbAssert(false, "Invalid boolification of string: ", String.c_str());
    return false; // Silence gcc
}

void NsbInterpreter::Display(const std::string& HandleName, int32_t unk0, int32_t unk1,
                             const std::string& unk2, const std::string& unk3)
{
    if (Drawable* pDrawable = CacheHolder<Drawable>::Read(HandleName))
        pGame->AddDrawable(pDrawable);
}

void NsbInterpreter::LoadMovie(const std::string& HandleName, int32_t Priority, int32_t x,
                               int32_t y, bool Loop, bool unk0, const std::string& File, bool unk1)
{
    if (Drawable* pDrawable = CacheHolder<Drawable>::Read(HandleName))
    {
        pGame->RemoveDrawable(pDrawable);
        delete pDrawable;
    }

    sfe::Movie* pMovie = new sfe::Movie;
    pMovie->openFromFile(File);
    pMovie->setPosition(x, y);
    // pMovie->setLoop(Loop);

    // Abuse CacheHolder as HandleHolder :)
    CacheHolder<Drawable>::Write(HandleName, new Drawable(pMovie, Priority, true, DRAWABLE_MOVIE));
}

void NsbInterpreter::LoadTexture(const std::string& HandleName, int32_t unk0, int32_t unk1,
                                 int32_t unk2, const std::string& File)
{
    if (Drawable* pDrawable = CacheHolder<Drawable>::Read(HandleName))
    {
        pGame->RemoveDrawable(pDrawable);
        delete pDrawable;
    }

    sf::Texture* pTexture = new sf::Texture;
    uint32_t Size;
    char* pTexData = pResourceMgr->Read(File, &Size);
    if (!pTexData)
    {
        std::cout << "Failed to read texture " << File << std::endl;
        return;
    }
    NsbAssert(pTexture->loadFromMemory(pTexData, Size), "Failed to load texture %!", File);
    CacheHolder<Drawable>::Write(HandleName, new Drawable(new sf::Sprite(*pTexture), 0, false, DRAWABLE_TEXTURE));
    Display(HandleName, 0, 0, "", ""); // Hack
}

void NsbInterpreter::LoadScript(const std::string& FileName)
{
    LoadedScripts.push_back(pResourceMgr->GetResource<NsbFile>(FileName));
}

void NsbInterpreter::CallScript(const std::string& FileName)
{
    pScript = pResourceMgr->GetResource<NsbFile>(FileName);
}

bool NsbInterpreter::CallFunction(NsbFile* pDestScript, const char* FuncName)
{
    if (uint32_t FuncLine = pDestScript->GetFunctionLine(FuncName))
    {
        Returns.push({pScript, pScript->GetNextLineEntry()});
        pScript = pDestScript;
        pScript->SetSourceIter(FuncLine - 1);
        return true;
    }
    return false;
}

void NsbInterpreter::NsbAssert(const char* fmt)
{
    std::cout << fmt << std::endl;
}

template<typename T, typename... A>
void NsbInterpreter::NsbAssert(bool expr, const char* fmt, T value, A... args)
{
    if (expr)
        return;

    while (*fmt)
    {
        if (*fmt == '%')
        {
            if (*(fmt + 1) == '%')
                ++fmt;
            else
            {
                std::cout << value << std::flush;
                NsbAssert(false, fmt + 1, args...); // call even when *s == 0 to detect extra arguments
                abort();
            }
        }
        std::cout << *fmt++;
    }

    abort();
}

void NsbInterpreter::NsbAssert(bool expr, const char* fmt)
{
    if (!expr)
    {
        NsbAssert(fmt);
        abort();
    }
}
