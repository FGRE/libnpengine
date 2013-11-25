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
#include "drawable.hpp"
#include "resourcemgr.hpp"
#include "nsbmagic.hpp"
#include "text.hpp"

#include <iostream>
#include <boost/chrono.hpp>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <sfeMovie/Movie.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Audio/Music.hpp>

#define SPECIAL_POS_NUM 7

enum : int32_t
{
    POS_CENTER = -1,
    POS_IN_BOTTOM = -2,
    POS_MIDDLE = -3,
    POS_ON_LEFT = -4,
    POS_OUT_TOP = -5,
    POS_IN_TOP = -6,
    POS_OUT_RIGHT = -7
};

const std::string SpecialPos[SPECIAL_POS_NUM] =
{
    "Center", "InBottom", "Middle",
    "OnLeft", "OutTop", "InTop",
    "OutRight"
};

std::function<int32_t(int32_t)> SpecialPosTable[SPECIAL_POS_NUM] =
{
  [] (int32_t x) { return WINDOW_WIDTH / 2 - x / 2; },
  [] (int32_t y) { return WINDOW_HEIGHT - y; },
  [] (int32_t y) { return WINDOW_HEIGHT / 2 + y / 2; },
  [] (int32_t x) { return 0; },
  [] (int32_t y) { return 0; },
  [] (int32_t y) { return 0; },
  [] (int32_t x) { return 0; }
};

NsbInterpreter::NsbInterpreter(Game* pGame, const string& InitScript) :
pGame(pGame),
pScript(sResourceMgr->GetResource<NsbFile>(InitScript)),
StopInterpreter(false),
ScriptThread(&NsbInterpreter::ThreadMain, this)
{
#ifdef _WIN32
    Text::Initialize("fonts-japanese-gothic.ttf");
#else
    Text::Initialize("/etc/alternatives/fonts-japanese-gothic.ttf");
#endif
}

NsbInterpreter::~NsbInterpreter()
{
}

void NsbInterpreter::ThreadMain()
{
    // TODO: from .map file
    LoadScript("nss/function_steinsgate.nsb");
    LoadScript("nss/function.nsb");
    LoadScript("nss/extra_achievements.nsb");
    LoadScript("nss/function_select.nsb");
    LoadScript("nss/function_stand.nsb");

    do
    {
        boost::this_thread::sleep_for(boost::chrono::milliseconds(10)); // yield? mutex?
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
            case uint16_t(MAGIC_APPLY_MASK):
                HandleName = GetParam<string>(0);
                pGame->GLCallback(std::bind(&NsbInterpreter::ApplyMask, this,
                                  CacheHolder<Drawable>::Read(HandleName),
                                  GetParam<int32_t>(1), GetParam<int32_t>(2),
                                  GetParam<int32_t>(3), GetParam<string>(4),
                                  GetParam<string>(5), GetParam<bool>(6)));
                break;
            case uint16_t(MAGIC_DISPLAY_TEXT):
                HandleName = GetParam<string>(0);
                DisplayText(GetParam<string>(1));
                return;
            case uint16_t(MAGIC_CREATE_BOX):
                HandleName = GetParam<string>(0);
                CreateBox(GetParam<int32_t>(1), GetParam<int32_t>(2), GetParam<int32_t>(3),
                          GetParam<int32_t>(4), GetParam<int32_t>(5), GetParam<bool>(6));
                break;
            case uint16_t(MAGIC_ARRAY_READ): break;
                ArrayRead(GetParam<string>(0), GetParam<int32_t>(1));
                break;
            case uint16_t(MAGIC_CREATE_ARRAY):
                for (uint32_t i = 1; i < Params.size(); ++i)
                    Arrays[GetParam<string>(0)].Members.push_back(std::make_pair(string(), ArrayVariable(Params[i])));
                break;
            case uint16_t(MAGIC_BIND_IDENTIFIER):
                BindIdentifier(GetParam<string>(0));
                break;
            case uint16_t(MAGIC_CREATE_COLOR):
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
            {
                HandleName = GetParam<string>(0);
                // Hack: Do not destroy * (aka everything)
                if (HandleName.back() == '*' && HandleName.size() != 1)
                {
                    std::cout << "Wildcard destroy " << HandleName << std::endl;
                    WildcardCall(HandleName, [this](Drawable* pDrawable)
                    {
                        pGame->GLCallback(std::bind(&NsbInterpreter::Destroy, this, pDrawable));
                        CacheHolder<Drawable>::Write(HandleName, nullptr);
                    });
                }
                else
                {
                    pGame->GLCallback(std::bind(&NsbInterpreter::Destroy, this, CacheHolder<Drawable>::Read(HandleName)));
                    CacheHolder<Drawable>::Write(HandleName, nullptr);
                }
                break;
            }
            case uint16_t(MAGIC_SET_AUDIO_STATE):
                SetAudioState(GetParam<string>(0), GetParam<int32_t>(1),
                              GetParam<int32_t>(2), GetParam<string>(3));
                break;
            case uint16_t(MAGIC_SET_AUDIO_LOOP):
                SetAudioLoop(GetParam<string>(0), GetParam<bool>(1));
                break;
            case uint16_t(MAGIC_SET_AUDIO_RANGE): break; // SFML bug #203
                SetAudioRange(GetParam<string>(0), GetParam<int32_t>(1), GetParam<int32_t>(2));
                break;
            case uint16_t(MAGIC_LOAD_AUDIO):
                LoadAudio(GetParam<string>(0), GetParam<string>(1), GetParam<string>(2) + ".ogg");
                break;
            case uint16_t(MAGIC_START_ANIMATION):
                StartAnimation(GetParam<string>(0), GetParam<int32_t>(1), GetParam<int32_t>(2),
                               GetParam<int32_t>(3), GetParam<string>(4), GetParam<bool>(5));
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
                pGame->GLCallback(std::bind(&NsbInterpreter::ParseText, this,
                                  GetParam<string>(0), GetParam<string>(1), GetParam<string>(2)));
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
                                  GetParam<bool>(4), GetParam<bool>(5),
                                  GetParam<string>(6), GetParam<bool>(7)));
                return;
            }
            case uint16_t(MAGIC_LOAD_TEXTURE):
            {
                int32_t Pos[2];
                for (int32_t i = 2; i <= 3; ++i)
                {
                    if (Params[i].Type == "STRING")
                    {
                        for (int32_t j = 0; j < SPECIAL_POS_NUM; ++j)
                            if (Params[i].Value == SpecialPos[j])
                                Pos[i - 2] = -(j + 1);
                    }
                    else
                        Pos[i - 2] = GetParam<int32_t>(i);
                }

                pGame->GLCallback(std::bind(&NsbInterpreter::LoadTexture, this,
                                  GetParam<string>(0), GetParam<int32_t>(1),
                                  Pos[0], Pos[1], GetParam<string>(4)));
                break;
            }
            case uint16_t(MAGIC_SET_OPACITY):
            {
                HandleName = GetParam<string>(0);
                if (HandleName.back() == '*')
                {
                    WildcardCall(HandleName, std::bind(&NsbInterpreter::SetOpacity, this,
                                 std::placeholders::_1, GetParam<int32_t>(1), GetParam<int32_t>(2),
                                 GetParam<string>(3), GetParam<bool>(4)));
                }
                else
                    SetOpacity(CacheHolder<Drawable>::Read(HandleName), GetParam<int32_t>(1),
                               GetParam<int32_t>(2), GetParam<string>(3), GetParam<bool>(4));
                break;
            }
            case uint16_t(MAGIC_SET_DISPLAY_STATE):
                SetDisplayState(GetParam<string>(0), GetParam<string>(1));
                break;
            case uint16_t(MAGIC_UNK3):
            case uint16_t(MAGIC_CLEAR_PARAMS):
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

template <class T> void NsbInterpreter::WildcardCall(std::string Handle, T Func)
{
    for (auto i = CacheHolder<Drawable>::ReadFirstMatch(Handle);
         i != CacheHolder<Drawable>::Cache.end();
         i = CacheHolder<Drawable>::ReadNextMatch(Handle, i))
    {
        HandleName = i->first;
        Func(i->second);
    }
}

template <class T> T NsbInterpreter::GetVariable(const string& Identifier)
{
    // NULL object
    if (Identifier == "@")
        return T();

    // Needs special handling, currently a hack
    if (Identifier[0] == '@')
        return boost::lexical_cast<T>(string(Identifier, 1, Identifier.size() - 1));

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

template <> bool NsbInterpreter::GetParam(int32_t Index)
{
    return Boolify(GetParam<string>(Index));
}

void NsbInterpreter::ApplyMask(Drawable* pDrawable, int32_t Time, int32_t Start, int32_t End, const string& Tempo, const string& File, bool Wait)
{
}

void NsbInterpreter::CreateBox(int32_t unk0, int32_t x, int32_t y, int32_t Width, int32_t Height, bool unk1)
{
    sf::IntRect* pRect = new sf::IntRect(x, y, Width, Height);
    CacheHolder<sf::IntRect>::Write(HandleName, pRect);
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

void NsbInterpreter::CreateColor(const string& HandleName, int32_t Priority, int32_t x,
                                 int32_t y, int32_t Width, int32_t Height, string Color)
{
    // Workaround
    if (HandleName == "クリア黒")
        return;

    if (Drawable* pDrawable = CacheHolder<Drawable>::Read(HandleName))
    {
        pGame->RemoveDrawable(pDrawable);
        delete pDrawable;
    }

    uint32_t IntColor;

    std::transform(Color.begin(), Color.end(), Color.begin(), ::tolower);
    if (Color[0] == '#')
    {
        Color = string(Color.c_str() + 1);
        std::stringstream ss(Color);
        ss >> std::hex >> IntColor;
    }
    else
    {
        if (Color == "black")
            IntColor = 0;
        else if (Color == "white")
            IntColor = 0xFFFFFF;
        else if (Color == "blue")
            IntColor = 0xFF;
        else
            NsbAssert(false, "Unknown color: %, ", Color);
    }

    sf::Image ColorImage;
    ColorImage.create(Width, Height, sf::Color(IntColor / 0x10000, (IntColor / 0x100) % 0x100, IntColor % 0x100));
    sf::Texture* pTexture = new sf::Texture;
    NsbAssert(pTexture->loadFromImage(ColorImage), "Failed to create color % texture to handle %.", Color, HandleName);
    sf::Sprite* pSprite = new sf::Sprite(*pTexture);
    pSprite->setPosition(x, y);
    CacheHolder<Drawable>::Write(HandleName, new Drawable(pSprite, Priority, DRAWABLE_TEXTURE));
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
    if (sf::Music* pMusic = CacheHolder<sf::Music>::Read(HandleName))
        pMusic->setVolume(Volume / 10);
}

void NsbInterpreter::SetAudioLoop(const string& HandleName, bool Loop)
{
    if (sf::Music* pMusic = CacheHolder<sf::Music>::Read(HandleName))
        pMusic->setLoop(Loop);
}

void NsbInterpreter::Destroy(Drawable* pDrawable)
{
    if (pDrawable)
    {
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
    char* pMusicData = sResourceMgr->Read(File, &Size);
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
        // TODO: Only if Time == 0, else animate
        if (pDrawable->Type == DRAWABLE_TEXTURE)
            ((sf::Sprite*)pDrawable->Get())->setPosition(x, y);
        else if (pDrawable->Type == DRAWABLE_MOVIE)
            ((sfe::Movie*)pDrawable->Get())->setPosition(x, y);
        else if (pDrawable->Type == DRAWABLE_TEXT)
            ((Text*)pDrawable)->setPosition(x, y);
    }
}

void NsbInterpreter::ParseText(const string& HandleName, const string& Box, const string& XML)
{
    string NewHandle = Box + "/" + HandleName;
    SetVariable("$SYSTEM_present_text", { "STRING", NewHandle });
    std::cout << "Parsing to handle " << NewHandle << std::endl;
    if (Drawable* pText = CacheHolder<Drawable>::Read(NewHandle))
        delete pText;
    Text* pText = new Text(XML);
    CacheHolder<Drawable>::Write(NewHandle, pText);
}

void NsbInterpreter::DisplayText(const string& unk)
{
    if (Text* pText = (Text*)CacheHolder<Drawable>::Read(HandleName))
    {
        if (sf::Music* pMusic = pText->Voices[0].pMusic)
        {
            pMusic->play();
            pText->pCurrentMusic = pMusic;
        }
        pGame->SetText(pText);
    }
    Pause();
}

void NsbInterpreter::Sleep(int32_t ms)
{
    boost::this_thread::sleep_for(boost::chrono::milliseconds(ms));
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

void NsbInterpreter::SetOpacity(Drawable* pDrawable, int32_t Time, int32_t Opacity, const string& Tempo, bool Wait)
{
    if (!pDrawable)
        return;

    if (/*Text* pText = */dynamic_cast<Text*>(pDrawable))
    {
        if (Opacity == 0)
        {
            pGame->GLCallback(std::bind(&Game::ClearText, pGame));
            CacheHolder<Drawable>::Write(HandleName, nullptr); // hack: see Game::ClearText
        }
        return;
    }

    //if (Time == 0)
    {
        if (Opacity == 0)
            pGame->GLCallback(std::bind(&Game::RemoveDrawable, pGame, pDrawable));
        else if (Opacity == 1000)
            pGame->GLCallback(std::bind(&Game::AddDrawable, pGame, pDrawable));
    }
    //else
        //pDrawable->Fade(Opacity, Time);
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
    pMovie->setLoop(Loop); // NYI
    pMovie->openFromFile(File);
    string BoxHandle(HandleName, 0, HandleName.find_first_of("/"));
    if (sf::IntRect* pRect = CacheHolder<sf::IntRect>::Read(BoxHandle))
    {
        pMovie->setTextureRect(*pRect);
        pMovie->setPosition(pRect->left, pRect->top);
    }
    else
        pMovie->setPosition(x, y); // Maybe add xy and pRect->xy?

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
    char* pTexData = sResourceMgr->Read(File, &Size);
    if (!pTexData)
    {
        std::cout << "Failed to read texture " << File << std::endl;
        delete pTexture;
        CacheHolder<Drawable>::Write(HandleName, nullptr);
        return;
    }
    NsbAssert(pTexture->loadFromMemory(pTexData, Size), "Failed to load texture %!", File);

    sf::Sprite* pSprite = new sf::Sprite(*pTexture);
    // TODO: Positions are x/y specific!
    if (x < 0 && x >= -SPECIAL_POS_NUM)
        x = SpecialPosTable[-(x + 1)](pTexture->getSize().x);
    if (y < 0 && y >= -SPECIAL_POS_NUM)
        y = SpecialPosTable[-(y + 1)](pTexture->getSize().y);
    pSprite->setPosition(x, y);
    Drawable* pDrawable = new Drawable(pSprite, Priority, DRAWABLE_TEXTURE);
    CacheHolder<Drawable>::Write(HandleName, pDrawable);
    pGame->AddDrawable(pDrawable);
}

void NsbInterpreter::LoadScript(const string& FileName)
{
    LoadedScripts.push_back(sResourceMgr->GetResource<NsbFile>(FileName));
}

void NsbInterpreter::CallScript(const string& FileName)
{
    pScript = sResourceMgr->GetResource<NsbFile>(FileName);
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
        if (pLine->Magic == MAGIC_CLEAR_PARAMS)
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
