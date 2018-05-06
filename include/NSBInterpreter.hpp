/* 
 * libnpengine: Nitroplus script interpreter
 * Copyright (C) 2014-2016,2018 Mislav Blažević <krofnica996@gmail.com>
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
#ifndef NSB_INTERPRETER_HPP
#define NSB_INTERPRETER_HPP

#include "Choice.hpp"
#include "ArrayVariable.hpp"
#include <SDL2/SDL.h>
#include <functional>
#include <queue>
#include <thread>
using namespace std;

class Stack
{
public:
    Stack() : ReadIndex(0), WriteIndex(0)
    {
    }

    void Push(Variable* pVar)
    {
        if (WriteIndex == Params.size())
            Params.push_back(pVar);
        else
            Params[WriteIndex] = pVar;
        WriteIndex++;
    }

    Variable* Pop()
    {
        return Params[ReadIndex++];
    }

    void Begin(size_t Size)
    {
        WriteIndex -= Size;
        ReadIndex = WriteIndex;
    }

    void Reset()
    {
        ReadIndex = WriteIndex = 0;
        Params.clear();
    }

private:
    vector<Variable*> Params;
    size_t ReadIndex;
    size_t WriteIndex;
};

typedef function<int32_t(int32_t)> PosFunc;
struct NSBPosition
{
    NSBPosition() : Func(nullptr) { }
    PosFunc Func;
    bool Relative;
    int32_t operator()(int32_t xy, int32_t Old = 0) { return Relative ? Old + Func(xy) : Func(xy); }
};

class Line;
class Window;
class Texture;
class Playable;
class NSBContext;
class NSBInterpreter
{
    struct NSBFunction
    {
        typedef void (NSBInterpreter::*BuiltinFunc)();
        NSBFunction(BuiltinFunc Func, uint8_t NumParams) : Func(Func), NumParams(NumParams) { }
        BuiltinFunc Func;
        uint8_t NumParams;
    };
    struct NSBShortcut
    {
        SDL_Keycode Key;
        const string Script;
    };
public:
    NSBInterpreter(Window* pWindow);
    virtual ~NSBInterpreter();

    void ExecuteLocalScript(const string& Filename);
    void ExecuteScript(const string& Filename);
    void StartDebugger();

    void PushEvent(const SDL_Event& Event);
    virtual void HandleEvent(const SDL_Event& Event);
    void Run(int NumCommands);
    void RunCommand();

protected:
    void FunctionDeclaration();
    void CallFunction();
    void CallScene();
    void CallChapter();
    void CmpLogicalAnd();
    void CmpLogicalOr();
    void CmpGreater();
    void CmpLess();
    void LogicalGreaterEqual();
    void LogicalLessEqual();
    void CmpEqual();
    void LogicalNotEqual();
    void LogicalNot();
    void AddExpression();
    void SubExpression();
    void MulExpression();
    void DivExpression();
    void ModExpression();
    void Increment();
    void Decrement();
    void Literal();
    void Assign();
    void Get();
    void ScopeBegin();
    void ScopeEnd();
    void Return();
    void If();
    void While();
    void WhileEnd();
    void Select();
    void SelectEnd();
    void SelectBreakEnd();
    void Break();
    void Jump();
    void AddAssign();
    void SubAssign();
    void WriteFile();
    void ReadFile();
    void CreateTexture();
    void ImageHorizon();
    void ImageVertical();
    void Time();
    void StrStr();
    void Exit();
    void CursorPosition();
    void MoveCursor();
    void Position();
    void Wait();
    void WaitKey();
    void NegaExpression();
    void System();
    void String();
    void VariableValue();
    void CreateProcess();
    void Count();
    void Array();
    void SubScript();
    void AssocArray();
    void ModuleFileName();
    void Request();
    void SetVertex();
    void Zoom();
    void Move();
    void SetShade();
    void DrawToTexture();
    void CreateRenderTexture();
    void DrawTransition();
    void CreateColor();
    void LoadImage();
    void Fade();
    void Delete();
    void ClearParams();
    void SetLoop();
    void SetVolume();
    void SetLoopPoint();
    void CreateSound();
    void RemainTime();
    void CreateMovie();
    void DurationTime();
    void SetFrequency();
    void SetPan();
    void SetAlias();
    void CreateName();
    void CreateWindow();
    void CreateChoice();
    void Case();
    void CaseEnd();
    void SetNextFocus();
    void PassageTime();
    void ParseText();
    void LoadText();
    void WaitText();
    void LockVideo();
    void Save();
    void DeleteSaveFile();
    void Conquest();
    void ClearScore();
    void ClearBacklog();
    void SetFont();
    void SetShortcut();
    void CreateClipTexture();
    void ExistSave();
    void WaitAction();
    void Load();
    void SetBacklog();
    void CreateText();
    void AtExpression();
    void Random();
    void CreateEffect();
    void SetTone();
    void DateTime();
    void Shake();
    void MoviePlay();
    void SetStream();
    void WaitPlay();
    void WaitFade();
    void SoundAmplitude();
    void Rotate();
    void Message();
    void Integer();

    int32_t PopInt();
    string PopString();
    NSBPosition PopPos();
    uint32_t PopColor();
    int32_t PopRequest();
    int32_t PopTone();
    int32_t PopEffect();
    int32_t PopShade();
    int32_t PopTempo();
    bool PopBool();
    string PopSave();
    Variable* PopVar();
    ArrayVariable* PopArr();
    Texture* PopTexture();
    Playable* PopPlayable();

    void PushInt(int32_t Int);
    void PushString(const string& Str);
    void PushVar(Variable* pVar);
    void Assign_(int Index);

    void IntUnaryOp(function<int32_t(int32_t)> Func);
    void IntBinaryOp(function<int32_t(int32_t, int32_t)> Func);
    void BoolBinaryOp(function<bool(bool, bool)> Func);

    void SetInt(const string& Name, int32_t Val);
    void SetString(const string& Name, const string& Val);
    void SetVar(const string& Name, Variable* pVar);
    virtual void OnVariableChanged(const string& Name);
    int32_t GetInt(const string& Name);
    string GetString(const string& Name);
    bool GetBool(const string& Name);
    bool ToBool(Variable* pVar);
    Variable* GetVar(const string& Name);
    ArrayVariable* GetArrSafe(const string& Name);
    ArrayVariable* GetArr(const string& Name);
    Object* GetObject(const string& Name);
    template <class T> T* Get(const string& Name);
    void CallFunction_(NSBContext* pThread, const string& Symbol);
    void CallScriptSymbol(const string& Prefix);
    void CallScript(const string& Filename, const string& Symbol);
    void Call(uint16_t Magic);
    bool SelectEvent();
    void AddThread(NSBContext* pThread);
    void RemoveThread(NSBContext* pThread);
    void ProcessKey(int Key, const string& Val);
    void ProcessButton(int button, const string& Val);

    void DebuggerMain();
    void Inspect(int32_t n);
    void DbgBreak(bool Break);
    void DebuggerTick();
    void PrintArray(ArrayVariable* pArray, const string& Identifier, int Depth = 1);
    void PrintVariable(Variable* pVar, const string& Identifier);
    void SetBreakpoint(const string& Script, int32_t LineNumber);
    string Disassemble(Line* pLine);
    thread* pDebuggerThread;
    bool LogCalls;
    bool DbgStepping;
    bool RunInterpreter;
    list<pair<string, uint32_t>> Breakpoints;

    bool ThreadsModified;
    SDL_Event Event;
    queue<SDL_Event> Events;
    Window* pWindow;
    NSBContext* pContext;
    vector<NSBFunction> Builtins;
    Stack Params;
    vector<NSBShortcut> Shortcuts;
    vector<ScriptFile*> Scripts;
    list<NSBContext*> Threads;
    Holder<Variable> VariableHolder;
    Holder<ArrayVariable> ArrayHolder;
    ObjectHolder_t ObjectHolder;
};

template <class T> T* NSBInterpreter::Get(const string& Name)
{
    return dynamic_cast<T*>(GetObject(Name));
}

#endif
