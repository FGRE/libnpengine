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
#include <chrono>
#include <boost/lexical_cast.hpp>
#include <sfeMovie/Movie.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Audio/Music.hpp>

NsbInterpreter::NsbInterpreter(Game* pGame, ResourceMgr* pResourceMgr, const std::string& InitScript) :
pGame(pGame),
pResourceMgr(pResourceMgr),
StopInterpreter(false),
ScriptThread(&NsbInterpreter::ThreadMain, this)
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

void NsbInterpreter::ThreadMain()
{
    do
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        Run();
    } while (!StopInterpreter);
}

void NsbInterpreter::Stop()
{
    StopInterpreter = true;
}

void NsbInterpreter::Pause()
{
    RunInterpreter = false;
}

void NsbInterpreter::Start()
{
    RunInterpreter = true;
}

void NsbInterpreter::Run()
{
    NsbAssert(pScript, "Interpreting null script");

    while (RunInterpreter)
    {
        Line* pLine = pScript->GetNextLine();
        NsbAssert(pLine, "Interpreting null line");

        switch (pLine->Magic)
        {
            case uint16_t(MAGIC_SET_AUDIO_RANGE): break; // SFML bug #203
                SetAudioRange(GetVariable<std::string>(pLine->Params[0]),
                              GetVariable<int32_t>(pLine->Params[1]),
                              GetVariable<int32_t>(pLine->Params[2]));
                break;
            case uint16_t(MAGIC_LOAD_AUDIO):
                LoadAudio(GetVariable<std::string>(pLine->Params[0]),
                          GetVariable<std::string>(pLine->Params[1]),
                          GetVariable<std::string>(pLine->Params[2]) + ".ogg");
                break;
            case uint16_t(MAGIC_START_ANIMATION):
                StartAnimation(GetVariable<std::string>(pLine->Params[0]),
                               GetVariable<int32_t>(pLine->Params[1]),
                               GetVariable<int32_t>(pLine->Params[2]),
                               GetVariable<int32_t>(pLine->Params[3]),
                               GetVariable<std::string>(pLine->Params[4]),
                               Boolify(GetVariable<std::string>(pLine->Params[5])));
                break;
            case uint16_t(MAGIC_UNK29):
                // This is (mistakenly) done by MAGIC_PARAM
                //SetVariable(pLine->Params[0], {"STRING", GetVariable<std::string>(pLine->Params[1])});
                break;
            case uint16_t(MAGIC_SLEEP):
                Sleep(GetVariable<int32_t>(Params[0].Value));
                break;
            case uint16_t(MAGIC_GET_MOVIE_TIME):
                GetMovieTime(GetVariable<std::string>(pLine->Params[0]));
                break;
            case uint16_t(MAGIC_CALL_SCRIPT):
                // TODO: extract entry function & convert nss to nsb
                //CallScript(pLine->Params[0]);
                break;
            case uint16_t(MAGIC_CALL):
            {
                const char* FuncName = pLine->Params[0].c_str();
                //std::cout << "Calling function " << FuncName << " in " << pScript->GetName() << " at " << pScript->GetNextLineEntry() << std::endl;

                // Find function override
                if (std::strcmp(FuncName, "MovieWaitSG") == 0)
                {
                    GetMovieTime("ムービー");
                    Sleep(GetVariable<int32_t>(Params[0].Value));
                    pGame->GLCallback(std::bind(&Game::RemoveDrawable, pGame,
                                      CacheHolder<Drawable>::Read("ムービー")));
                    break;
                }

                // Find function locally
                if (CallFunction(pScript, FuncName))
                    break;

                // Find function globally
                for (uint32_t i = 0; i < LoadedScripts.size(); ++i)
                    if (CallFunction(LoadedScripts[i], FuncName))
                        break;

                //std::cerr << "Failed to lookup function symbol " << FuncName << std::endl;
                break;
            }
            case uint16_t(MAGIC_UNK5):
                Params[0] = {"STRING", std::string()}; // Hack
                break;
            case uint16_t(MAGIC_TEXT):
                ParseText(GetVariable<std::string>(pLine->Params[0]),
                          GetVariable<std::string>(pLine->Params[1]),
                          GetVariable<std::string>(pLine->Params[2]));
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
                pGame->GLCallback(std::bind(&NsbInterpreter::LoadMovie, this,
                                  GetVariable<std::string>(pLine->Params[0]),
                                  GetVariable<int32_t>(pLine->Params[1]),
                                  GetVariable<int32_t>(pLine->Params[2]),
                                  GetVariable<int32_t>(pLine->Params[3]),
                                  Boolify(GetVariable<std::string>(pLine->Params[4])),
                                  Boolify(GetVariable<std::string>(pLine->Params[5])),
                                  GetVariable<std::string>(pLine->Params[6]),
                                  Boolify(GetVariable<std::string>(pLine->Params[7]))));
                return;
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

                pGame->GLCallback(std::bind(&NsbInterpreter::LoadTexture, this,
                                  GetVariable<std::string>(pLine->Params[0]),
                                  GetVariable<int32_t>(pLine->Params[1]),
                                  unk1,
                                  unk2,
                                  GetVariable<std::string>(pLine->Params[4])));
                break;
            }
            case uint16_t(MAGIC_DISPLAY):
                Display(GetVariable<std::string>(pLine->Params[0]),
                        GetVariable<int32_t>(pLine->Params[1]),
                        GetVariable<int32_t>(pLine->Params[2]),
                        GetVariable<std::string>(pLine->Params[3]),
                        Boolify(GetVariable<std::string>(pLine->Params[4])));
                break;
            case uint16_t(MAGIC_SET_DISPLAY_STATE):
                SetDisplayState(GetVariable<std::string>(pLine->Params[0]),
                                GetVariable<std::string>(pLine->Params[1]));
                break;
            case uint16_t(MAGIC_UNK12):
                return;
            case uint16_t(MAGIC_UNK3):
            case uint16_t(MAGIC_UNK6):
                // Guess...
                Params.clear();
                break;
            case uint16_t(MAGIC_CALLBACK):
                pGame->RegisterCallback(static_cast<sf::Keyboard::Key>(pLine->Params[0][0] - 'A'), pLine->Params[1]);
                break;
            default:
                //std::cerr << "Unknown magic: " << std::hex << pLine->Magic << std::dec << std::endl;
                break;
        }
    }
}

template <class T> T NsbInterpreter::GetVariable(const std::string& Identifier)
{
    // NULL object
    if (Identifier == "@")
        return T();

    auto iter = Variables.find(Identifier);

    try
    {
        if (iter == Variables.end())
            return boost::lexical_cast<T>(Identifier);
        return boost::lexical_cast<T>(iter->second.Value);
    }
    catch (...)
    {
        std::cout << "Failed to cast " << Identifier << " to correct type." << std::endl;
        return T();
    }
}

void NsbInterpreter::LoadAudio(const std::string& HandleName, const std::string& Type, const std::string& File)
{
    if (sf::Music* pMusic = CacheHolder<sf::Music>::Read(HandleName))
    {
        pMusic->stop();
        delete pMusic;
    }

    sf::Music* pMusic = new sf::Music;
    uint32_t Size;
    char* pMusicData = pResourceMgr->Read(File, &Size);
    if (!pMusicData)
    {
        std::cout << "Failed to read music " << File << std::endl;
        DumpTrace();
        CacheHolder<sf::Music>::Write(HandleName, nullptr);
        return;
    }
    NsbAssert(pMusic->openFromMemory(pMusicData, Size), "Failed to load music %!", File);
    CacheHolder<sf::Music>::Write(HandleName, pMusic);
}

void NsbInterpreter::SetAudioRange(const std::string& HandleName, int32_t begin, int32_t end)
{
    if (sf::Music* pMusic = CacheHolder<sf::Music>::Read(HandleName))
        pMusic->setPlayingOffset(sf::milliseconds(begin));
}

void NsbInterpreter::StartAnimation(const std::string& HandleName, int32_t TimeRequired,
                                    int32_t x, int32_t y, const std::string& Tempo, bool Wait)
{
    ;
}

void NsbInterpreter::ParseText(const std::string& unk0, const std::string& unk1, const std::string& Text)
{
    std::cout << unk0 << " " << unk1 << " " << Text << std::endl;
    std::cin.get();
}

void NsbInterpreter::Sleep(int32_t ms)
{
    std::cout << "Sleeping for " << ms << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void NsbInterpreter::SetVariable(const std::string& Identifier, const Variable& Var)
{
    Variables[Identifier] = Var;
}

void NsbInterpreter::GetMovieTime(const std::string& HandleName)
{
    Params.clear();
    if (Drawable* pDrawable = CacheHolder<Drawable>::Read(HandleName))
    {
        if (sfe::Movie* pMovie = dynamic_cast<sfe::Movie*>(pDrawable->Get()))
            Params.push_back({"INT", boost::lexical_cast<std::string>(pMovie->getDuration().asMilliseconds())});
        else
            std::cout << "Failed to get movie duration because Drawable is not movie" << std::endl;
    }
    else
        std::cout << "Failed to get movie time because there is no Drawable " << HandleName << std::endl;
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

void NsbInterpreter::SetDisplayState(const std::string& HandleName, const std::string& State)
{
    if (Drawable* pDrawable = CacheHolder<Drawable>::Read(HandleName))
    {
        if (State == "Play")
        {
            if (sfe::Movie* pMovie = dynamic_cast<sfe::Movie*>(pDrawable->Get()))
            {
                pGame->AddDrawable(pDrawable);
                pMovie->play();
            }
            else
                NsbAssert(false, "Attempted to Play non-movie object ", HandleName);
        }

    }
}

void NsbInterpreter::Display(const std::string& HandleName, int32_t unk0, int32_t unk1,
                             const std::string& unk2, bool unk3)
{
    if (unk1 > 0)
        if (Drawable* pDrawable = CacheHolder<Drawable>::Read(HandleName))
            if (pDrawable->Type == DRAWABLE_TEXTURE)
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
    CacheHolder<Drawable>::Write(HandleName, new Drawable(pMovie, Priority, DRAWABLE_MOVIE));
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
        CacheHolder<Drawable>::Write(HandleName, nullptr);
        return;
    }
    NsbAssert(pTexture->loadFromMemory(pTexData, Size), "Failed to load texture %!", File);
    CacheHolder<Drawable>::Write(HandleName, new Drawable(new sf::Sprite(*pTexture), 0, DRAWABLE_TEXTURE));
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

void NsbInterpreter::DumpTrace()
{
    std::cout << "**STACK TRACE BEGIN**" << std::endl;
    std::stack<FuncReturn> Stack = Returns;
    while (!Stack.empty())
    {
        std::cout << Stack.top().pScript->GetName() << " at " << Stack.top().SourceLine << std::endl;
        Stack.pop();
    }
    std::cout << "STACK TRACE END**" << std::endl;
}

// Rename/eliminate pls?
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
                DumpTrace();
                abort();
            }
        }
        std::cout << *fmt++;
    }
}

void NsbInterpreter::NsbAssert(bool expr, const char* fmt)
{
    if (expr)
        return;

    NsbAssert(fmt);
    DumpTrace();
    abort();
}
