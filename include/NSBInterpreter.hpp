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

#include "ResourceMgr.hpp"
#include "Variable.hpp"
#include <SDL2/SDL.h>
#include <stack>
#include <deque>
#include <list>
#include <functional>
using namespace std;

class Object;
typedef CacheHolder<Object> ObjectHolder;

class ArrayVariable : public Variable
{
public:
    ArrayVariable();

    ArrayVariable* Find(const string& Key);
    ArrayVariable* Find(int32_t Index);
    void Push(ArrayVariable* pVar);

    static ArrayVariable* MakeCopy(Variable* pVar);

    list<pair<string, ArrayVariable*>> Members;
};

class Window;
class Texture;
class NSBContext;
class NSBInterpreter
{
    typedef void (NSBInterpreter::*BuiltinFunc)();
public:
    NSBInterpreter(Window* pWindow);
    virtual ~NSBInterpreter();

    void ExecuteLocalNSS(const string& Filename);
    void HandleEvent(SDL_Event Event);
    void Run(int NumCommands);
    void RunCommand();

private:
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
    void GetModuleFileName();
    void Request();

    int32_t PopInt();
    string PopString();
    Variable* PopVar();
    ArrayVariable* PopArr();
    void PushInt(int32_t Int);
    void PushString(string Str);
    void PushVar(Variable* pVar);
    void Assign_(int Index);

    void IntUnaryOp(function<int32_t(int32_t)> Func);
    void IntBinaryOp(function<int32_t(int32_t, int32_t)> Func);

    void SetInt(const string& Name, int32_t Val);
    void SetVar(const string& Name, Variable* pVar);
    string GetString(const string& Name);
    Variable* GetVar(const string& Name);
    ArrayVariable* GetArr(const string& Name);
    Object* GetObject(const string& Name);
    template <class T> T* Get(const string& Name);
    void CallFunction_(NSBContext* pThread, const string& Symbol);
    void CallScriptSymbol(const string& Prefix);
    void LoadScript(const string& Filename);
    void CallScript(const string& Filename, const string& Symbol);

    ScriptFile* pTest;
    Window* pWindow;
    NSBContext* pContext;
    vector<BuiltinFunc> Builtins;
    deque<Variable*> Params;
    vector<ScriptFile*> Scripts;
    list<NSBContext*> Threads;
};

template <class T> T* NSBInterpreter::Get(const string& Name)
{
    return dynamic_cast<T*>(GetObject(Name));
}

#endif
