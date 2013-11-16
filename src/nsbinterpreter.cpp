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
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <sfeMovie/Movie.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Audio/Music.hpp>

NsbInterpreter::NsbInterpreter(Game* pGame, ResourceMgr* pResourceMgr, const string& InitScript) :
pGame(pGame),
pResourceMgr(pResourceMgr),
pScript(pResourceMgr->GetResource<NsbFile>(InitScript)),
StopInterpreter(false),
ScriptThread(&NsbInterpreter::ThreadMain, this)
{
    // Global variable (hack)
    SetVariable("OutRight", {"INT", "0"});
}

NsbInterpreter::~NsbInterpreter()
{
    delete pResourceMgr;
}

void NsbInterpreter::ThreadMain()
{
    // TODO: from .map file
    LoadScript("nss/function_steinsgate.nsb");
    LoadScript("nss/function.nsb");
    LoadScript("nss/extra_achievements.nsb");
    LoadScript("nss/function_select.nsb");
    //LoadScript("nss/function_stand.nsb");

    do
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // yield? mutex?
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
    while (RunInterpreter)
    {
        pLine = pScript->GetNextLine();
        NsbAssert(pLine, "Interpreting null line");
        NsbAssert(pScript, "Interpreting null script");

        switch (pLine->Magic)
        {
            case uint16_t(MAGIC_CREATE_BOX):
                CreateBox(GetParam<string>(0), GetParam<int32_t>(1),
                          GetParam<int32_t>(2), GetParam<int32_t>(3),
                          GetParam<int32_t>(4), GetParam<int32_t>(5),
                          Boolify(GetParam<string>(6)));
                break;
            case uint16_t(MAGIC_ARRAY_READ):
                ArrayRead(GetParam<string>(0), GetParam<int32_t>(1));
                break;
            case uint16_t(MAGIC_CREATE_ARRAY):
                for (uint32_t i = 1; i < Params.size(); ++i)
                    Arrays[GetParam<string>(0)].Members.push_back(std::make_pair(string(), ArrayVariable(Params[i])));
                break;
            case uint16_t(MAGIC_BIND_IDENTIFIER):
                BindIdentifier(GetParam<string>(0));
                break;
            case uint16_t(MAGIC_CREATE_COLOR): break;
                pGame->GLCallback(std::bind(&NsbInterpreter::CreateColor, this,
                                  GetParam<string>(0), GetParam<int32_t>(1),
                                  GetParam<int32_t>(2), GetParam<int32_t>(3),
                                  GetParam<int32_t>(4), GetParam<int32_t>(5),
                                  GetParam<string>(6)));
                break;
            case uint16_t(MAGIC_SET_TEXTBOX_ATTRIBUTES):
                SetTextboxAttributes(GetParam<string>(0), GetParam<int32_t>(1),
                                     GetParam<string>(2), GetParam<int32_t>(3),
                                     GetParam<string>(4), GetParam<string>(5),
                                     GetParam<int32_t>(6), GetParam<string>(7));
                break;
            case uint16_t(MAGIC_SET_FONT_ATTRIBUTES):
                SetFontAttributes(GetParam<string>(0), GetParam<int32_t>(1),
                                  GetParam<string>(2), GetParam<string>(3),
                                  GetParam<int32_t>(4), GetParam<string>(5));
                break;
            case uint16_t(MAGIC_DESTROY):
                pGame->GLCallback(std::bind(&NsbInterpreter::Destroy, this, GetParam<string>(0)));
                break;
            case uint16_t(MAGIC_SET_AUDIO_STATE):
                SetAudioState(GetParam<string>(0), GetParam<int32_t>(1),
                              GetParam<int32_t>(2), GetParam<string>(3));
                break;
            case uint16_t(MAGIC_SET_AUDIO_LOOP):
                SetAudioLoop(GetParam<string>(0), Boolify(GetParam<string>(1)));
                break;
            case uint16_t(MAGIC_SET_AUDIO_RANGE): break; // SFML bug #203
                SetAudioRange(GetParam<string>(0), GetParam<int32_t>(1), GetParam<int32_t>(2));
                break;
            case uint16_t(MAGIC_LOAD_AUDIO):
                LoadAudio(GetParam<string>(0), GetParam<string>(1), GetParam<string>(2) + ".ogg");
                break;
            case uint16_t(MAGIC_START_ANIMATION):
                StartAnimation(GetParam<string>(0), GetParam<int32_t>(1), GetParam<int32_t>(2),
                               GetParam<int32_t>(3), GetParam<string>(4), Boolify(GetParam<string>(5)));
                break;
            case uint16_t(MAGIC_UNK29):
                // This is (mistakenly) done by MAGIC_CALL
                //SetVariable(pLine->Params[0], {"STRING", GetVariable<string>(pLine->Params[1])});
                break;
            case uint16_t(MAGIC_SLEEP_MS):
                Sleep(GetVariable<int32_t>(Params[0].Value));
                break;
            case uint16_t(MAGIC_GET_MOVIE_TIME):
                GetMovieTime(GetParam<string>(0));
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
                        goto found;

                std::cerr << "Failed to lookup function symbol " << FuncName << std::endl;
                found:
                break;
            }
            case uint16_t(MAGIC_UNK5):
                Params[0] = {"STRING", string()}; // Hack
                break;
            case uint16_t(MAGIC_TEXT):
                ParseText(GetParam<string>(0), GetParam<string>(1), GetParam<string>(2));
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
                                  GetParam<string>(0), GetParam<int32_t>(1),
                                  GetParam<int32_t>(2), GetParam<int32_t>(3),
                                  Boolify(GetParam<string>(4)), Boolify(GetParam<string>(5)),
                                  GetParam<string>(6), Boolify(GetParam<string>(7))));
                return;
            }
            case uint16_t(MAGIC_LOAD_TEXTURE):
            {
                int32_t unk1, unk2;
                if (Params[2].Type == "STRING")
                    unk1 = 0;
                else
                    unk1 = GetParam<int32_t>(2);
                if (Params[3].Type == "STRING")
                    unk2 = 0;
                else
                    unk2 = GetParam<int32_t>(3);

                pGame->GLCallback(std::bind(&NsbInterpreter::LoadTexture, this,
                                  GetParam<string>(0), GetParam<int32_t>(1),
                                  unk1, unk2, GetParam<string>(4)));
                break;
            }
            case uint16_t(MAGIC_DISPLAY):
                Display(GetParam<string>(0), GetParam<int32_t>(1),
                        GetParam<int32_t>(2), GetParam<string>(3),
                        Boolify(GetParam<string>(4)));
                break;
            case uint16_t(MAGIC_SET_DISPLAY_STATE):
                SetDisplayState(GetParam<string>(0), GetParam<string>(1));
                break;
            case uint16_t(MAGIC_UNK12):
                return;
            case uint16_t(MAGIC_UNK3):
            case uint16_t(MAGIC_UNK6):
                Params.clear();
                ArrayParams.clear();
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

template <class T> T NsbInterpreter::GetVariable(const string& Identifier)
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

template <class T> T NsbInterpreter::GetParam(int32_t Index)
{
    return GetVariable<T>(pLine->Params[Index]);
}

void NsbInterpreter::CreateBox(const string& HandleName, int32_t unk0, int32_t x, int32_t y, int32_t Width, int32_t Height, bool unk1)
{
}

void NsbInterpreter::BindIdentifier(const string& /*HandleName*/)
{
    ArrayVariable* Var = ArrayParams[ArrayParams.size() - 1];
    for (uint32_t i = 1; i < Params.size(); ++i)
        Var->Members[i - 1].first = Params[i].Value;
}

void NsbInterpreter::ArrayRead(const string& HandleName, int32_t Depth)
{
    const string* MemberName = &HandleName;
    ArrayVariable* pVariable = nullptr;

    while (Depth --> 0) // Depth goes to zero; 'cause recursion is too mainstream
    {
        ArrayMembers& Members = Arrays[*MemberName].Members;
        for (uint32_t i = 0; i < Members.size(); ++i)
        {
            if (Members[i].first == Params[Params.size() - Depth - 2].Value)
            {
                MemberName = &Members[i].first;
                pVariable = &Members[i].second;
                break;
            }
        }
    }

    if (!pVariable)
        return;

    ArrayParams.push_back(pVariable);
    Params.push_back(*pVariable);
}

void NsbInterpreter::CreateColor(const string& HandleName, int32_t Priority, int32_t unk0, int32_t unk1,
                                 int32_t Width, int32_t Height, const string& Color)
{
    if (Drawable* pDrawable = CacheHolder<Drawable>::Read(HandleName))
    {
        pGame->RemoveDrawable(pDrawable);
        delete pDrawable;
    }

    std::stringstream ss(Color);
    uint32_t IntColor;
    ss >> std::hex >> IntColor;
    sf::Image ColorImage;
    ColorImage.create(Width, Height, sf::Color(IntColor / 0x10000, (IntColor / 0x100) % 0x100, IntColor % 0x100));
    sf::Texture* pTexture = new sf::Texture;
    NsbAssert(pTexture->loadFromImage(ColorImage), "Failed to create color % texture to handle %.", Color, HandleName);
    CacheHolder<Drawable>::Write(HandleName, new Drawable(new sf::Sprite(*pTexture), Priority, DRAWABLE_TEXTURE));
}


void NsbInterpreter::SetTextboxAttributes(const string& Handle, int32_t unk0,
                                          const string& Font, int32_t unk1,
                                          const string& Color1, const string& Color2,
                                          int32_t unk2, const string& unk3)
{
}

void NsbInterpreter::SetFontAttributes(const string& Font, int32_t size,
                                       const string& Color1, const string& Color2,
                                       int32_t unk0, const string& unk1)
{
}

void NsbInterpreter::SetAudioState(const string& HandleName, int32_t NumSeconds,
                                   int32_t Volume, const string& Tempo)
{
}

void NsbInterpreter::SetAudioLoop(const string& HandleName, bool Loop)
{
    if (sf::Music* pMusic = CacheHolder<sf::Music>::Read(HandleName))
        pMusic->setLoop(Loop);
}

void NsbInterpreter::Destroy(string& HandleName)
{
    // Handle wildcard
    if (HandleName[HandleName.size() - 1] == '*')
        HandleName.pop_back();

    if (Drawable* pDrawable = CacheHolder<Drawable>::Read(HandleName))
    {
        CacheHolder<Drawable>::Write(HandleName, nullptr);
        pGame->RemoveDrawable(pDrawable);
        delete pDrawable;
    }
}

void NsbInterpreter::LoadAudio(const string& HandleName, const string& Type, const string& File)
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

void NsbInterpreter::SetAudioRange(const string& HandleName, int32_t begin, int32_t end)
{
    if (sf::Music* pMusic = CacheHolder<sf::Music>::Read(HandleName))
        pMusic->setPlayingOffset(sf::milliseconds(begin));
}

void NsbInterpreter::StartAnimation(const string& HandleName, int32_t TimeRequired,
                                    int32_t x, int32_t y, const string& Tempo, bool Wait)
{
    if (Drawable* pDrawable = CacheHolder<Drawable>::Read(HandleName))
    {
        if (pDrawable->Type == DRAWABLE_TEXTURE)
            ((sf::Sprite*)pDrawable->Get())->setPosition(x, y);
        else if (pDrawable->Type == DRAWABLE_MOVIE)
            ((sfe::Movie*)pDrawable->Get())->setPosition(x, y);
    }
}

void NsbInterpreter::ParseText(const string& unk0, const string& unk1, const string& Text)
{
    std::cout << unk0 << " " << unk1 << " " << Text << std::endl;
    std::cin.get();
}

void NsbInterpreter::Sleep(int32_t ms)
{
    std::cout << "Sleeping for " << ms << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void NsbInterpreter::SetVariable(const string& Identifier, const Variable& Var)
{
    Variables[Identifier] = Var;
}

void NsbInterpreter::GetMovieTime(const string& HandleName)
{
    Params.clear();
    if (Drawable* pDrawable = CacheHolder<Drawable>::Read(HandleName))
    {
        if (sfe::Movie* pMovie = dynamic_cast<sfe::Movie*>(pDrawable->Get()))
            Params.push_back({"INT", boost::lexical_cast<string>(pMovie->getDuration().asMilliseconds())});
        else
            std::cout << "Failed to get movie duration because Drawable is not movie" << std::endl;
    }
    else
        std::cout << "Failed to get movie time because there is no Drawable " << HandleName << std::endl;
}

bool NsbInterpreter::Boolify(const string& String)
{
    if (String == "true")
        return true;
    else if (String == "false")
        return false;
    NsbAssert(false, "Invalid boolification of string: ", String.c_str());
    return false; // Silence gcc
}

void NsbInterpreter::SetDisplayState(const string& HandleName, const string& State)
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
    if (sf::Music* pMusic = CacheHolder<sf::Music>::Read(HandleName))
        if (State == "Play")
            pMusic->play();
}

// Display($ColorNut, 処理時間, 1000, テンポ, 待ち);
void NsbInterpreter::Display(const string& HandleName, int32_t unk0, int32_t unk1, const string& Tempo, bool Wait)
{
    if (unk1 > 0)
        if (Drawable* pDrawable = CacheHolder<Drawable>::Read(HandleName))
            if (pDrawable->Type == DRAWABLE_TEXTURE)
                pGame->AddDrawable(pDrawable);
}

void NsbInterpreter::LoadMovie(const string& HandleName, int32_t Priority, int32_t x,
                               int32_t y, bool Loop, bool unk0, const string& File, bool unk1)
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

void NsbInterpreter::LoadTexture(const string& HandleName, int32_t Priority, int32_t x, int32_t y, const string& File)
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
        delete pTexture;
        CacheHolder<Drawable>::Write(HandleName, nullptr);
        return;
    }
    NsbAssert(pTexture->loadFromMemory(pTexData, Size), "Failed to load texture %!", File);

    sf::Sprite* pSprite = new sf::Sprite(*pTexture);
    pSprite->setPosition(x, y);
    CacheHolder<Drawable>::Write(HandleName, new Drawable(pSprite, Priority, DRAWABLE_TEXTURE));
}

void NsbInterpreter::LoadScript(const string& FileName)
{
    LoadedScripts.push_back(pResourceMgr->GetResource<NsbFile>(FileName));
}

void NsbInterpreter::CallScript(const string& FileName)
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
    std::cout << "\nCRASH:\n**STACK TRACE BEGIN**\n";
    std::stack<FuncReturn> Stack = Returns;
    while (!Stack.empty())
    {
        std::cout << Stack.top().pScript->GetName() << " at " << Stack.top().SourceLine << std::endl;
        Stack.pop();
    }
    std::cout << "**STACK TRACE END**\nRecovering...\n" << std::endl;
}

void NsbInterpreter::Abort()
{
#ifdef DEBUG
    abort();
#else
    Recover();
#endif
}

void NsbInterpreter::Recover()
{
    while (Line* pLine = pScript->GetNextLine())
        if (pLine->Magic == MAGIC_UNK6)
            break;
    pScript->SetSourceIter(pScript->GetNextLineEntry() - 2);
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
                std::cout << value << std::endl;
                NsbAssert(false, fmt + 1, args...); // call even when *s == 0 to detect extra arguments
                DumpTrace();
                Abort();
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
    Abort();
}
