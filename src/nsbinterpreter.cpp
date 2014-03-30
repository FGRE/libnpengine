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
    //Builtins[MAGIC_CREATE_PROCESS] = &NsbInterpreter::CreateProcess;
    Builtins[MAGIC_LOAD_TEXTURE_CLIP] = &NsbInterpreter::LoadTextureClip;
    Builtins[MAGIC_INCREMENT] = &NsbInterpreter::Increment;
    Builtins[MAGIC_LOGICAL_GREATER] = &NsbInterpreter::LogicalGreater;
    Builtins[MAGIC_LOGICAL_LESS] = &NsbInterpreter::LogicalLess;
    Builtins[MAGIC_ARRAY_SIZE] = &NsbInterpreter::ArraySize;
    Builtins[MAGIC_SET_VERTEX] = &NsbInterpreter::SetVertex;
    Builtins[MAGIC_ZOOM] = &NsbInterpreter::Zoom;
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
    Builtins[MAGIC_CALL] = &NsbInterpreter::Call;
    Builtins[MAGIC_ADD] = &NsbInterpreter::Add;
    Builtins[MAGIC_DELETE] = &NsbInterpreter::Delete;
    Builtins[MAGIC_FADE] = &NsbInterpreter::Fade;
    Builtins[MAGIC_BIND_IDENTIFIER] = &NsbInterpreter::NSBBindIdentifier;
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
    //Builtins[MAGIC_SET_ALIAS] = &NsbInterpreter::SetAlias;
    //Builtins[MAGIC_SET_FONT_ATTRIBUTES] = &NsbInterpreter::SetFontAttributes;
    //Builtins[MAGIC_SET_TEXTBOX_ATTRIBUTES] = &NsbInterpreter::SetTextboxAttributes;
    //Builtins[MAGIC_PLACEHOLDER_PARAM] = &NsbInterpreter::PlaceholderParam;

    // Stubs
    Builtins[MAGIC_UNK77] = &NsbInterpreter::UNK77;

    // Hack
    SetVariable("#SYSTEM_cosplay_patch", Variable("false"));

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
    LoadScript("nss/sys_title.nsb");
}

void NsbInterpreter::ExecuteScript(const string& ScriptName)
{
    if (ScriptFile* pScript = sResourceMgr->GetScriptFile(ScriptName))
    {
        pMainContext->pScript = pScript;
        Run();
    }
    else
        StopInterpreter = true;
}

void NsbInterpreter::ExecuteScriptLocal(const string& ScriptName)
{
    // This leaks memory but nobody really cares
    pMainContext->pScript = new ScriptFile(ScriptName);
    Run();
}

void NsbInterpreter::Run()
{
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

void NsbInterpreter::StringToVariable()
{
    // TODO: Type may be INT!

    // Set
    if (pContext->pLine->Params.size() == 3)
        SetVariable(Params[0].Value + Params[1].Value, { "STRING", GetParam<string>(2) });
    // Get
    else if (pContext->pLine->Params.size() == 2)
    {
        Params[0] = { "STRING", GetVariable<string>(Params[0].Value + Params[1].Value) };
        Params.resize(1);
    }
    else
        assert(false && "This will trigger when we get new season of Haruhi");
}

template <class T>
void NsbInterpreter::LogicalOperator(std::function<bool(T, T)> Func)
{
    T First = Pop<T>();
    T Second = Pop<T>();
    bool Val = Func(First, Second);
    if (Val) Stack.push(new Variable("true"));
    else Stack.push(new Variable("false"));
}

void NsbInterpreter::ArraySize()
{
    Params.back() = Variable("INT", boost::lexical_cast<string>(Arrays[pContext->pLine->Params[0]].Members.size()));
}

void NsbInterpreter::Jump()
{
    pContext->pScript->SetSourceIter(pContext->pScript->GetSymbol(pContext->pLine->Params[0]));
}

void NsbInterpreter::SetVertex()
{
    if (Drawable* pDrawable = (Drawable*)CacheHolder<DrawableBase>::Read(GetParam<string>(0)))
        pDrawable->SetCenter(GetParam<int32_t>(1), GetParam<int32_t>(2));
}

void NsbInterpreter::CallScene()
{
    CallScriptSymbol("scene.");
}

void NsbInterpreter::CallChapter()
{
    CallScriptSymbol("chapter.");
}

void NsbInterpreter::CallScriptSymbol(const string& Prefix)
{
    string ScriptName = GetParam<string>(0), Symbol;
    size_t i = ScriptName.find("->");
    if (i != string::npos)
    {
        Symbol = ScriptName.substr(i + 2);
        ScriptName.erase(i);
    }
    if (!ScriptName.empty()) // TODO: Where did @ disappear?
        ScriptName.back() = 'b'; // .nss -> .nsb
    else
        ScriptName = pContext->pScript->GetName();
    CallScript(ScriptName, Prefix + Symbol);
}

void NsbInterpreter::Zoom()
{
    bool Wait = Pop<bool>();
    string Tempo = Pop<string>();
    int32_t Y = Pop<int32_t>();
    int32_t X = Pop<int32_t>();
    int32_t Time = Pop<int32_t>();
    string HandleName = Pop<string>();
    if (Drawable* pDrawable = (Drawable*)CacheHolder<DrawableBase>::Read(HandleName))
        pGame->GLCallback(std::bind(&NsbInterpreter::GLZoom, this, pDrawable, Time, X, Y, Tempo, Wait));
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
    string Directory = Pop<string>();
    string Parameters = Pop<string>();
    string Command = Pop<string>();
    NSBSystem(Command, Parameters, Directory);
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

void NsbInterpreter::CreateSound()
{
    string File = Pop<string>(0);
    string Type = Pop<string>(0);
    HandleName = GetParam<string>(0);
    NSBCreateSound(Type, File + ".ogg");
}

void NsbInterpreter::CreateWindow()
{
    bool unk1 = Pop<bool>();
    int32_t Height = Pop<int32_t>();
    int32_t Width = Pop<int32_t>();
    int32_t Y = Pop<int32_t>();
    int32_t X = Pop<int32_t>();
    int32_t unk0 = Pop<int32_t>();
    HandleName = Pop<string>();
    NSBCreateWindow(unk0, X, Y, Width, Height, unk1);
}

void NsbInterpreter::ApplyBlur()
{
    string Heaviness = Pop<string>();
    string HandleName = Pop<string>();
    if (Drawable* pDrawable = (Drawable*)CacheHolder<DrawableBase>::Read(HandleName))
        pGame->GLCallback(std::bind(&NsbInterpreter::GLApplyBlur, this, pDrawable, Heaviness);
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
    // TODO: Can second parameter be a variable and not a literal?
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
    HandleName = Pop<string>();
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

void NsbInterpreter::Substract()
{
    BinaryOperator([] (int32_t a, int32_t b) -> int32_t { return a - b; });
}

void NsbInterpreter::Add()
{
    uint32_t First = Params.size() - 2, Second = Params.size() - 1;
    // STRING + STRING = STRING
    // STRING + INT = STRING
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
    if (Params.size() < 2)
        return;

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

template <class T> T NsbInterpreter::Pop()
{
    Variable* pVar = Stack.top();
    T Ret;
    if (T* pT = boost::get<T>(pVar->Value))
        Ret = *pT;
    if (!pVar->IsPtr)
        delete pVar;
    Stack.pop();
    return Ret;
}

template <> bool NsbInterpreter::Pop()
{
    string Val = Pop<string>();
    if (Val == "true") return true;
    if (Val == "false") return false;
    assert(false && "Boolean must be either true or false!");
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

    std::cout << error << '\n';
    WriteTrace(std::cout);
    return true;
}
