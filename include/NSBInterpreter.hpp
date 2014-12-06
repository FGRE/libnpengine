/* 
 * libnpengine: Nitroplus script interpreter
 * Copyright (C) 2014 Mislav Blažević <krofnica996@gmail.com>
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
#include <stack>
#include <deque>
#include <functional>
#include <queue>
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

class Window;
class Texture;
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

    void PushEvent(const SDL_Event& Event);
    void HandleEvent(const SDL_Event& Event);
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
    void ArrayRead();
    void AssocArray();
    void ModuleFileName();
    void Request();
    void SetVertex();
    void Zoom();
    void Move();
    void ApplyBlur();
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

    int32_t PopInt();
    string PopString();
    bool PopBool();
    PosFunc PopPos();
    uint32_t PopColor();
    Variable* PopVar();
    ArrayVariable* PopArr();

    void PushInt(int32_t Int);
    void PushString(string Str);
    void PushVar(Variable* pVar);
    void Assign_(int Index);

    void IntUnaryOp(function<int32_t(int32_t)> Func);
    void IntBinaryOp(function<int32_t(int32_t, int32_t)> Func);
    void BoolBinaryOp(function<bool(bool, bool)> Func);

    void SetInt(const string& Name, int32_t Val);
    void SetVar(const string& Name, Variable* pVar);
    string GetString(const string& Name);
    Variable* GetVar(const string& Name);
    ArrayVariable* GetArr(const string& Name);
    Object* GetObject(const string& Name);
    template <class T> T* Get(const string& Name);
    void CallFunction_(NSBContext* pThread, const string& Symbol);
    void CallScriptSymbol(const string& Prefix);
    void CallScript(const string& Filename, const string& Symbol);
    void Call(uint16_t Magic);
    bool SelectEvent();

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
    Holder<Variable> LocalVariableHolder;
    ObjectHolder_t ObjectHolder;
};

template <class T> T* NSBInterpreter::Get(const string& Name)
{
    return dynamic_cast<T*>(GetObject(Name));
}

#endif
