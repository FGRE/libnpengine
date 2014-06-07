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
#include "playable.hpp"
#include "npafile.hpp"

#include <fstream>
#include <iostream>
#include <thread>

std::map<std::string, std::string> ObjectHolder::Aliases;

NsbInterpreter::NsbInterpreter() :
StopInterpreter(false),
pGame(nullptr),
pDebuggerThread(nullptr),
DbgStepping(false),
BreakOnAssert(false),
EventLoop(false)
{
    MagicBreakpoints.resize(MAGIC_UNK119 + 1, false);

    Builtins.resize(MAGIC_UNK119 + 1, nullptr);
    Builtins[MAGIC_BREAK] = &NsbInterpreter::Break;
    Builtins[MAGIC_JUMP] = &NsbInterpreter::Jump;
    Builtins[MAGIC_LOGICAL_AND] = &NsbInterpreter::LogicalAnd;
    Builtins[MAGIC_LOGICAL_OR] = &NsbInterpreter::LogicalOr;
    Builtins[MAGIC_LOGICAL_GREATER_EQUAL] = &NsbInterpreter::LogicalGreaterEqual;
    Builtins[MAGIC_LOGICAL_LESS_EQUAL] = &NsbInterpreter::LogicalLessEqual;
    Builtins[MAGIC_SUBSTRACT] = &NsbInterpreter::Substract;
    Builtins[MAGIC_IMAGE_HORIZON] = &NsbInterpreter::ImageHorizon;
    Builtins[MAGIC_IMAGE_VERTICAL] = &NsbInterpreter::ImageVertical;
    Builtins[MAGIC_SHAKE] = &NsbInterpreter::Shake;
    Builtins[MAGIC_TIME] = &NsbInterpreter::Time;
    Builtins[MAGIC_CALL_SCENE] = &NsbInterpreter::CallScene;
    Builtins[MAGIC_CREATE_SCROLLBAR] = &NsbInterpreter::CreateScrollbar;
    Builtins[MAGIC_SYSTEM] = &NsbInterpreter::System;
    Builtins[MAGIC_LOAD_TEXTURE_CLIP] = &NsbInterpreter::LoadTextureClip;
    Builtins[MAGIC_INCREMENT] = &NsbInterpreter::Increment;
    Builtins[MAGIC_LOGICAL_GREATER] = &NsbInterpreter::LogicalGreater;
    Builtins[MAGIC_LOGICAL_LESS] = &NsbInterpreter::LogicalLess;
    Builtins[MAGIC_COUNT] = &NsbInterpreter::Count;
    Builtins[MAGIC_SET_VERTEX] = &NsbInterpreter::SetVertex;
    Builtins[MAGIC_ZOOM] = &NsbInterpreter::Zoom;
    Builtins[MAGIC_NEGATIVE] = &NsbInterpreter::Negative;
    Builtins[MAGIC_ARRAY] = &NsbInterpreter::Array;
    Builtins[MAGIC_ASSIGN] = &NsbInterpreter::Assign;
    Builtins[MAGIC_ARRAY_READ] = &NsbInterpreter::ArrayRead;
    Builtins[MAGIC_SET_SHORTCUT] = &NsbInterpreter::SetShortcut;
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
    Builtins[MAGIC_REMAIN_TIME] = &NsbInterpreter::RemainTime;
    Builtins[MAGIC_LITERAL] = &NsbInterpreter::Literal;
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
    Builtins[MAGIC_ASSOC_ARRAY] = &NsbInterpreter::AssocArray;
    Builtins[MAGIC_FUNCTION_BEGIN] = &NsbInterpreter::Begin;
    Builtins[MAGIC_CALL_CHAPTER] = &NsbInterpreter::CallChapter;
    Builtins[MAGIC_IF] = &NsbInterpreter::If;
    Builtins[MAGIC_WHILE] = &NsbInterpreter::While;
    Builtins[MAGIC_LOGICAL_NOT] = &NsbInterpreter::LogicalNot;
    Builtins[MAGIC_LOGICAL_EQUAL] = &NsbInterpreter::LogicalEqual;
    Builtins[MAGIC_LOGICAL_NOT_EQUAL] = &NsbInterpreter::LogicalNotEqual;
    Builtins[MAGIC_FUNCTION_END] = &NsbInterpreter::End;
    Builtins[MAGIC_END_SCENE] = &NsbInterpreter::End;
    Builtins[MAGIC_END_CHAPTER] = &NsbInterpreter::End;
    Builtins[MAGIC_FWN_UNK] = &NsbInterpreter::End; // Fuwanovel hack, unknown purpose
    Builtins[MAGIC_CLEAR_PARAMS] = &NsbInterpreter::ClearParams;
    Builtins[MAGIC_GET_MODULE_FILE_NAME] = &NsbInterpreter::GetModuleFileName;
    Builtins[MAGIC_SCOPE_BEGIN] = &NsbInterpreter::ScopeBegin;
    Builtins[MAGIC_SCOPE_END] = &NsbInterpreter::ScopeEnd;
    Builtins[MAGIC_STRING] = &NsbInterpreter::String;
    Builtins[MAGIC_WRITE_FILE] = &NsbInterpreter::WriteFile;
    Builtins[MAGIC_DIVIDE] = &NsbInterpreter::Divide;
    Builtins[MAGIC_MULTIPLY] = &NsbInterpreter::Multiply;
    Builtins[MAGIC_RETURN] = &NsbInterpreter::End;
    Builtins[MAGIC_VARIABLE_VALUE] = &NsbInterpreter::VariableValue;
    Builtins[MAGIC_LOAD_IMAGE] = &NsbInterpreter::LoadImage;
    Builtins[MAGIC_READ_FILE] = &NsbInterpreter::ReadFile;
    Builtins[MAGIC_SET_ALIAS] = &NsbInterpreter::SetAlias;
    Builtins[MAGIC_CREATE_CHOICE] = &NsbInterpreter::CreateChoice;
    Builtins[MAGIC_SET_NEXT_FOCUS] = &NsbInterpreter::SetNextFocus;
    Builtins[MAGIC_SET_FREQUENCY] = &NsbInterpreter::SetFrequency;
    Builtins[MAGIC_SET_PAN] = &NsbInterpreter::SetPan;
    Builtins[MAGIC_SOUND_AMPLITUDE] = &NsbInterpreter::SoundAmplitude;
    Builtins[MAGIC_SELECT] = &NsbInterpreter::Select;
    Builtins[MAGIC_CASE_BEGIN] = &NsbInterpreter::CaseBegin;
    Builtins[MAGIC_EXIT] = &NsbInterpreter::Exit;
    Builtins[MAGIC_STR_STR] = &NsbInterpreter::StrStr;
    Builtins[MAGIC_ADD_ASSIGN] = &NsbInterpreter::AddAssign;
    Builtins[MAGIC_UNK90] = &NsbInterpreter::UNK90;
    Builtins[MAGIC_DURATION_TIME] = &NsbInterpreter::DurationTime;
    //Builtins[MAGIC_CREATE_PROCESS] = &NsbInterpreter::CreateProcess;
    //Builtins[MAGIC_SET_FONT_ATTRIBUTES] = &NsbInterpreter::SetFontAttributes;
    //Builtins[MAGIC_SET_TEXTBOX_ATTRIBUTES] = &NsbInterpreter::SetTextboxAttributes;
    //Builtins[MAGIC_PLACEHOLDER_PARAM] = &NsbInterpreter::PlaceholderParam;

    // Stubs
    Builtins[MAGIC_UNK20] = &NsbInterpreter::UNK20;
    Builtins[MAGIC_UNK63] = &NsbInterpreter::UNK63;
    Builtins[MAGIC_UNK77] = &NsbInterpreter::UNK77;
    Builtins[MAGIC_UNK101] = &NsbInterpreter::UNK101;
    Builtins[MAGIC_UNK115] = &NsbInterpreter::UNK115;
    Builtins[MAGIC_LOCK_VIDEO] = &NsbInterpreter::LockVideo;
    Builtins[MAGIC_SAVE] = &NsbInterpreter::Save;
    Builtins[MAGIC_DELETE_SAVE_FILE] = &NsbInterpreter::DeleteSaveFile;
    Builtins[MAGIC_CONQUEST] = &NsbInterpreter::Conquest;
    Builtins[MAGIC_CLEAR_SCORE] = &NsbInterpreter::ClearScore;
    Builtins[MAGIC_CLEAR_BACKLOG] = &NsbInterpreter::ClearBacklog;

    // Main script thread
    pMainContext = new NsbContext("__nitroscript_main__");
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
    LoadScript("nss/sys_save_function.nsb");

    for (char i = 'a'; i <= 'z'; ++i)
    {
        string Identifier = "$SYSTEM_keydown_" + string(1, i);
        SetVariable(Identifier, new Variable("false"));
    }

    SetVariable("$BOX_Init", new Variable("false"));
    SetVariable("$BGM_Init", new Variable("false"));
    SetVariable("#N2systemVERSION_old", new Variable("false"));
    SetVariable("$format", new Variable("false"));
    SetVariable("$DebugMode", new Variable("false"));
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
    if (std::ifstream("DEBUG"))
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
        std::this_thread::sleep_for(std::chrono::microseconds(100));
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
    if (Symbol.empty()) // CallChapter requires this
        Symbol = "main";
    CallScript(ScriptName, Prefix + Symbol);
}

template <> void NsbInterpreter::Push(bool Val)
{
    if (Val) Push("true");
    else Push("false");
    // TODO: What if Val != "false"
}

template <> bool NsbInterpreter::Pop()
{
    bool Val = true; // TODO: hack if NsbAssert fails
    if (int32_t* pInt = boost::get<int32_t>(&Stack.top()->Value))
    {
        Val = (*pInt) != 0;
        Pop();
    }
    else
    {
        string SVal = Pop<string>();
        if (SVal == "true") Val = true;
        else if (SVal == "false") Val = false;
        else NsbAssert(false, "Boolean must be either true or false");
    }
    return Val;
}

template <> PosFunc NsbInterpreter::Pop()
{
    PosFunc Func = boost::apply_visitor(SpecialPosVisitor(), Stack.top()->Value);
    Pop();
    return Func;
}

void NsbInterpreter::Sleep(int32_t ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void NsbInterpreter::SetVariable(const string& Identifier, Variable* pVar)
{
    SetVariable(Identifier, pVar, Variables);
}

void NsbInterpreter::SetLocalVariable(const string& Identifier, Variable* pVar)
{
    SetVariable(Identifier, pVar, LocalVariables);
}

void NsbInterpreter::SetVariable(const string& Identifier, Variable* pVar, VariableStore& Container)
{
    auto iter = Container.find(Identifier);
    // Variable exists, copy the value
    if (iter != Container.end())
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
            pNew = new Variable;
            pNew->Value = pVar->Value;
        }
        // Turn literal into variable
        pNew->IsPtr = true;
        // Set
        Container[Identifier] = pNew;
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
void NsbInterpreter::HandleEvent(sf::Event Event)
{
    switch (Event.type)
    {
        case sf::Event::KeyPressed:
            KeyPressed(Event.key.code);
            break;
        case sf::Event::MouseButtonPressed:
            MouseClicked(Event.mouseButton);
            break;
        case sf::Event::MouseMoved:
            MouseMoved(sf::Mouse::getPosition(*pGame));
            break;
        default:
            break;
    }
}

void NsbInterpreter::MouseMoved(sf::Vector2i Pos)
{
}

void NsbInterpreter::MouseClicked(sf::Event::MouseButtonEvent Event)
{
    if (!EventLoop)
        return;

    for (auto iter = CacheHolder<Button>::Cache.begin();
         iter != CacheHolder<Button>::Cache.end(); ++iter)
         iter->second->CheckClicked(Event.x, Event.y);
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

DrawableBase* NsbInterpreter::GetDrawable(bool Expect)
{
    return Get<DrawableBase>(Expect);
}

Playable* NsbInterpreter::GetPlayable(bool Expect)
{
    return Get<Playable>(Expect);
}

Object* NsbInterpreter::GetObject(bool Expect)
{
    return Get<Object>(Expect);
}

void NsbInterpreter::WaitForResume()
{
    while (!RunInterpreter)
        Sleep(1);
}

bool NsbInterpreter::NsbAssert(bool expr, string error)
{
    if (expr)
        return false;

    std::cout << error << '\n';
    pContext->WriteTrace(std::cout);
    if (BreakOnAssert) DbgStepping = true;
    return true;
}
