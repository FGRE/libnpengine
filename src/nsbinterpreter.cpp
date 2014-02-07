/* 
 * libnpengine: Nitroplus script interpreter
 * Copyright (C) 2013-2014 Mislav Blažević <krofnica996@gmail.com>
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
#include "steinsgate.hpp"

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

NsbContext::NsbContext() :
BranchCondition(true)
{
}

bool NsbContext::CallSubroutine(NsbFile* pDestScript, const char* Symbol, SymbolType Type)
{
    if (!pDestScript)
        return false;

    uint32_t CodeLine = pDestScript->GetSymbol(Symbol, Type);

    if (CodeLine != NSB_INVALIDE_LINE)
    {
        if (pScript)
            Returns.push({pScript, pScript->GetNextLineEntry()});
        pScript = pDestScript;
        pScript->SetSourceIter(CodeLine);
        return true;
    }
    return false;
}

void NsbContext::ReturnSubroutine()
{
    if (!Returns.empty())
    {
        pScript = Returns.top().pScript;
        pScript->SetSourceIter(Returns.top().SourceLine);
        Returns.pop();
    }
    else
        pScript = nullptr;
}

void NsbContext::Sleep(int32_t ms)
{
    SleepTime = sf::milliseconds(ms);
    SleepClock.restart();
}

bool NsbContext::NextLine()
{
    pLine = pScript->GetNextLine();
    return pLine != nullptr;
}

bool NsbContext::PrevLine()
{
    pLine = pScript->GetPrevLine();
    return pLine != nullptr;
}

NsbInterpreter::NsbInterpreter(Game* pGame) :
pPhone(nullptr),
pGame(pGame),
StopInterpreter(false)
{
#ifdef _WIN32
    Text::Initialize("fonts-japanese-gothic.ttf");
#else
    Text::Initialize("/usr/share/fonts/cjkuni-uming/uming.ttc");
#endif

    Builtins.resize(MAGIC_UNK119 + 1, nullptr);
    Builtins[MAGIC_TEXTURE_WIDTH] = &NsbInterpreter::TextureWidth;
    Builtins[MAGIC_TEXTURE_HEIGHT] = &NsbInterpreter::TextureHeight;
    Builtins[MAGIC_SHAKE] = &NsbInterpreter::Shake;
    Builtins[MAGIC_TIME] = &NsbInterpreter::Time;
    Builtins[MAGIC_CALL_SCENE] = &NsbInterpreter::CallScene;
    Builtins[MAGIC_CREATE_SCROLLBAR] = &NsbInterpreter::CreateScrollbar;
    Builtins[MAGIC_SYSTEM] = &NsbInterpreter::System;
    Builtins[MAGIC_CREATE_PROCESS] = &NsbInterpreter::CreateProcess;
    Builtins[MAGIC_LOAD_TEXTURE_CLIP] = &NsbInterpreter::LoadTextureClip;
    Builtins[MAGIC_INCREMENT] = &NsbInterpreter::Increment;
    Builtins[MAGIC_LOGICAL_GREATER] = &NsbInterpreter::LogicalGreater;
    Builtins[MAGIC_LOGICAL_LESS] = &NsbInterpreter::LogicalLess;
    Builtins[MAGIC_ARRAY_SIZE] = &NsbInterpreter::ArraySize;
    Builtins[MAGIC_CENTER] = &NsbInterpreter::Center;
    Builtins[MAGIC_ZOOM] = &NsbInterpreter::Zoom;
    Builtins[MAGIC_PLACEHOLDER_PARAM] = &NsbInterpreter::PlaceholderParam;
    Builtins[MAGIC_NEGATIVE] = &NsbInterpreter::Negative;
    Builtins[MAGIC_CREATE_ARRAY] = &NsbInterpreter::NSBCreateArray;
    Builtins[MAGIC_SET] = &NsbInterpreter::Set;
    Builtins[MAGIC_ARRAY_READ] = &NsbInterpreter::ArrayRead;
    Builtins[MAGIC_REGISTER_CALLBACK] = &NsbInterpreter::RegisterCallback;
    Builtins[MAGIC_REQUEST] = &NsbInterpreter::Request;
    Builtins[MAGIC_PARSE_TEXT] = &NsbInterpreter::ParseText;
    Builtins[MAGIC_SET_LOOP] = &NsbInterpreter::SetLoop;
    Builtins[MAGIC_WAIT] = &NsbInterpreter::Wait;
    Builtins[MAGIC_MOVE] = &NsbInterpreter::Move;
    Builtins[MAGIC_DISPLAY_TEXT] = &NsbInterpreter::DisplayText;
    Builtins[MAGIC_SET_VOLUME] = &NsbInterpreter::SetVolume;
    Builtins[MAGIC_SET_AUDIO_RANGE] = &NsbInterpreter::SetAudioRange;
    Builtins[MAGIC_SET_FONT_ATTRIBUTES] = &NsbInterpreter::SetFontAttributes;
    Builtins[MAGIC_LOAD_AUDIO] = &NsbInterpreter::LoadAudio;
    Builtins[MAGIC_SET_TEXTBOX_ATTRIBUTES] = &NsbInterpreter::SetTextboxAttributes;
    Builtins[MAGIC_CREATE_WINDOW] = &NsbInterpreter::CreateWindow;
    Builtins[MAGIC_APPLY_BLUR] = &NsbInterpreter::ApplyBlur;
    Builtins[MAGIC_GET_MOVIE_TIME] = &NsbInterpreter::GetMovieTime;
    Builtins[MAGIC_SET_PARAM] = &NsbInterpreter::SetParam;
    Builtins[MAGIC_GET] = &NsbInterpreter::Get;
    Builtins[MAGIC_DRAW_TO_TEXTURE] = &NsbInterpreter::DrawToTexture;
    Builtins[MAGIC_CREATE_RENDER_TEXTURE] = &NsbInterpreter::CreateRenderTexture;
    Builtins[MAGIC_CREATE_MOVIE] = &NsbInterpreter::CreateMovie;
    Builtins[MAGIC_DRAW_TRANSITION] = &NsbInterpreter::DrawTransition;
    Builtins[MAGIC_CREATE_COLOR] = &NsbInterpreter::CreateColor;
    Builtins[MAGIC_CREATE_TEXTURE] = &NsbInterpreter::CreateTexture;
    Builtins[MAGIC_CALL] = &NsbInterpreter::Call;
    Builtins[MAGIC_ADD] = &NsbInterpreter::Add;
    Builtins[MAGIC_DELETE] = &NsbInterpreter::Delete;
    Builtins[MAGIC_FADE] = &NsbInterpreter::Fade;
    Builtins[MAGIC_BIND_IDENTIFIER] = &NsbInterpreter::NSBBindIdentifier;
    Builtins[MAGIC_FUNCTION_BEGIN] = &NsbInterpreter::Begin;
    Builtins[MAGIC_CALL_CHAPTER] = &NsbInterpreter::CallChapter;
    Builtins[MAGIC_IF] = &NsbInterpreter::If;
    //Builtins[MAGIC_WHILE] = &NsbInterpreter::While;
    Builtins[MAGIC_LOGICAL_NOT] = &NsbInterpreter::LogicalNot;
    //Builtins[MAGIC_LOGICAL_EQUAL] = &NsbInterpreter::LogicalEqual;
    Builtins[MAGIC_LOGICAL_NOT_EQUAL] = &NsbInterpreter::LogicalNotEqual;
    Builtins[MAGIC_FUNCTION_END] = &NsbInterpreter::End;
    Builtins[MAGIC_FWN_UNK] = &NsbInterpreter::End; // Fuwanovel hack, unknown purpose
    Builtins[MAGIC_CLEAR_PARAMS] = &NsbInterpreter::ClearParams;
    Builtins[MAGIC_GET_SCRIPT_NAME] = &NsbInterpreter::GetScriptName;
    Builtins[MAGIC_SCOPE_BEGIN] = &NsbInterpreter::ScopeBegin;
    Builtins[MAGIC_SCOPE_END] = &NsbInterpreter::ScopeEnd;
    Builtins[MAGIC_FORMAT] = &NsbInterpreter::Format;
    Builtins[MAGIC_WRITE_FILE] = &NsbInterpreter::WriteFile;
    Builtins[MAGIC_DIVIDE] = &NsbInterpreter::Divide;
    //Builtins[MAGIC_LOOP_JUMP] = &NsbInterpreter::LoopJump;
    //Builtins[MAGIC_SET_ALIAS] = &NsbInterpreter::SetAlias;

    // Stubs
    Builtins[MAGIC_UNK1] = &NsbInterpreter::UNK1;
    Builtins[MAGIC_UNK2] = &NsbInterpreter::UNK2;
    Builtins[MAGIC_UNK77] = &NsbInterpreter::UNK77;

    // TODO: include.nss/herpderp.nss from .map files instead
    LoadScript("nss/macrosys2.nsb");
    LoadScript("nss/function_steinsgate.nsb");
    LoadScript("nss/function.nsb");
    LoadScript("nss/extra_achievements.nsb");
    LoadScript("nss/function_select.nsb");
    LoadScript("nss/function_stand.nsb");

    // Hack
    SetVariable("#SYSTEM_cosplay_patch", Variable{"STRING", "false"});

    // Steins gate
    pPhone = new Phone(new sf::Sprite());

    // Main script thread
    pMainContext = new NsbContext;
    pMainContext->Active = true;
}

NsbInterpreter::~NsbInterpreter()
{
    delete pPhone;
}

void NsbInterpreter::ExecuteScript(const string& ScriptName)
{
    pMainContext->pScript = sResourceMgr->GetResource<NsbFile>(ScriptName);
    Run();
}

void NsbInterpreter::ExecuteScriptLocal(const string& ScriptName)
{
    // This leaks memory but nobody really cares
    pMainContext->pScript = new NsbFile(ScriptName);
    Run();
}

void NsbInterpreter::Run()
{
    // Hack: boot script should call StArray()
    pContext = pMainContext;
    for (uint32_t i = 0; i < LoadedScripts.size(); ++i)
        if (pContext->CallSubroutine(LoadedScripts[i], "StArray", SYMBOL_FUNCTION))
            break;

    Threads.push_back(pMainContext);
    do
    {
        auto iter = Threads.begin();
        while (iter != Threads.end())
        {
            if (!RunInterpreter)
            {
                boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
                continue;
            }

            pContext = *iter++;

            if (!pContext->Active)
                continue;

            if (pContext->SleepClock.getElapsedTime() < pContext->SleepTime)
                continue;
            else
                pContext->SleepTime = sf::Time::Zero;

            do
            {
                if (!pContext->pScript || !pContext->NextLine())
                {
                    // Main thread died, terminate all others
                    if (pContext == pMainContext)
                    {
                        auto i = Threads.begin();
                        ++i; // Dont delete main thread
                        for (; i != Threads.end(); ++i)
                        {
                            CacheHolder<NsbContext>::Write((*i)->Identifier, nullptr);
                            delete *i;
                        }
                        Threads.clear();
                    }
                    else
                    {
                        Threads.remove(pContext);
                        CacheHolder<NsbContext>::Write(pContext->Identifier, nullptr);
                        delete pContext;
                        pContext = nullptr;
                    }
                    iter = Threads.end(); // Hack :) Do not invalidate iterator
                    break;
                }

                try
                {
                    if (pContext->pLine->Magic < Builtins.size())
                        if (BuiltinFunc pFunc = Builtins[pContext->pLine->Magic])
                            (this->*pFunc)();
                }
                catch (...)
                {
                    NsbAssert(false, "Exception caught");
                }
            } while (pContext && pContext->pLine->Magic != MAGIC_CLEAR_PARAMS);
        }
    } while (!StopInterpreter && !Threads.empty());
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

void NsbInterpreter::Time()
{
}

void NsbInterpreter::Shake()
{
}

void NsbInterpreter::CreateScrollbar()
{
}

void NsbInterpreter::TextureWidth()
{
    if (DrawableBase* pDrawable = CacheHolder<DrawableBase>::Read(GetParam<string>(0)))
        Params.push_back({"INT", boost::lexical_cast<string>(pDrawable->ToSprite()->getTexture()->getSize().x)});
}

void NsbInterpreter::TextureHeight()
{
    if (DrawableBase* pDrawable = CacheHolder<DrawableBase>::Read(GetParam<string>(0)))
        Params.push_back({"INT", boost::lexical_cast<string>(pDrawable->ToSprite()->getTexture()->getSize().y)});
}

void NsbInterpreter::LoadTextureClip()
{
    HandleName = GetParam<string>(0);
    pGame->GLCallback(std::bind(&NsbInterpreter::GLLoadTextureClip, this,
                      GetParam<int32_t>(1), GetParam<int32_t>(2), GetParam<int32_t>(3),
                      GetParam<int32_t>(4), GetParam<int32_t>(5), GetParam<int32_t>(6),
                      GetParam<int32_t>(7), GetParam<string>(8)));
}

// CreateProcess in Chaos;Head
void NsbInterpreter::CreateProcess()
{
    HandleName = GetParam<string>(0);
    NSBCreateProcess(GetParam<int32_t>(1), GetParam<int32_t>(2), GetParam<int32_t>(3), GetParam<string>(4));
}

void NsbInterpreter::WriteFile()
{
    NSBWriteFile(GetParam<string>(0), GetParam<string>(1));
}

void NsbInterpreter::Increment()
{
    uint32_t Index = Params.size() - 1;
    if (NsbAssert(Params[Index].Type == "INT", "Incrementing non-integer type"))
        return;

    Params[Index].Value = boost::lexical_cast<string>(boost::lexical_cast<int32_t>(Params[Index].Value) + 1);
}

void NsbInterpreter::LogicalGreater()
{
    if (NsbAssert(Params[0].Type == Params[1].Type && Params[0].Type == "INT",
        "Comparing variables of different or non-integer types"))
        return;

    pContext->BranchCondition = GetVariable<int32_t>(Params[0].Value) > GetVariable<int32_t>(Params[1].Value);
}

void NsbInterpreter::LogicalLess()
{
    if (NsbAssert(Params[0].Type == Params[1].Type && Params[0].Type == "INT",
        "Comparing variables of different or non-integer types"))
        return;

    pContext->BranchCondition = GetVariable<int32_t>(Params[0].Value) < GetVariable<int32_t>(Params[1].Value);
}

void NsbInterpreter::ArraySize()
{
    Params.push_back(Variable("INT", boost::lexical_cast<string>(Arrays[GetParam<string>(0)].Members.size())));
}

void NsbInterpreter::If()
{
    // TODO: Check if Params evaluate to true
    //      (Set BranchCondition before check)
    // Hack: Propertly implementing If breaks a lot of stuff
    //       because of other bugs, however this check is essential
    // See:  #SYSTEM_cosplay_patch in constructor
    if (Params.size() == 1 && Params[0].Value == "false")
        pContext->BranchCondition = false;

    // Jump to end of block only if condition is not met
    if (pContext->BranchCondition)
        return;

    string Label = GetParam<string>(0);
    Label.pop_back();
    Label.insert(Label.find_last_of('.') + 1, "end");
    do
    {
        // TODO: This can be done faster with symbol lookup table (.map)
        if (!JumpTo(MAGIC_LABEL))
            return;
    } while (pContext->pLine->Params[0] != Label);
}

void NsbInterpreter::While()
{
    if (pContext->BranchCondition)
        return;

    // Use the fact that labels are consistently named
    string Label = GetParam<string>(0);
    size_t i = Label.find("begin");
    Label.erase(i, 5);
    Label.insert(i, "end");

    do
    {
        if (!JumpTo(MAGIC_LABEL))
            return;
    } while (pContext->pLine->Params[0] != Label);
}

void NsbInterpreter::LoopJump()
{
    if (!pContext->NextLine())
        return;

    // Opposite of While()
    string Label = GetParam<string>(0);
    size_t i = Label.find("end");
    Label.erase(i, 3);
    Label.insert(i, "begin");

    do
    {
        ReverseJumpTo(MAGIC_WHILE);
    } while (pContext->pLine->Params[0] != Label);

    // Jump before logical condition
    ReverseJumpTo(MAGIC_CLEAR_PARAMS);
}

void NsbInterpreter::Center()
{
    if (Drawable* pDrawable = (Drawable*)CacheHolder<DrawableBase>::Read(GetParam<string>(0)))
        pDrawable->SetCenter(GetParam<int32_t>(1), GetParam<int32_t>(2));
}

void NsbInterpreter::CallScene()
{
    CallScriptSymbol(SYMBOL_SCENE);
}

void NsbInterpreter::CallChapter()
{
    CallScriptSymbol(SYMBOL_CHAPTER);
}

void NsbInterpreter::CallScriptSymbol(SymbolType Type)
{
    string ScriptName = GetParam<string>(0), Symbol;
    if (size_t i = ScriptName.find("->"))
    {
        Symbol = ScriptName.substr(i + 2);
        ScriptName.erase(i);
    }
    ScriptName.back() = 'b';
    CallScript(ScriptName, Symbol, Type);
}

void NsbInterpreter::LogicalNotEqual()
{
    if (NsbAssert(Params[0].Type == Params[1].Type, "Comparing variables of different types for non-equality"))
        return;

    pContext->BranchCondition = GetVariable<string>(Params[0].Value) != GetVariable<string>(Params[1].Value);
}

void NsbInterpreter::LogicalEqual()
{
    if (NsbAssert(Params[0].Type == Params[1].Type, "Comparing variables of different types for equality"))
        return;

    pContext->BranchCondition = GetVariable<string>(Params[0].Value) == GetVariable<string>(Params[1].Value);
}

void NsbInterpreter::LogicalNot()
{
    if (Params.back().Value == "true")
        pContext->BranchCondition = false;
    else if (Params.back().Value == "false")
        pContext->BranchCondition = true;
    else
        std::cout << "LogicalNot(): Applying to " << Params.back().Value << std::endl;
}

void NsbInterpreter::Zoom()
{
    if (Drawable* pDrawable = (Drawable*)CacheHolder<DrawableBase>::Read(GetParam<string>(0)))
        NSBZoom(pDrawable, GetParam<int32_t>(1), GetParam<float>(2),
                GetParam<float>(3), GetParam<string>(4), GetParam<bool>(5));
}

void NsbInterpreter::UNK1()
{
}

// LogicalOr, probably
void NsbInterpreter::UNK2()
{
}

// BlockBegin, called after Function/Chapter/Scene begin or If
void NsbInterpreter::ScopeBegin()
{
    ClearParams();
}

// BlockEnd, called before Function/Chapter/Scene end or Label
void NsbInterpreter::ScopeEnd()
{
}

void NsbInterpreter::GetScriptName()
{
    string Name = pContext->pScript->GetName();
    Name = Name.substr(4, Name.size() - 8); // Remove nss/ and .nsb
    Params.push_back(Variable("STRING", Name));
}

// CreateDialog, see: cg/sys/dialog/
void NsbInterpreter::UNK77()
{
}

// WinAPI ShellExecute: Only OPEN:https://www.somewhere.derp is actually used
void NsbInterpreter::System()
{
    NSBSystem(GetParam<string>(0), GetParam<string>(1), GetParam<string>(2));
}

void NsbInterpreter::PlaceholderParam()
{
}

void NsbInterpreter::Negative()
{
    Params.back().Value = boost::lexical_cast<string>(-GetVariable<int32_t>(Params.back().Value));
    // Negative integers are incorrectly compiled by Nitroplus
    // This works around the issue: See: NsbInterpreter::GetParam<T>
    Params.back().Type = "WTF";
}

void NsbInterpreter::Set()
{
    const string& Identifier = pContext->pLine->Params[0];
    if (pContext->pLine->Params.back() == "__array_variable__")
    {
        if (ArrayParams.empty())
            return;

        ArrayParams.back()->Type = Params.back().Type;
        ArrayParams.back()->Value = Params.back().Value;
    }
    else
    {
        if (Params.empty())
            return;

        // SetParam(STRING, value1)
        // SetParam(STRING, value2); <- Take last param
        // Set($var); <- Put it into first argument
        SetVariable(Identifier, Params.back());
    }

    // Handle hardcoded operations
    if (Identifier == "$SF_Phone_Open")
        pGame->GLCallback(std::bind(&NsbInterpreter::SGPhoneOpen, this));
    else if (Identifier == "$SW_PHONE_MODE")
        pGame->GLCallback(std::bind(&NsbInterpreter::SGPhoneMode, this));
    else if (Identifier == "$SF_PhoneMailReciveNew")
        pGame->GLCallback(std::bind(&Phone::MailReceive, pPhone, GetVariable<int32_t>("$SF_PhoneMailReciveNew")));
    else if (Identifier == "$SF_PhoneSD_Disp")
        pGame->GLCallback(std::bind(&Phone::SDDisplay, pPhone, GetVariable<int32_t>("$SF_PhoneSD_Disp")));
    else if (Identifier == "$LR_DATE")
        pGame->GLCallback(std::bind(&Phone::SetDate, pPhone, GetVariable<string>("$LR_DATE")));
    else if (Identifier == "$SW_PHONE_PRI")
        pGame->GLCallback(std::bind(&NsbInterpreter::SGPhonePriority, this));
}

void NsbInterpreter::ArrayRead()
{
    HandleName = pContext->pLine->Params[0];
    NSBArrayRead(GetParam<int32_t>(1));
}

void NsbInterpreter::RegisterCallback()
{
    pGame->RegisterCallback(static_cast<sf::Keyboard::Key>(pContext->pLine->Params[0][0] - 'A'), pContext->pLine->Params[1]);
}

void NsbInterpreter::Request()
{
    HandleName = GetParam<string>(0);
    NSBRequest(GetParam<string>(1));
}

void NsbInterpreter::ParseText()
{
    HandleName = GetParam<string>(0);
    pGame->GLCallback(std::bind(&NsbInterpreter::GLParseText, this,
                      GetParam<string>(1), GetParam<string>(2)));
}

void NsbInterpreter::SetLoop()
{
    HandleName = GetParam<string>(0);
    if (Playable* pMusic = CacheHolder<Playable>::Read(HandleName))
        NSBSetLoop(pMusic, GetParam<bool>(1));
}

void NsbInterpreter::Wait()
{
    Sleep(GetVariable<int32_t>(Params[0].Value));
}

void NsbInterpreter::Move()
{
    if (DrawableBase* pDrawable = CacheHolder<DrawableBase>::Read(GetParam<string>(0)))
        NSBMove(pDrawable, GetParam<int32_t>(1), GetParam<int32_t>(2),
                           GetParam<int32_t>(3), GetParam<string>(4), GetParam<bool>(5));
}

void NsbInterpreter::DisplayText()
{
    if (Text* pText = (Text*)CacheHolder<DrawableBase>::Read(GetParam<string>(0)))
        NSBDisplayText(pText, GetParam<string>(1));
}

void NsbInterpreter::SetVolume()
{
    HandleName = GetParam<string>(0);
    if (HandleName.back() == '*' && HandleName.size() > 2)
    {
        WildcardCall<Playable>(HandleName, [this] (Playable* pMusic)
        {
            NSBSetVolume(pMusic, GetParam<int32_t>(1), GetParam<int32_t>(2), GetParam<string>(3));
        });
    }
    else if (Playable* pMusic = CacheHolder<Playable>::Read(HandleName))
        NSBSetVolume(pMusic, GetParam<int32_t>(1), GetParam<int32_t>(2), GetParam<string>(3));
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

void NsbInterpreter::CreateWindow()
{
    HandleName = GetParam<string>(0);
    NSBCreateWindow(GetParam<int32_t>(1), GetParam<int32_t>(2), GetParam<int32_t>(3),
                    GetParam<int32_t>(4), GetParam<int32_t>(5), GetParam<bool>(6));
}

void NsbInterpreter::ApplyBlur()
{
    if (Drawable* pDrawable = (Drawable*)CacheHolder<DrawableBase>::Read(GetParam<string>(0)))
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
    Params.push_back({pContext->pLine->Params[0], pContext->pLine->Params[1]});
}

void NsbInterpreter::Get()
{
    Params.push_back(Variables[pContext->pLine->Params[0]]);
}

void NsbInterpreter::DrawToTexture()
{
    HandleName = GetParam<string>(0);
    if (sf::RenderTexture* pTexture = CacheHolder<sf::RenderTexture>::Read(HandleName))
        pGame->GLCallback(std::bind(&NsbInterpreter::GLDrawToTexture, this, pTexture,
                         GetParam<int32_t>(1), GetParam<int32_t>(2), GetParam<string>(3)));
}

void NsbInterpreter::CreateRenderTexture()
{
    HandleName = GetParam<string>(0);
    pGame->GLCallback(std::bind(&NsbInterpreter::GLCreateRenderTexture, this,
                      GetParam<int32_t>(1), GetParam<int32_t>(2), GetParam<string>(3)));
}

void NsbInterpreter::ClearParams()
{
    Params.clear();
    ArrayParams.clear();
    pContext->BranchCondition = true; // Not sure about this...
}

void NsbInterpreter::Begin()
{
    // Turn params into global variables
    // TODO: Should scope be respected instead?
    for (uint32_t i = 1; i < pContext->pLine->Params.size(); ++i)
        SetVariable(pContext->pLine->Params[i], Params[i - 1]);
}

void NsbInterpreter::DrawTransition()
{
    if (Drawable* pDrawable = (Drawable*)CacheHolder<DrawableBase>::Read(GetParam<string>(0)))
    {
        pGame->GLCallback(std::bind(&NsbInterpreter::GLDrawTransition, this, pDrawable,
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

void NsbInterpreter::CreateMovie()
{
    HandleName = GetParam<string>(0);
    pGame->GLCallback(std::bind(&NsbInterpreter::GLCreateMovie, this, GetParam<int32_t>(1),
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

void NsbInterpreter::Fade()
{
    HandleName = GetParam<string>(0);
    if (HandleName.back() == '*')
    {
        WildcardCall<DrawableBase>(HandleName, [this] (DrawableBase* pDrawable)
        {
            NSBFade(pDrawable, GetParam<int32_t>(1), GetParam<int32_t>(2),
                    GetParam<string>(3), GetParam<bool>(4));
        });
    }
    else if (DrawableBase* pDrawable = CacheHolder<DrawableBase>::Read(HandleName))
        NSBFade(pDrawable, GetParam<int32_t>(1), GetParam<int32_t>(2),
                GetParam<string>(3), GetParam<bool>(4));
}

void NsbInterpreter::End()
{
    pContext->ReturnSubroutine();
}

void NsbInterpreter::CreateTexture()
{
    // Represent special position as negative index to function
    // in SpecialPosTable. See: NsbInterpreter::GLLoadTexture
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
    pGame->GLCallback(std::bind(&NsbInterpreter::GLCreateTexture, this,
                      GetParam<int32_t>(1), Pos[0], Pos[1], GetParam<string>(4)));
}

void NsbInterpreter::Delete()
{
    HandleName = GetParam<string>(0);
    NSBDelete();
}

void NsbInterpreter::Call()
{
    const char* FuncName = pContext->pLine->Params[0].c_str();

    // Find function override (i.e. a hack)
    if (std::strcmp(FuncName, "MovieWaitSG") == 0 && pGame->pMovie)
    {
        ClearParams();
        HandleName = "ムービー";
        NSBGetMovieTime();
        if (!std::ifstream("NOMOVIE"))
            Wait();
        pGame->GLCallback(std::bind(&Game::RemoveDrawable, pGame,
                          CacheHolder<DrawableBase>::Read("ムービー")));
        return;
    }
    else if (std::strcmp(FuncName, "DeleteAllSt") == 0)
    {
        ClearParams();
        HandleName = "StNameSTBUF1/STBUF100";
        NSBDelete();
        return;
    }
    else if (std::strcmp(FuncName, "St") == 0 ||
             std::strcmp(FuncName, "PosSt") == 0)
        Params[0].Value = "STBUF1";

    // Find function locally
    if (pContext->CallSubroutine(pContext->pScript, FuncName, SYMBOL_FUNCTION))
        return;

    // Find function globally
    for (uint32_t i = 0; i < LoadedScripts.size(); ++i)
        if (pContext->CallSubroutine(LoadedScripts[i], FuncName, SYMBOL_FUNCTION))
            return;

    std::cout << "Failed to lookup function symbol " << FuncName << std::endl;
}

void NsbInterpreter::Format()
{
    boost::format Fmt(Params[0].Value);

    // Don't format more Params than specified by argument list (pLine->Params)
    for (uint8_t i = Params.size() - (pContext->pLine->Params.size() - 1); i < Params.size(); ++i)
        Fmt % Params[i].Value;

    // Remove arguments used by Format
    Params.resize(Params.size() - (pContext->pLine->Params.size() - 1));
    Params.back().Value = Fmt.str();
}

void NsbInterpreter::Add()
{
    uint32_t First = Params.size() - 2, Second = Params.size() - 1;
    if (NsbAssert(Params[First].Type == Params[Second].Type,
                  "Concating params of different types (% and %)",
                  Params[First].Type, Params[Second].Type))
        return;

    // If parameters are integers, perform addition instead of string concat
    if (Params[First].Type == "INT")
        Params[First].Value = boost::lexical_cast<string>(
                              boost::lexical_cast<int32_t>(Params[First].Value) +
                              boost::lexical_cast<int32_t>(Params[Second].Value));
    else
        Params[First].Value += Params[Second].Value;
    Params.resize(Second);
}

void NsbInterpreter::Divide()
{
    uint32_t First = Params.size() - 2, Second = Params.size() - 1;
    if (NsbAssert(Params[First].Type == Params[Second].Type && Params[First].Type == "INT",
                  "Dividing params of non-integer types (% and %)",
                  Params[First].Type, Params[Second].Type))
        return;

    // Do not divide by zero
    if (boost::lexical_cast<int32_t>(Params[Second].Value) != 0)
        Params[First].Value = boost::lexical_cast<string>(
                              boost::lexical_cast<int32_t>(Params[First].Value) /
                              boost::lexical_cast<int32_t>(Params[Second].Value));
    else
    {
        std::cout << "Division by zero!" << std::endl;
        WriteTrace(std::cout);
    }
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
    // "WTF" is workaround for Nitroplus bug
    // See: NsbInterpreter::Negative
    // TODO: Should probably take all from parameter list but currently it may regress
    if (Params.size() > Index && Params[Index].Type == "WTF")
        return GetVariable<T>(Params[Index].Value);
    return GetVariable<T>(pContext->pLine->Params[Index]);
}

template <> bool NsbInterpreter::GetParam(int32_t Index)
{
    std::string String = GetParam<string>(Index);
    if (String == "true")
        return true;
    else if (String == "false")
        return false;
    NsbAssert(false, "Invalid boolification of string: ", String);
    return false; // Silence gcc
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
    if (NsbFile* pScript = sResourceMgr->GetResource<NsbFile>(FileName))
        LoadedScripts.push_back(pScript);
}

void NsbInterpreter::CallScript(const string& FileName, const string& Symbol, SymbolType Type)
{
    pContext->CallSubroutine(sResourceMgr->GetResource<NsbFile>(FileName), Symbol.c_str(), Type);
}

bool NsbInterpreter::JumpTo(uint16_t Magic)
{
#warning Remove return value. Its a hack for If() hack
    if (!pContext->pLine)
        return false;

    do
    {
        // Just in case, jumping beyond scope end can be very bad
        // TODO: Perhaps it should be logged instead
        if (pContext->pLine->Magic == MAGIC_SCOPE_END)
            return false;

        if (pContext->pLine->Magic == Magic)
        {
            // TODO: Do not skip this line in Run()
            // pContext->PrevLine();
            return true;
        }

    } while (pContext->NextLine());
    return false;
}

void NsbInterpreter::ReverseJumpTo(uint16_t Magic)
{
    do
    {
        // Don't jump outside scope
        // TODO: Log this?
        if (pContext->pLine->Magic == MAGIC_SCOPE_BEGIN ||
            pContext->pLine->Magic == Magic)
        {
            // TODO: Do not skip this line in Run()
            // pContext->PrevLine();
            return;
        }
    } while (pContext->PrevLine());
}

void NsbInterpreter::WriteTrace(std::ostream& Stream)
{
    if (!pContext->pScript)
        return;

    std::stack<FuncReturn> Stack = pContext->Returns;
    Stack.push({pContext->pScript, pContext->pScript->GetNextLineEntry()});
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
    // It is generally segfault-safe to jump to next ClearParams()
    if (pContext->pScript)
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

template <> bool NsbInterpreter::NsbAssert(bool expr, const char* fmt, string value)
{
    return NsbAssert(expr, fmt, value.c_str());
}
