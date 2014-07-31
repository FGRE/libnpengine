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
#include <cstdint>
#include <stack>
#include <queue>
#include <functional>
using namespace std;

class Variable
{
    enum
    {
        INT,
        STRING
    } Tag;
    union
    {
        int32_t Int;
        string* Str;
    } Val;

    Variable();
public:
    ~Variable();

    static Variable* MakeInt(int32_t Int);
    static Variable* MakeString(string Str);
    static Variable* MakeCopy(Variable* pVar);

    int32_t ToInt();
    string ToString();

    static Variable* Add(Variable* pFirst, Variable* pSecond)
    {
        Variable* pThird = nullptr;
        if (pFirst->Tag == INT && pSecond->Tag == INT)
            pThird = MakeInt(pFirst->Val.Int + pSecond->Val.Int);
        if (pFirst->Tag == STRING && pSecond->Tag == STRING)
            pThird = MakeString(*pFirst->Val.Str + *pSecond->Val.Str);
        Destroy(pFirst, pSecond);
        return pThird;
    };

    static Variable* Equal(Variable* pFirst, Variable* pSecond)
    {
        Variable* pThird = nullptr;
        if (pFirst->Tag == INT && pSecond->Tag == INT)
            pThird = MakeInt(pFirst->Val.Int == pSecond->Val.Int);
        else if (pFirst->Tag == STRING && pSecond->Tag == STRING)
            pThird = MakeInt(*pFirst->Val.Str == *pSecond->Val.Str);
        Destroy(pFirst, pSecond);
        return pThird;
    };

    static void Destroy(Variable* pVar1, Variable* pVar2)
    {
        Destroy(pVar1);
        Destroy(pVar2);
    }

    static void Destroy(Variable* pVar)
    {
        if (pVar->Literal)
            delete pVar;
    }

    bool Literal;
};



class ScriptFile;
class Line;
class NSBContext
{
    struct StackFrame
    {
        ScriptFile* pScript;
        uint32_t SourceLine;
    };
public:
    NSBContext(const string& Name);
    ~NSBContext();

    bool Call(ScriptFile* pScript, const string& Symbol);
    void Jump(const string& Symbol);
    const string& GetParam(uint32_t Index);
    int GetNumParams();
    ScriptFile* GetScript();
    Line* GetLine();
    uint32_t GetMagic();
    uint32_t Advance();
    bool Return();

private:
    StackFrame* GetFrame();

    const string Name;
    stack<StackFrame> CallStack;
};

template <class T>
class Queue : public queue<T>
{
    typedef typename queue<T>::size_type size_type;
public:
    T& operator[] (size_type n)
    {
        assert(n < this->c.size());
        return this->c[n];
    }

    size_type size()
    {
        return this->c.size();
    }
};

class Window;
class NSBInterpreter
{
    typedef void (NSBInterpreter::*BuiltinFunc)();
public:
    NSBInterpreter(Window* pWindow);
    virtual ~NSBInterpreter();

    void ExecuteLocalNSS(const string& Filename);
    void Run();

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
    void Add();
    void Substract();
    void Multiply();
    void Divide();
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
    void Select();
    void Break();
    void Jump();
    void AddAssign();
    void SubAssign();
    void WriteFile();
    void ReadFile();
    void CreateTexture();

    int32_t PopInt();
    string PopString();
    Variable* PopVar();
    void PushInt(int32_t Int);
    void PushString(string Str);
    void PushVar(Variable* pVar);
    void Assign_(int Index);

    void IntUnaryOp(function<int(int)> Func);
    void IntBinaryOp(function<int(int, int)> Func);

    ScriptFile* pTest;
    Window* pWindow;
    NSBContext* pContext;
    vector<BuiltinFunc> Builtins;
    Queue<Variable*> Params;
    map<string, Variable*> Variables;
    vector<ScriptFile*> Scripts;
};

#endif
