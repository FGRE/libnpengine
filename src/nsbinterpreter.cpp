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
#include "game.hpp"
#include "drawable.hpp"
#include "resourcemgr.hpp"
#include "nsbmagic.hpp"
#include "text.hpp"
#include "playable.hpp"

#include <fstream>
#include <iostream>
#include <boost/chrono.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/thread/thread.hpp>
#include <SFML/Graphics/Sprite.hpp>

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

bool NsbContext::CallSubroutine(ScriptFile* pDestScript, string Symbol)
{
    if (!pDestScript)
        return false;

    uint32_t CodeLine = pDestScript->GetSymbol(Symbol);
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

NsbInterpreter::NsbInterpreter() :
StopInterpreter(false),
pGame(nullptr)
{
    Builtins.resize(MAGIC_UNK119 + 1, nullptr);
    //Builtins[MAGIC_JUMP] = &NsbInterpreter::Jump;
    Builtins[MAGIC_LOGICAL_AND] = &NsbInterpreter::LogicalAnd;
    Builtins[MAGIC_LOGICAL_OR] = &NsbInterpreter::LogicalOr;
    Builtins[MAGIC_LOGICAL_GREATER_EQUAL] = &NsbInterpreter::LogicalGreaterEqual;
    Builtins[MAGIC_LOGICAL_LESS_EQUAL] = &NsbInterpreter::LogicalLessEqual;
    Builtins[MAGIC_SUBSTRACT] = &NsbInterpreter::Substract;
    Builtins[MAGIC_TEXTURE_WIDTH] = &NsbInterpreter::TextureWidth;
    Builtins[MAGIC_TEXTURE_HEIGHT] = &NsbInterpreter::TextureHeight;
    Builtins[MAGIC_SHAKE] = &NsbInterpreter::Shake;
    Builtins[MAGIC_TIME] = &NsbInterpreter::Time;
    Builtins[MAGIC_CALL_SCENE] = &NsbInterpreter::CallScene;
    Builtins[MAGIC_CREATE_SCROLLBAR] = &NsbInterpreter::CreateScrollbar;
    Builtins[MAGIC_SYSTEM] = &NsbInterpreter::System;
    //Builtins[MAGIC_CREATE_PROCESS] = &NsbInterpreter::CreateProcess;
    Builtins[MAGIC_LOAD_TEXTURE_CLIP] = &NsbInterpreter::LoadTextureClip;
    Builtins[MAGIC_INCREMENT] = &NsbInterpreter::Increment;
    Builtins[MAGIC_LOGICAL_GREATER] = &NsbInterpreter::LogicalGreater;
    Builtins[MAGIC_LOGICAL_LESS] = &NsbInterpreter::LogicalLess;
    Builtins[MAGIC_ARRAY_SIZE] = &NsbInterpreter::ArraySize;
    Builtins[MAGIC_CENTER] = &NsbInterpreter::Center;
    Builtins[MAGIC_ZOOM] = &NsbInterpreter::Zoom;
    //Builtins[MAGIC_PLACEHOLDER_PARAM] = &NsbInterpreter::PlaceholderParam;
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
    Builtins[MAGIC_WAIT_TEXT] = &NsbInterpreter::WaitText;
    Builtins[MAGIC_SET_VOLUME] = &NsbInterpreter::SetVolume;
    Builtins[MAGIC_SET_LOOP_POINT] = &NsbInterpreter::SetLoopPoint;
    Builtins[MAGIC_SET_FONT_ATTRIBUTES] = &NsbInterpreter::SetFontAttributes;
    Builtins[MAGIC_CREATE_SOUND] = &NsbInterpreter::CreateSound;
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
    Builtins[MAGIC_MULTIPLY] = &NsbInterpreter::Multiply;
    Builtins[MAGIC_RETURN] = &NsbInterpreter::Return;
    Builtins[MAGIC_LOOP_JUMP] = &NsbInterpreter::LoopJump;
    //Builtins[MAGIC_SET_ALIAS] = &NsbInterpreter::SetAlias;

    // Stubs
    Builtins[MAGIC_UNK77] = &NsbInterpreter::UNK77;

    // Hack
    SetVariable("#SYSTEM_cosplay_patch", Variable{"STRING", "false"});

    // Main script thread
    pMainContext = new NsbContext;
    pMainContext->Active = true;
}

NsbInterpreter::~NsbInterpreter()
{
}

extern "C" { void gst_init(int* argc, char** argv[]); }

void NsbInterpreter::Initialize(Game* pGame)
{
    this->pGame = pGame;
    gst_init(nullptr, nullptr);
    Text::Initialize("/usr/share/fonts/cjkuni-uming/uming.ttc");
    sResourceMgr = new ResourceMgr({"cg.npa", "nss.npa", "voice.npa", "sound.npa"});

    // TODO: include.nss/herpderp.nss from .map files instead
    LoadScript("nss/macrosys2.nsb");
    LoadScript("nss/function_steinsgate.nsb");
    LoadScript("nss/function.nsb");
    LoadScript("nss/extra_achievements.nsb");
    LoadScript("nss/function_select.nsb");
    LoadScript("nss/function_stand.nsb");
}

void NsbInterpreter::ExecuteScript(const string& ScriptName)
{
    pMainContext->pScript = sResourceMgr->GetScriptFile(ScriptName);
    Run();
}

void NsbInterpreter::ExecuteScriptLocal(const string& ScriptName)
{
    // This leaks memory but nobody really cares
    pMainContext->pScript = new ScriptFile(ScriptName);
    Run();
}

void NsbInterpreter::Run()
{
    // Hack: boot script should call StArray()
    pContext = pMainContext;
    for (uint32_t i = 0; i < LoadedScripts.size(); ++i)
        if (pContext->CallSubroutine(LoadedScripts[i], "function.StArray"))
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

    // HACK: Since this->Params contains a copy of real variable, incrementing it has no effect
    //       These copies do not contain identifiers or references to original variable, we must go
    //       back one line to find identifier.
    // TODO: Sometimes, ArrayRead can be found before Increment (instead of Get)
    //       In that case, this code is incorrect
    pContext->PrevLine();
    SetVariable(pContext->pLine->Params[0], { "INT", boost::lexical_cast<string>(boost::lexical_cast<int32_t>(Params[Index].Value) + 1) });
    pContext->NextLine();
}

void NsbInterpreter::LogicalAnd()
{
    // TODO
}

void NsbInterpreter::LogicalOr()
{
    // TODO
}

void NsbInterpreter::LogicalGreaterEqual()
{
    LogicalOperator([](int32_t a, int32_t b) { return a >= b; });
}

void NsbInterpreter::LogicalGreater()
{
    LogicalOperator([](int32_t a, int32_t b) { return a > b; });
}

void NsbInterpreter::LogicalLess()
{
    LogicalOperator([](int32_t a, int32_t b) { return a < b; });
}

void NsbInterpreter::LogicalLessEqual()
{
    LogicalOperator([](int32_t a, int32_t b) { return a <= b; });
}

void NsbInterpreter::LogicalOperator(std::function<bool(int32_t, int32_t)> Func)
{
    uint32_t First = Params.size() - 2, Second = Params.size() - 1;
    if (NsbAssert(Params[First].Type == Params[Second].Type && Params[First].Type == "INT",
                  "LogicalOperator: Params of different or non-integer types"))
        return;

    pContext->BranchCondition = Func(GetVariable<int32_t>(Params[First].Value),
                                         GetVariable<int32_t>(Params[Second].Value));
}

void NsbInterpreter::ArraySize()
{
    Params.back() = Variable("INT", boost::lexical_cast<string>(Arrays[pContext->pLine->Params[0]].Members.size()));
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
    if (!pContext->BranchCondition)
        Jump();
}

void NsbInterpreter::While()
{
    // If condition is not met, jump to end of block
    if (!pContext->BranchCondition)
        Jump();
}

void NsbInterpreter::Jump()
{
    pContext->pScript->SetSourceIter(pContext->pScript->GetSymbol(pContext->pLine->Params[0]));
}

void NsbInterpreter::LoopJump()
{
}

void NsbInterpreter::Center()
{
    if (Drawable* pDrawable = (Drawable*)CacheHolder<DrawableBase>::Read(GetParam<string>(0)))
        pDrawable->SetCenter(GetParam<int32_t>(1), GetParam<int32_t>(2));
}

void NsbInterpreter::CallScene()
{
    Params[0].Value = string("scene.") + Params[0].Value;
    CallScriptSymbol();
}

void NsbInterpreter::CallChapter()
{
    Params[0].Value = string("chapter.") + Params[0].Value;
    CallScriptSymbol();
}

void NsbInterpreter::CallScriptSymbol()
{
    string ScriptName = GetParam<string>(0), Symbol;
    if (size_t i = ScriptName.find("->"))
    {
        Symbol = ScriptName.substr(i + 2);
        ScriptName.erase(i);
    }
    ScriptName.back() = 'b'; // .nss -> .nsb
    CallScript(ScriptName, Symbol);
}

void NsbInterpreter::LogicalNotEqual()
{
    LogicalOperator([](int32_t a, int32_t b) { return a != b; });
}

void NsbInterpreter::LogicalEqual()
{
    LogicalOperator([](int32_t a, int32_t b) { return a == b; });
}

void NsbInterpreter::LogicalNot()
{
    if (NsbAssert(!Params.empty(), "No parameter passed to LogicalNot"))
        return;

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
        pGame->GLCallback(std::bind(&NsbInterpreter::GLZoom, this, pDrawable, GetParam<int32_t>(1),
                          GetParam<float>(2), GetParam<float>(3), GetParam<string>(4), GetParam<bool>(5)));
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
    // Parameters with this flag are @ in MAGIC_CALL argument list
    // This works around the issue: See: NsbInterpreter::GetParam<T>
    Params.back().Type = "WTF";
}

void NsbInterpreter::Negative()
{
    Params.back().Value = boost::lexical_cast<string>(-GetVariable<int32_t>(Params.back().Value));
    // Negative integers are incorrectly compiled by Nitroplus
    PlaceholderParam();
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
}

void NsbInterpreter::ArrayRead()
{
    HandleName = pContext->pLine->Params[0];
    NSBArrayRead(GetParam<int32_t>(1));
}

void NsbInterpreter::RegisterCallback()
{
    string Script = pContext->pLine->Params[1];
    Script.back() = 'b'; // .nss -> .nsb
    pGame->RegisterCallback(static_cast<sf::Keyboard::Key>(pContext->pLine->Params[0][0] - 'A'), Script);
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
        pGame->GLCallback(std::bind(&NsbInterpreter::GLMove, this, pDrawable, GetParam<int32_t>(1),
                          GetParam<int32_t>(2), GetParam<int32_t>(3), GetParam<string>(4), GetParam<bool>(5)));
}

void NsbInterpreter::WaitText()
{
    if (Text* pText = (Text*)CacheHolder<DrawableBase>::Read(GetParam<string>(0)))
        NSBWaitText(pText, GetParam<string>(1));
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

void NsbInterpreter::SetLoopPoint()
{
    if (Playable* pMusic = CacheHolder<Playable>::Read(GetParam<string>(0)))
        NSBSetLoopPoint(pMusic, GetParam<int32_t>(1), GetParam<int32_t>(2));
}

void NsbInterpreter::SetFontAttributes()
{
    NSBSetFontAttributes(GetParam<string>(0), GetParam<int32_t>(1), GetParam<string>(2),
                         GetParam<string>(3), GetParam<int32_t>(4), GetParam<string>(5));
}

void NsbInterpreter::CreateSound()
{
    HandleName = GetParam<string>(0);
    NSBCreateSound(GetParam<string>(1), GetParam<string>(2) + ".ogg");
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
    string FuncNameFull = string("function.") + FuncName;

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
    if (pContext->CallSubroutine(pContext->pScript, FuncNameFull))
        return;

    // Find function globally
    for (uint32_t i = 0; i < LoadedScripts.size(); ++i)
        if (pContext->CallSubroutine(LoadedScripts[i], FuncNameFull))
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

void NsbInterpreter::Return()
{
    End();
}

void NsbInterpreter::Substract()
{
    BinaryOperator([] (int32_t a, int32_t b) -> int32_t { return a - b; });
}

void NsbInterpreter::Add()
{
    // If parameters are strings, concat
    uint32_t First = Params.size() - 2, Second = Params.size() - 1;
    if (Params[First].Type == "STRING")
    {
        Params[First].Value += Params[Second].Value;
        Params.resize(Second);
    }
    else
        BinaryOperator([](int32_t a, int32_t b) -> int32_t { return a + b; });
}

void NsbInterpreter::Divide()
{
    BinaryOperator([](int32_t a, int32_t b) -> int32_t
    {
        if (b == 0)
            return 0;
        return a / b;
    });
}

void NsbInterpreter::Multiply()
{
    BinaryOperator([](int32_t a, int32_t b) -> int32_t { return a + b; });
}

void NsbInterpreter::BinaryOperator(std::function<int32_t(int32_t, int32_t)> Func)
{
    uint32_t First = Params.size() - 2, Second = Params.size() - 1;
    if (NsbAssert(Params[First].Type == Params[Second].Type,
                  "BianryOperator: Params of different types"))
        return;

    Params[First].Value = boost::lexical_cast<string>(Func(boost::lexical_cast<int32_t>(Params[First].Value),
                                                           boost::lexical_cast<int32_t>(Params[Second].Value)));
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
    NsbAssert(false, "Boolification of string failed");
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
    if (ScriptFile* pScript = sResourceMgr->GetScriptFile(FileName))
        LoadedScripts.push_back(pScript);
}

void NsbInterpreter::CallScript(const string& FileName, const string& Symbol)
{
    pContext->CallSubroutine(sResourceMgr->GetScriptFile(FileName), Symbol.c_str());
}

// TODO: Obsolete?
bool NsbInterpreter::JumpTo(uint16_t Magic)
{
#warning Remove return value. Its a hack for If() hack
    if (!pContext->pLine)
        return false;

    while (pContext->NextLine())
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
    }
    return false;
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

bool NsbInterpreter::NsbAssert(bool expr, string error)
{
    if (expr)
        return false;

    std::cout << error << std::endl;
    std::cout << "\n**STACK TRACE BEGIN**\n";
    WriteTrace(std::cout);
    std::cout << "**STACK TRACE END**\n" << std::endl;

    // It is generally segfault-safe to jump to next ClearParams()
    if (pContext->pScript)
        JumpTo(MAGIC_CLEAR_PARAMS);
    return true;
}
