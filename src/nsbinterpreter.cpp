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
#include "nsbmagic.hpp"
#include "text.hpp"
#include "nsbcontext.hpp"

#include <fstream>
#include <iostream>
#include <thread>
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>

NsbInterpreter::NsbInterpreter() :
StopInterpreter(false),
pGame(nullptr),
pDebuggerThread(nullptr),
DbgStepping(false)
{
    Builtins.resize(MAGIC_UNK119 + 1, nullptr);
    Builtins[MAGIC_JUMP] = &NsbInterpreter::Jump;
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
    Builtins[MAGIC_LOAD_TEXTURE_CLIP] = &NsbInterpreter::LoadTextureClip;
    Builtins[MAGIC_INCREMENT] = &NsbInterpreter::Increment;
    Builtins[MAGIC_LOGICAL_GREATER] = &NsbInterpreter::LogicalGreater;
    Builtins[MAGIC_LOGICAL_LESS] = &NsbInterpreter::LogicalLess;
    Builtins[MAGIC_ARRAY_SIZE] = &NsbInterpreter::ArraySize;
    Builtins[MAGIC_SET_VERTEX] = &NsbInterpreter::SetVertex;
    Builtins[MAGIC_ZOOM] = &NsbInterpreter::Zoom;
    Builtins[MAGIC_NEGATIVE] = &NsbInterpreter::Negative;
    Builtins[MAGIC_CREATE_ARRAY] = &NsbInterpreter::CreateArray;
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
    Builtins[MAGIC_CREATE_SOUND] = &NsbInterpreter::CreateSound;
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
    Builtins[MAGIC_CALL_FUNCTION] = &NsbInterpreter::CallFunction;
    Builtins[MAGIC_ADD] = &NsbInterpreter::Add;
    Builtins[MAGIC_DELETE] = &NsbInterpreter::Delete;
    Builtins[MAGIC_FADE] = &NsbInterpreter::Fade;
    Builtins[MAGIC_BIND_IDENTIFIER] = &NsbInterpreter::BindIdentifier;
    Builtins[MAGIC_FUNCTION_BEGIN] = &NsbInterpreter::Begin;
    Builtins[MAGIC_CALL_CHAPTER] = &NsbInterpreter::CallChapter;
    Builtins[MAGIC_IF] = &NsbInterpreter::If;
    Builtins[MAGIC_WHILE] = &NsbInterpreter::If;
    Builtins[MAGIC_LOGICAL_NOT] = &NsbInterpreter::LogicalNot;
    Builtins[MAGIC_LOGICAL_EQUAL] = &NsbInterpreter::LogicalEqual;
    Builtins[MAGIC_LOGICAL_NOT_EQUAL] = &NsbInterpreter::LogicalNotEqual;
    Builtins[MAGIC_FUNCTION_END] = &NsbInterpreter::End;
    Builtins[MAGIC_SCENE_END] = &NsbInterpreter::End;
    Builtins[MAGIC_CHAPTER_END] = &NsbInterpreter::End;
    Builtins[MAGIC_FWN_UNK] = &NsbInterpreter::End; // Fuwanovel hack, unknown purpose
    Builtins[MAGIC_CLEAR_PARAMS] = &NsbInterpreter::ClearParams;
    Builtins[MAGIC_GET_SCRIPT_NAME] = &NsbInterpreter::GetScriptName;
    Builtins[MAGIC_SCOPE_BEGIN] = &NsbInterpreter::ScopeBegin;
    Builtins[MAGIC_SCOPE_END] = &NsbInterpreter::ScopeEnd;
    Builtins[MAGIC_FORMAT] = &NsbInterpreter::Format;
    Builtins[MAGIC_WRITE_FILE] = &NsbInterpreter::WriteFile;
    Builtins[MAGIC_DIVIDE] = &NsbInterpreter::Divide;
    Builtins[MAGIC_MULTIPLY] = &NsbInterpreter::Multiply;
    Builtins[MAGIC_RETURN] = &NsbInterpreter::End;
    Builtins[MAGIC_STRING_TO_VARIABLE] = &NsbInterpreter::StringToVariable;
    Builtins[MAGIC_LOAD_IMAGE] = &NsbInterpreter::LoadImage;
    Builtins[MAGIC_READ_FILE] = &NsbInterpreter::ReadFile;
    Builtins[MAGIC_SET_ALIAS] = &NsbInterpreter::SetAlias;
    Builtins[MAGIC_CREATE_CHOICE] = &NsbInterpreter::CreateChoice;
    Builtins[MAGIC_SET_NEXT_FOCUS] = &NsbInterpreter::SetNextFocus;
    Builtins[MAGIC_SET_FREQUENCY] = &NsbInterpreter::SetFrequency;
    Builtins[MAGIC_SET_PAN] = &NsbInterpreter::SetPan;
    Builtins[MAGIC_SOUND_AMPLITUDE] = &NsbInterpreter::SoundAmplitude;
    Builtins[MAGIC_CREATE_PROCESS] = &NsbInterpreter::CreateProcess;
    //Builtins[MAGIC_SET_FONT_ATTRIBUTES] = &NsbInterpreter::SetFontAttributes;
    //Builtins[MAGIC_SET_TEXTBOX_ATTRIBUTES] = &NsbInterpreter::SetTextboxAttributes;
    //Builtins[MAGIC_PLACEHOLDER_PARAM] = &NsbInterpreter::PlaceholderParam;

    // Stubs
    Builtins[MAGIC_UNK20] = &NsbInterpreter::UNK20;
    Builtins[MAGIC_UNK61] = &NsbInterpreter::UNK61;
    Builtins[MAGIC_UNK66] = &NsbInterpreter::UNK66;
    Builtins[MAGIC_UNK77] = &NsbInterpreter::UNK77;
    Builtins[MAGIC_UNK101] = &NsbInterpreter::UNK101;
    Builtins[MAGIC_UNK103] = &NsbInterpreter::UNK103;
    Builtins[MAGIC_UNK104] = &NsbInterpreter::UNK104;
    Builtins[MAGIC_UNK161] = &NsbInterpreter::UNK161;
    Builtins[MAGIC_CLEAR_SCORE] = &NsbInterpreter::ClearScore;
    Builtins[MAGIC_CLEAR_BACKLOG] = &NsbInterpreter::ClearBacklog;

    // Main script thread
    pMainContext = new NsbContext("");
    pMainContext->Start();
}

NsbInterpreter::~NsbInterpreter()
{
    if (pDebuggerThread)
        pDebuggerThread->join();
    delete pDebuggerThread;
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
    LoadScript("nss/sys_title.nsb");

    for (char i = 'a'; i <= 'z'; ++i)
    {
        string Identifier = "$SYSTEM_keydown_" + string(1, i);
        SetVariable(Identifier, new Variable("false"));
    }
}

void NsbInterpreter::ExecuteScript(const string& ScriptName)
{
    if (ScriptFile* pScript = sResourceMgr->GetScriptFile(ScriptName))
    {
        pMainContext->CallSubroutine(pScript, "chapter.main");
        Run();
    }
    else
        StopInterpreter = true;
}

void NsbInterpreter::ExecuteScriptLocal(const string& ScriptName)
{
    // This leaks memory but nobody really cares
    pMainContext->CallSubroutine(new ScriptFile(ScriptName), "chapter.main");
    Run();
}

void NsbInterpreter::Run()
{
    StartDebugger();
    Threads.push_back(pMainContext);
    do
    {
        auto iter = Threads.begin();
        while (iter != Threads.end())
        {
            pContext = *iter++;
            try
            {
                pContext->Run(this);
            }
            catch (...)
            {
                NsbAssert(false, "Exception caught");
            }
        }
    } while (!StopInterpreter && !Threads.empty());
}

void NsbInterpreter::Stop()
{
    RunInterpreter = true;
    DbgStepping = false;
    StopInterpreter = true;
    pGame->IsRunning = false;
}

void NsbInterpreter::Pause()
{
    RunInterpreter = false;
}

void NsbInterpreter::Start()
{
    if (!DbgStepping)
        RunInterpreter = true;
}

void NsbInterpreter::CallScriptSymbol(const string& Prefix)
{
    string ScriptName = GetVariable<string>(pContext->GetLineArgs()[0]), Symbol;
    size_t i = ScriptName.find("->");
    if (i != string::npos)
    {
        Symbol = ScriptName.substr(i + 2);
        ScriptName.erase(i);
    }
    if (ScriptName == "@") // @-> = this->
        ScriptName = pContext->GetScriptName();
    if (!ScriptName.empty())
        ScriptName.back() = 'b'; // .nss -> .nsb
    CallScript(ScriptName, Prefix + Symbol);
}

template <> void NsbInterpreter::Push(bool Val)
{
    if (Val) Push("true");
    else Push("false");
}

template <> bool NsbInterpreter::Pop()
{
    string Val = Pop<string>();
    if (Val == "true") return true;
    if (Val == "false") return false;
    NsbAssert(false, "Boolean must be either true or false!");
}

void NsbInterpreter::Sleep(int32_t ms)
{
    boost::this_thread::sleep_for(boost::chrono::milliseconds(ms));
}

void NsbInterpreter::SetVariable(const string& Identifier, Variable* pVar)
{
    NsbAssert(!pVar->IsPtr, "Non-literal passed to SetVariable");
    auto iter = Variables.find(Identifier);
    // Variable exists, copy the value
    if (iter != Variables.end())
    {
        iter->second->Value = pVar->Value;
        if (!pVar->IsPtr)
            delete pVar;
    }
    else
    {
        Variable* pNew = pVar;
        // Make a copy of the variable
        if (pVar->IsPtr)
        {
            pNew = new Variable(true);
            pNew->Value = pVar->Value;
        }
        // Turn literal into variable
        else
            pVar->IsPtr = true;
        Variables[Identifier] = pVar;
    }
}

void NsbInterpreter::LoadScript(const string& FileName)
{
    if (ScriptFile* pScript = sResourceMgr->GetScriptFile(FileName))
        LoadedScripts.push_back(pScript);
}

void NsbInterpreter::CallScript(const string& FileName, const string& Symbol)
{
    if (ScriptFile* pScript = sResourceMgr->GetScriptFile(FileName))
        pContext->CallSubroutine(pScript, Symbol);
}

void NsbInterpreter::KeyPressed(sf::Keyboard::Key Key)
{
    if (Key >= sf::Keyboard::A && Key <= sf::Keyboard::Z)
    {
        char c = 'a' + Key - sf::Keyboard::A;
        string Identifier = "$SYSTEM_keydown_" + string(1, c);
        SetVariable(Identifier, new Variable("true"));
    }

    for (uint32_t i = 0; i < Callbacks.size(); ++i)
        if (Callbacks[i].Key == Key)
            CallScript(Callbacks[i].Script, "chapter.main");
}

void NsbInterpreter::Pop()
{
    Variable* pVar = Stack.top();
    if (!pVar->IsPtr)
        delete pVar;
    Stack.pop();
}

void NsbInterpreter::WaitForResume()
{
    while (!RunInterpreter)
        Sleep(1);
}

void NsbInterpreter::DumpState()
{
    std::ofstream Log("state-log.txt");
    pContext->WriteTrace(Log);
}

bool NsbInterpreter::NsbAssert(bool expr, string error)
{
    if (expr)
        return false;

    std::cout << error << '\n';
    pContext->WriteTrace(std::cout);
    return true;
}
