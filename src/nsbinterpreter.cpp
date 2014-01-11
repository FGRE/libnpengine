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
#include "playable.hpp"

#include <iostream>
#include <boost/chrono.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/thread/thread.hpp>

static const std::string SpecialPos[SPECIAL_POS_NUM] =
{
    "Center", "InBottom", "Middle",
    "OnLeft", "OutTop", "InTop",
    "OutRight"
};

NsbInterpreter::NsbInterpreter(Game* pGame) :
pGame(pGame),
StopInterpreter(false),
WaitTime(0),
BranchCondition(true)
{
#ifdef _WIN32
    Text::Initialize("fonts-japanese-gothic.ttf");
#else
    Text::Initialize("/usr/share/fonts/cjkuni-uming/uming.ttc");
#endif

    Builtins.resize(0xFF, nullptr);
    Builtins[MAGIC_ZOOM] = &NsbInterpreter::Zoom;
    Builtins[MAGIC_PLACEHOLDER_PARAM] = &NsbInterpreter::PlaceholderParam;
    Builtins[MAGIC_NEGATIVE] = &NsbInterpreter::Negative;
    Builtins[MAGIC_CREATE_ARRAY] = &NsbInterpreter::CreateArray;
    Builtins[MAGIC_SET] = &NsbInterpreter::Set;
    Builtins[MAGIC_ARRAY_READ] = &NsbInterpreter::ArrayRead;
    Builtins[MAGIC_REGISTER_CALLBACK] = &NsbInterpreter::RegisterCallback;
    Builtins[MAGIC_SET_STATE] = &NsbInterpreter::SetState;
    Builtins[MAGIC_PARSE_TEXT] = &NsbInterpreter::ParseText;
    Builtins[MAGIC_SET_AUDIO_LOOP] = &NsbInterpreter::SetAudioLoop;
    Builtins[MAGIC_SLEEP_MS] = &NsbInterpreter::SleepMs;
    Builtins[MAGIC_START_ANIMATION] = &NsbInterpreter::StartAnimation;
    Builtins[MAGIC_DISPLAY_TEXT] = &NsbInterpreter::DisplayText;
    Builtins[MAGIC_SET_AUDIO_STATE] = &NsbInterpreter::SetAudioState;
    Builtins[MAGIC_SET_AUDIO_RANGE] = &NsbInterpreter::SetAudioRange;
    Builtins[MAGIC_SET_FONT_ATTRIBUTES] = &NsbInterpreter::SetFontAttributes;
    Builtins[MAGIC_LOAD_AUDIO] = &NsbInterpreter::LoadAudio;
    Builtins[MAGIC_SET_TEXTBOX_ATTRIBUTES] = &NsbInterpreter::SetTextboxAttributes;
    Builtins[MAGIC_CREATE_BOX] = &NsbInterpreter::CreateBox;
    Builtins[MAGIC_APPLY_BLUR] = &NsbInterpreter::ApplyBlur;
    Builtins[MAGIC_GET_MOVIE_TIME] = &NsbInterpreter::GetMovieTime;
    Builtins[MAGIC_SET_PARAM] = &NsbInterpreter::SetParam;
    Builtins[MAGIC_GET] = &NsbInterpreter::Get;
    Builtins[MAGIC_DRAW_TO_TEXTURE] = &NsbInterpreter::DrawToTexture;
    Builtins[MAGIC_CREATE_TEXTURE] = &NsbInterpreter::CreateTexture;
    Builtins[MAGIC_LOAD_MOVIE] = &NsbInterpreter::LoadMovie;
    Builtins[MAGIC_APPLY_MASK] = &NsbInterpreter::ApplyMask;
    Builtins[MAGIC_CREATE_COLOR] = &NsbInterpreter::CreateColor;
    Builtins[MAGIC_LOAD_TEXTURE] = &NsbInterpreter::LoadTexture;
    Builtins[MAGIC_CALL] = &NsbInterpreter::Call;
    Builtins[MAGIC_ADD] = &NsbInterpreter::Add;
    Builtins[MAGIC_DESTROY] = &NsbInterpreter::Destroy;
    Builtins[MAGIC_SET_OPACITY] = &NsbInterpreter::SetOpacity;
    Builtins[MAGIC_BIND_IDENTIFIER] = &NsbInterpreter::BindIdentifier;
    Builtins[MAGIC_FUNCTION_BEGIN] = &NsbInterpreter::Begin;
    Builtins[MAGIC_CALL_SCRIPT] = &NsbInterpreter::CallScript;
    Builtins[MAGIC_IF] = &NsbInterpreter::If;
    Builtins[MAGIC_WHILE] = &NsbInterpreter::While;
    Builtins[MAGIC_LOGICAL_NOT] = &NsbInterpreter::LogicalNot;
    //Builtins[MAGIC_LOGICAL_EQUAL] = &NsbInterpreter::LogicalEqual;
    Builtins[MAGIC_LOGICAL_NOT_EQUAL] = &NsbInterpreter::LogicalNotEqual;
    Builtins[MAGIC_FUNCTION_END] = &NsbInterpreter::End;
    Builtins[MAGIC_FWN_UNK] = &NsbInterpreter::End; // Fuwanovel hack, unknown purpose
    Builtins[MAGIC_CLEAR_PARAMS] = &NsbInterpreter::ClearParams;
    Builtins[MAGIC_UNK3] = &NsbInterpreter::ClearParams; // Unknown if this hack is still needed
    Builtins[MAGIC_UNK5] = &NsbInterpreter::UNK5;
    Builtins[MAGIC_UNK65] = &NsbInterpreter::UNK65;
    //Builtins[MAGIC_FORMAT] = &NsbInterpreter::Format; // Depends on ArrayRead

    // TODO: include.nss/herpderp.nss from .map files instead
    LoadScript("nss/function_steinsgate.nsb");
    LoadScript("nss/function.nsb");
    LoadScript("nss/extra_achievements.nsb");
    LoadScript("nss/function_select.nsb");
    LoadScript("nss/function_stand.nsb");
}

NsbInterpreter::~NsbInterpreter()
{
}

void NsbInterpreter::ExecuteScript(const string& ScriptName)
{
    pScript = sResourceMgr->GetResource<NsbFile>(ScriptName);
    Run();
}

void NsbInterpreter::ExecuteScriptLocal(const string& ScriptName)
{
    pScript = new NsbFile(ScriptName);
    Run();
}

void NsbInterpreter::Run()
{
    do
    {
        while (!RunInterpreter)
            Sleep(10);

        if (WaitTime > 0)
        {
            Sleep(WaitTime);
            WaitTime = 0;
        }

        if (NsbAssert(pScript, "Interpreting null script"))
            break;
        pLine = pScript->GetNextLine();
        if (NsbAssert(pLine, "Interpreting null line"))
            break;

        try
        {
            if (pLine->Magic < Builtins.size())
                if (BuiltinFunc pFunc = Builtins[pLine->Magic])
                    (this->*pFunc)();
        }
        catch (...)
        {
            NsbAssert(false, "Exception caught");
        }
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

void NsbInterpreter::If()
{
    if (BranchCondition)
        return;

    string Label = GetParam<string>(0);
    Label.pop_back();
    Label.insert(Label.find_last_of('.') + 1, "end");
    do
    {
        // TODO: This can be done faster with symbol lookup table (.map)
        if (!JumpTo(MAGIC_LABEL))
            return;
    } while (pLine->Params[0] != Label);
}

void NsbInterpreter::While()
{
    If();
}

void NsbInterpreter::UNK65()
{
    //ReverseJumpTo(MAGIC_WHILE);
}

void NsbInterpreter::CallScript()
{
    CallScript(GetParam<string>(0));
}

void NsbInterpreter::LogicalNotEqual()
{
    if (NsbAssert(Params[0].Type == Params[1].Type, "Comparing variables of different types for non-equality"))
        return;

    if (Params[0].Type == "INT")
        BranchCondition = (GetVariable<int32_t>(Params[0].Value) !=
                           GetVariable<int32_t>(Params[1].Value));
    else
        BranchCondition = (GetVariable<string>(Params[0].Value) !=
                           GetVariable<string>(Params[1].Value));

}

void NsbInterpreter::LogicalEqual()
{
    if (NsbAssert(Params[0].Type == Params[1].Type, "Comparing variables of different types for equality"))
        return;

    if (Params[0].Type == "INT")
        BranchCondition = (GetVariable<int32_t>(Params[0].Value) ==
                           GetVariable<int32_t>(Params[1].Value));
    else
        BranchCondition = (GetVariable<string>(Params[0].Value) ==
                           GetVariable<string>(Params[1].Value));
}

void NsbInterpreter::LogicalNot()
{
    if (Params.back().Value == "true")
        BranchCondition = false;
    else if (Params.back().Value == "false")
        BranchCondition = true;
    else
        std::cout << "LogicalNot(): Applying to " << Params.back().Value << std::endl;
}

void NsbInterpreter::Zoom()
{
    if (Drawable* pDrawable = CacheHolder<Drawable>::Read(GetParam<string>(0)))
        NSBZoom(pDrawable, GetParam<int32_t>(1), GetParam<float>(2),
                GetParam<float>(3), GetParam<string>(4), GetParam<bool>(5));
}

void NsbInterpreter::UNK5()
{
    Params[0] = {"STRING", string()};
}

void NsbInterpreter::PlaceholderParam()
{
}

void NsbInterpreter::Negative()
{
    Params.back().Value = boost::lexical_cast<string>(-GetVariable<int32_t>(Params.back().Value));
    Params.back().Type = "WTF";
}

void NsbInterpreter::CreateArray()
{
    for (uint32_t i = 1; i < Params.size(); ++i)
        Arrays[pLine->Params[0]].Members.push_back(std::make_pair(string(), ArrayVariable(Params[i])));
}

void NsbInterpreter::Set()
{
    if (pLine->Params[0] == "__array_variable__")
        ;//*ArrayParams[ArrayParams.size() - 1] = Params[0];
    else
        SetVariable(pLine->Params[0], Params[0]);
}

void NsbInterpreter::ArrayRead()
{
    HandleName = pLine->Params[0];
    NSBArrayRead(GetParam<int32_t>(1));
}

void NsbInterpreter::RegisterCallback()
{
    pGame->RegisterCallback(static_cast<sf::Keyboard::Key>(pLine->Params[0][0] - 'A'), pLine->Params[1]);
}

void NsbInterpreter::SetState()
{
    HandleName = GetParam<string>(0);
    NSBSetState(GetParam<string>(1));
}

void NsbInterpreter::ParseText()
{
    HandleName = GetParam<string>(0);
    pGame->GLCallback(std::bind(&NsbInterpreter::GLParseText, this,
                      GetParam<string>(1), GetParam<string>(2)));
}

void NsbInterpreter::SetAudioLoop()
{
    if (Playable* pMusic = CacheHolder<Playable>::Read(HandleName))
        NSBSetAudioLoop(pMusic, GetParam<bool>(1));
}

void NsbInterpreter::SleepMs()
{
    Sleep(GetVariable<int32_t>(Params[0].Value));
}

void NsbInterpreter::StartAnimation()
{
    if (Drawable* pDrawable = CacheHolder<Drawable>::Read(GetParam<string>(0)))
        NSBStartAnimation(pDrawable, GetParam<int32_t>(1), GetParam<int32_t>(2),
                          GetParam<int32_t>(3), GetParam<string>(4), GetParam<bool>(5));
}

void NsbInterpreter::DisplayText()
{
    if (Text* pText = (Text*)CacheHolder<Drawable>::Read(GetParam<string>(0)))
        NSBDisplayText(pText, GetParam<string>(1));
}

void NsbInterpreter::SetAudioState()
{
    HandleName = GetParam<string>(0);
    if (HandleName.back() == '*' && HandleName.size() > 2)
    {
        WildcardCall<Playable>(HandleName, [this] (Playable* pMusic)
        {
            NSBSetAudioState(pMusic, GetParam<int32_t>(1), GetParam<int32_t>(2), GetParam<string>(3));
        });
    }
    else if (Playable* pMusic = CacheHolder<Playable>::Read(HandleName))
        NSBSetAudioState(pMusic, GetParam<int32_t>(1), GetParam<int32_t>(2), GetParam<string>(3));
}

void NsbInterpreter::SetAudioRange()
{
    if (Playable* pMusic = CacheHolder<Playable>::Read(GetParam<string>(0)))
        NSBSetAudioRange(pMusic, GetParam<int32_t>(1), GetParam<int32_t>(2));
}

void NsbInterpreter::SetFontAttributes()
{
    NSBSetFontAttributes(GetParam<string>(0), GetParam<int32_t>(1), GetParam<string>(2),
                         GetParam<string>(3), GetParam<int32_t>(4), GetParam<string>(5));
}

void NsbInterpreter::LoadAudio()
{
    HandleName = GetParam<string>(0);
    NSBLoadAudio(GetParam<string>(1), GetParam<string>(2) + ".ogg");
}

void NsbInterpreter::SetTextboxAttributes()
{
    HandleName = GetParam<string>(0);
    NSBSetTextboxAttributes(GetParam<int32_t>(1), GetParam<string>(2), GetParam<int32_t>(3),
                            GetParam<string>(4), GetParam<string>(5), GetParam<int32_t>(6), GetParam<string>(7));
}

void NsbInterpreter::CreateBox()
{
    HandleName = GetParam<string>(0);
    NSBCreateBox(GetParam<int32_t>(1), GetParam<int32_t>(2), GetParam<int32_t>(3),
                 GetParam<int32_t>(4), GetParam<int32_t>(5), GetParam<bool>(6));
}

void NsbInterpreter::ApplyBlur()
{
    if (Drawable* pDrawable = CacheHolder<Drawable>::Read(GetParam<string>(0)))
        pGame->GLCallback(std::bind(&NsbInterpreter::GLApplyBlur, this, pDrawable, GetParam<string>(1)));
    else
    {
        std::cout << "Applying blur to NULL drawable!" << std::endl;
        WriteTrace(std::cout);
    }
}

void NsbInterpreter::GetMovieTime()
{
    HandleName = GetParam<string>(0);
    NSBGetMovieTime();
}

void NsbInterpreter::SetParam()
{
    Params.push_back({pLine->Params[0], pLine->Params[1]});
}

void NsbInterpreter::Get()
{
    Params.push_back(Variables[pLine->Params[0]]);
}

void NsbInterpreter::DrawToTexture()
{
    if (sf::RenderTexture* pTexture = CacheHolder<sf::RenderTexture>::Read(HandleName))
        pGame->GLCallback(std::bind(&NsbInterpreter::GLDrawToTexture, this, pTexture,
                         GetParam<int32_t>(1), GetParam<int32_t>(2), GetParam<string>(3)));
}

void NsbInterpreter::CreateTexture()
{
    HandleName = GetParam<string>(0);
    pGame->GLCallback(std::bind(&NsbInterpreter::GLCreateTexture, this,
                      GetParam<int32_t>(1), GetParam<int32_t>(2), GetParam<string>(3)));
}

void NsbInterpreter::ClearParams()
{
    Params.clear();
    ArrayParams.clear();
    BranchCondition = true; // Not sure about this...
}

void NsbInterpreter::Begin()
{
    // Turn params into variables
    for (uint32_t i = 1; i < pLine->Params.size(); ++i)
        SetVariable(pLine->Params[i], Params[i - 1]);
}

void NsbInterpreter::ApplyMask()
{
    if (Drawable* pDrawable = CacheHolder<Drawable>::Read(GetParam<string>(0)))
    {
        pGame->GLCallback(std::bind(&NsbInterpreter::GLApplyMask, this, pDrawable,
                          GetParam<int32_t>(1), GetParam<int32_t>(2), GetParam<int32_t>(3),
                          GetParam<int32_t>(4), GetParam<string>(5), GetParam<string>(6),
                          GetParam<bool>(7)));
    }
    else
    {
        std::cout << "Applying mask to NULL drawable!" << std::endl;
        WriteTrace(std::cout);
    }
}

void NsbInterpreter::LoadMovie()
{
    HandleName = GetParam<string>(0);
    pGame->GLCallback(std::bind(&NsbInterpreter::GLLoadMovie, this, GetParam<int32_t>(1),
                      GetParam<int32_t>(2), GetParam<int32_t>(3), GetParam<bool>(4),
                      GetParam<bool>(5), GetParam<string>(6), GetParam<bool>(7)));
}

void NsbInterpreter::CreateColor()
{
    HandleName = GetParam<string>(0);
    pGame->GLCallback(std::bind(&NsbInterpreter::GLCreateColor, this,
                      GetParam<int32_t>(1), GetParam<int32_t>(2), GetParam<int32_t>(3),
                      GetParam<int32_t>(4), GetParam<int32_t>(5), GetParam<string>(6)));
}

void NsbInterpreter::SetOpacity()
{
    HandleName = GetParam<string>(0);
    if (HandleName.back() == '*')
    {
        WildcardCall<Drawable>(HandleName, [this] (Drawable* pDrawable)
        {
            NSBSetOpacity(pDrawable, GetParam<int32_t>(1), GetParam<int32_t>(2),
                          GetParam<string>(3), GetParam<bool>(4));
        });
    }
    else if (Drawable* pDrawable = CacheHolder<Drawable>::Read(HandleName))
        NSBSetOpacity(pDrawable, GetParam<int32_t>(1), GetParam<int32_t>(2),
                      GetParam<string>(3), GetParam<bool>(4));
}

void NsbInterpreter::End()
{
    if (NsbAssert(!Returns.empty(), "Empty return stack"))
        pScript = nullptr;
    else
    {
        pScript = Returns.top().pScript;
        pScript->SetSourceIter(Returns.top().SourceLine);
        Returns.pop();
    }
}

void NsbInterpreter::LoadTexture()
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

    HandleName = GetParam<string>(0);
    pGame->GLCallback(std::bind(&NsbInterpreter::GLLoadTexture, this,
                      GetParam<int32_t>(1), Pos[0], Pos[1], GetParam<string>(4)));
}

void NsbInterpreter::Destroy()
{
    HandleName = GetParam<string>(0);
    // Hack: Do not destroy * (aka everything)
    if (HandleName.back() == '*' && HandleName.size() != 1)
    {
        WildcardCall<Drawable>(HandleName, [this](Drawable* pDrawable)
        {
            pGame->GLCallback(std::bind(&NsbInterpreter::GLDestroy, this, pDrawable));
            CacheHolder<Drawable>::Write(HandleName, nullptr);
        });
        WildcardCall<Playable>(HandleName, [this](Playable* pMovie)
        {
            delete pMovie;
            pGame->AddDrawable((Movie*)nullptr);
            CacheHolder<Playable>::Write(HandleName, nullptr);
        });
    }
    else if (Drawable* pDrawable = CacheHolder<Drawable>::Read(HandleName))
    {
        pGame->GLCallback(std::bind(&NsbInterpreter::GLDestroy, this, pDrawable));
        CacheHolder<Drawable>::Write(HandleName, nullptr);
    }
    else if (Playable* pMovie = CacheHolder<Playable>::Read(HandleName))
    {
        delete pMovie;
        pGame->AddDrawable((Movie*)nullptr);
        CacheHolder<Playable>::Write(HandleName, nullptr);
    }
}

void NsbInterpreter::Call()
{
    const char* FuncName = pLine->Params[0].c_str();

    // Find function override
    if (std::strcmp(FuncName, "MovieWaitSG") == 0)
    {
        HandleName = "ムービー";
        NSBGetMovieTime();
        Sleep(GetVariable<int32_t>(Params[0].Value));
        pGame->GLCallback(std::bind(&Game::RemoveDrawable, pGame,
                          CacheHolder<Drawable>::Read("ムービー")));
        return;
    }

    // Find function locally
    if (CallFunction(pScript, FuncName))
        return;

    // Find function globally
    for (uint32_t i = 0; i < LoadedScripts.size(); ++i)
        if (CallFunction(LoadedScripts[i], FuncName))
            return;

    std::cout << "Failed to lookup function symbol " << FuncName << std::endl;
}

void NsbInterpreter::Format()
{
    boost::format Fmt(Params[0].Value);
    for (uint8_t i = 1; i < Params.size(); ++i)
        Fmt % Params[i].Value;
    Params[0].Value = Fmt.str();
}

void NsbInterpreter::Add()
{
    uint32_t First = Params.size() - 2, Second = Params.size() - 1;
    NsbAssert(Params[First].Type == Params[Second].Type,
              "Concating params of different types (% and %)",
              Params[First].Type, Params[Second].Type);
    if (Params[First].Type == "INT" && Params[Second].Type == "INT")
        Params[First].Value = boost::lexical_cast<string>(
                              boost::lexical_cast<int32_t>(Params[First].Value) +
                              boost::lexical_cast<int32_t>(Params[Second].Value));
    else
        Params[First].Value += Params[Second].Value;
    Params.resize(Second);
}

template <class T> void NsbInterpreter::WildcardCall(std::string Handle, std::function<void(T*)> Func)
{
    for (auto i = CacheHolder<T>::ReadFirstMatch(Handle);
         i != CacheHolder<T>::Cache.end();
         i = CacheHolder<T>::ReadNextMatch(Handle, i))
    {
        HandleName = i->first;
        if (i->second)
            Func(i->second);
    }
}

template <class T> T NsbInterpreter::GetVariable(const string& Identifier)
{
    if (Identifier == "@")
        return T();

    if (Identifier[0] == '@')
        return boost::lexical_cast<T>(string(Identifier.c_str() + 1));

    auto iter = Variables.find(Identifier);
    try
    {
        // Not a variable but a literal
        if (iter == Variables.end())
            return boost::lexical_cast<T>(Identifier);
        // Special variable. I don't know why this works...
        else if (iter->second.Value[0] == '@')
            return boost::lexical_cast<T>(string(iter->second.Value.c_str() + 1));
        // Regular variable, TODO: Only dereference if $?
        else
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
    if (Params.size() > Index && Params[Index].Type == "WTF")
        return GetVariable<T>(Params[Index].Value);
    return GetVariable<T>(pLine->Params[Index]);
}

template <> bool NsbInterpreter::GetParam(int32_t Index)
{
    std::string String = GetParam<string>(Index);
    if (String == "true")
        return true;
    else if (String == "false")
        return false;
    NsbAssert(false, "Invalid boolification of string: ", String.c_str());
    return false; // Silence gcc
}

void NsbInterpreter::BindIdentifier()
{
    HandleName = pLine->Params[0];
    ArrayVariable* Var = &Arrays[HandleName];
    for (uint32_t i = 1; i < Params.size(); ++i)
        Var->Members[i - 1].first = Params[i].Value;
}

void NsbInterpreter::Sleep(int32_t ms)
{
    boost::this_thread::sleep_for(boost::chrono::milliseconds(ms));
}

void NsbInterpreter::SetVariable(const string& Identifier, const Variable& Var)
{
    Variables[Identifier] = Var;
}

void NsbInterpreter::LoadScript(const string& FileName)
{
    LoadedScripts.push_back(sResourceMgr->GetResource<NsbFile>(FileName));
}

void NsbInterpreter::CallScript(const string& FileName)
{
    if (NsbFile* pScript = sResourceMgr->GetResource<NsbFile>(FileName))
        this->pScript = pScript;
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

bool NsbInterpreter::JumpTo(uint16_t Magic)
{
#warning Remove return value. Its a hack for If() hack
    while (pLine = pScript->GetNextLine())
    {
        if (pLine->Magic == MAGIC_FUNCTION_END)
            return false;

        if (pLine->Magic == Magic)
            return true;
    }
    return false;
}

void NsbInterpreter::ReverseJumpTo(uint16_t Magic)
{
    while (pLine = pScript->GetPrevLine())
    {
        if (pLine->Magic == MAGIC_FUNCTION_BEGIN ||
            pLine->Magic == MAGIC_CHAPTER_BEGIN ||
            pLine->Magic == Magic)
            return;
    }
}

void NsbInterpreter::WriteTrace(std::ostream& Stream)
{
    if (!pScript)
        return;

    std::stack<FuncReturn> Stack = Returns;
    Stack.push({pScript, pScript->GetNextLineEntry()});
    while (!Stack.empty())
    {
        Stream << Stack.top().pScript->GetName() << " at " << Stack.top().SourceLine << std::endl;
        Stack.pop();
    }
}

void NsbInterpreter::DumpState()
{
    std::ofstream Log("state-log.txt");
    WriteTrace(Log);
}

void NsbInterpreter::Crash()
{
    std::cout << "\n**STACK TRACE BEGIN**\n";
    WriteTrace(std::cout);
    std::cout << "**STACK TRACE END**\nRecovering...\n" << std::endl;

#ifdef DEBUG
    abort();
#else
    Recover();
#endif
}

void NsbInterpreter::Recover()
{
    if (pScript)
        JumpTo(MAGIC_CLEAR_PARAMS);
}

// Rename/eliminate pls?
void NsbInterpreter::NsbAssert(const char* fmt)
{
    std::cout << fmt << std::endl;
}

template<typename T, typename... A>
bool NsbInterpreter::NsbAssert(bool expr, const char* fmt, T value, A... args)
{
    if (expr)
        return false;

    NsbAssert(fmt, value, args...);
    Crash();
    return true;
}

template<typename T, typename... A>
void NsbInterpreter::NsbAssert(const char* fmt, T value, A... args)
{
    while (*fmt)
    {
        if (*fmt == '%')
        {
            if (*(fmt + 1) == '%')
                ++fmt;
            else
            {
                std::cout << value;
                NsbAssert(fmt + 1, args...);
                return;
            }
        }
        std::cout << *fmt++;
    }
}

bool NsbInterpreter::NsbAssert(bool expr, const char* fmt)
{
    if (expr)
        return false;

    NsbAssert(fmt);
    Crash();
    return true;
}

// Because fuck you krofna, that's why
template <> bool NsbInterpreter::NsbAssert(bool expr, const char* fmt, std::string value)
{
#warning This is workaround for possible gcc bug
    if (expr)
        return false;
    std::cout << fmt << " " << value << std::endl; // I don't even care enough to format it anymore...
    Crash();
    return true;
}
