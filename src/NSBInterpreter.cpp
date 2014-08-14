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
#include "NSBInterpreter.hpp"
#include "NSBContext.hpp"
#include "Texture.hpp"
#include "Window.hpp"
#include "nsbmagic.hpp"
#include "scriptfile.hpp"
#include "npafile.hpp"
#include "fscommon.hpp"
#include <boost/format.hpp>
#include <iostream>
#include <memory>
#include <algorithm>

#define NSB_ERROR(MSG1, MSG2) cout << __PRETTY_FUNCTION__ << ": " << MSG1 << " " << MSG2 << endl;

NSBInterpreter::NSBInterpreter(Window* pWindow) :
pTest(nullptr),
pWindow(pWindow),
pContext(nullptr),
Builtins(MAGIC_UNK119 + 1, {nullptr, 0})
{
    Builtins[MAGIC_FUNCTION_DECLARATION] = { &NSBInterpreter::FunctionDeclaration, 0 };
    Builtins[MAGIC_CALL_FUNCTION] = { &NSBInterpreter::CallFunction, 0 };
    Builtins[MAGIC_CALL_SCENE] = { &NSBInterpreter::CallScene, 0 };
    Builtins[MAGIC_CALL_CHAPTER] = { &NSBInterpreter::CallChapter, 0 };
    Builtins[MAGIC_CMP_LOGICAL_AND] = { &NSBInterpreter::CmpLogicalAnd, 2 };
    Builtins[MAGIC_CMP_LOGICAL_OR] = { &NSBInterpreter::CmpLogicalOr, 2 };
    Builtins[MAGIC_LOGICAL_GREATER_EQUAL] = { &NSBInterpreter::LogicalGreaterEqual, 2 };
    Builtins[MAGIC_LOGICAL_LESS_EQUAL] = { &NSBInterpreter::LogicalLessEqual, 2 };
    Builtins[MAGIC_CMP_GREATER] = { &NSBInterpreter::CmpGreater, 2 };
    Builtins[MAGIC_CMP_LESS] = { &NSBInterpreter::CmpLess, 2 };
    Builtins[MAGIC_CMP_EQUAL] = { &NSBInterpreter::CmpEqual, 2 };
    Builtins[MAGIC_LOGICAL_NOT_EQUAL] = { &NSBInterpreter::LogicalNotEqual, 2 };
    Builtins[MAGIC_LOGICAL_NOT] = { &NSBInterpreter::LogicalNot, 1 };
    Builtins[MAGIC_ADD_EXPRESSION] = { &NSBInterpreter::AddExpression, 2 };
    Builtins[MAGIC_SUB_EXPRESSION] = { &NSBInterpreter::SubExpression, 2 };
    Builtins[MAGIC_MUL_EXPRESSION] = { &NSBInterpreter::MulExpression, 2 };
    Builtins[MAGIC_DIV_EXPRESSION] = { &NSBInterpreter::DivExpression, 2 };
    Builtins[MAGIC_MOD_EXPRESSION] = { &NSBInterpreter::ModExpression, 2 };
    Builtins[MAGIC_INCREMENT] = { &NSBInterpreter::Increment, 1 };
    Builtins[MAGIC_DECREMENT] = { &NSBInterpreter::Decrement, 1 };
    Builtins[MAGIC_LITERAL] = { &NSBInterpreter::Literal, 0 };
    Builtins[MAGIC_ASSIGN] = { &NSBInterpreter::Assign, 1 };
    Builtins[MAGIC_VARIABLE] = { &NSBInterpreter::Get, 0 };
    Builtins[MAGIC_SCOPE_BEGIN] = { &NSBInterpreter::ScopeBegin, 0 };
    Builtins[MAGIC_SCOPE_END] = { &NSBInterpreter::ScopeEnd, 0 };
    Builtins[MAGIC_RETURN] = { &NSBInterpreter::Return, 0 };
    Builtins[MAGIC_END_FUNCTION] = { &NSBInterpreter::Return, 0 };
    Builtins[MAGIC_END_SCENE] = { &NSBInterpreter::Return, 0 };
    Builtins[MAGIC_END_CHAPTER] = { &NSBInterpreter::Return, 0 };
    Builtins[MAGIC_IF] = { &NSBInterpreter::If, 1 };
    Builtins[MAGIC_WHILE] = { &NSBInterpreter::While, 1 };
    Builtins[MAGIC_WHILE_END] = { &NSBInterpreter::WhileEnd, 0 };
    Builtins[MAGIC_SELECT] = { &NSBInterpreter::Select, 0 };
    Builtins[MAGIC_BREAK] = { &NSBInterpreter::Break, 0 };
    Builtins[MAGIC_JUMP] = { &NSBInterpreter::Jump, 0 };
    Builtins[MAGIC_ADD_ASSIGN] = { &NSBInterpreter::AddAssign, 1 };
    Builtins[MAGIC_SUB_ASSIGN] = { &NSBInterpreter::SubAssign, 1 };
    Builtins[MAGIC_WRITE_FILE] = { &NSBInterpreter::WriteFile, 2 };
    Builtins[MAGIC_READ_FILE] = { &NSBInterpreter::ReadFile, 2 };
    Builtins[MAGIC_CREATE_TEXTURE] = { &NSBInterpreter::CreateTexture, 5 };
    Builtins[MAGIC_IMAGE_HORIZON] = { &NSBInterpreter::ImageHorizon, 1 };
    Builtins[MAGIC_IMAGE_VERTICAL] = { &NSBInterpreter::ImageVertical, 1 };
    Builtins[MAGIC_TIME] = { &NSBInterpreter::Time, 0 };
    Builtins[MAGIC_STR_STR] = { &NSBInterpreter::StrStr, 2 };
    Builtins[MAGIC_EXIT] = { &NSBInterpreter::Exit, 0 };
    Builtins[MAGIC_CURSOR_POSITION] = { &NSBInterpreter::CursorPosition, 2 };
    Builtins[MAGIC_MOVE_CURSOR] = { &NSBInterpreter::MoveCursor, 2 };
    Builtins[MAGIC_POSITION] = { &NSBInterpreter::Position, 2 };
    Builtins[MAGIC_WAIT] = { &NSBInterpreter::Wait, 1 };
    Builtins[MAGIC_WAIT_KEY] = { &NSBInterpreter::WaitKey, 1 };
    Builtins[MAGIC_NEGA_EXPRESSION] = { &NSBInterpreter::NegaExpression, 1 };
    Builtins[MAGIC_SYSTEM] = { &NSBInterpreter::System, 3 };
    Builtins[MAGIC_STRING] = { &NSBInterpreter::String, 0 };
    Builtins[MAGIC_VARIABLE_VALUE] = { &NSBInterpreter::VariableValue, 0 };
    Builtins[MAGIC_CREATE_PROCESS] = { &NSBInterpreter::CreateProcess, 5 };
    Builtins[MAGIC_COUNT] = { &NSBInterpreter::Count, 1 };
    Builtins[MAGIC_ARRAY] = { &NSBInterpreter::Array, 0 };
    Builtins[MAGIC_ARRAY_READ] = { &NSBInterpreter::ArrayRead, 0 };
    Builtins[MAGIC_ASSOC_ARRAY] = { &NSBInterpreter::AssocArray, 0 };
    Builtins[MAGIC_GET_MODULE_FILE_NAME] = { &NSBInterpreter::GetModuleFileName, 1 };
    Builtins[MAGIC_REQUEST] = { &NSBInterpreter::Request, 2 };
    Builtins[MAGIC_SET_VERTEX] = { &NSBInterpreter::SetVertex, 3 };
    Builtins[MAGIC_ZOOM] = { &NSBInterpreter::Zoom, 6 };
    Builtins[MAGIC_MOVE] = { &NSBInterpreter::Move, 6 };
    Builtins[MAGIC_APPLY_BLUR] = { &NSBInterpreter::ApplyBlur, 2 };
    Builtins[MAGIC_DRAW_TO_TEXTURE] = { &NSBInterpreter::DrawToTexture, 4 };
    Builtins[MAGIC_CREATE_RENDER_TEXTURE] = { &NSBInterpreter::CreateRenderTexture, 4 };
    Builtins[MAGIC_DRAW_TRANSITION] = { &NSBInterpreter::DrawTransition, 8 };
    Builtins[MAGIC_CREATE_COLOR] = { &NSBInterpreter::CreateColor, 7 };
    Builtins[MAGIC_LOAD_IMAGE] = { &NSBInterpreter::LoadImage, 2 };
    Builtins[MAGIC_FADE] = { &NSBInterpreter::Fade, 5 };
    Builtins[MAGIC_DELETE] = { &NSBInterpreter::Delete, 1 };
    Builtins[MAGIC_CLEAR_PARAMS] = { &NSBInterpreter::ClearParams, 0 };
}

NSBInterpreter::~NSBInterpreter()
{
    delete pTest;
    CacheHolder<Variable>::Clear();
}

void NSBInterpreter::ExecuteLocalNSS(const string& Filename)
{
    pContext = new NSBContext("__nitroscript_main__");
    pTest = new ScriptFile(Filename, ScriptFile::NSS);
    pContext->Call(pTest, "chapter.main");
    pContext->Start();
    Threads.push_back(pContext);
}

void NSBInterpreter::Run(int NumCommands)
{
    for (int i = 0; i < NumCommands; ++i)
        RunCommand();
}

void NSBInterpreter::RunCommand()
{
    if (Threads.empty())
        Exit();

    for (auto i = Threads.begin(); i != Threads.end(); ++i)
    {
        pContext = *i;

        while (pContext->IsActive() && !pContext->IsStarving() && !pContext->IsSleeping() && pContext->Advance() != MAGIC_CLEAR_PARAMS)
            if (pContext->GetMagic() < Builtins.size())
                 Call(pContext->GetMagic());

        ClearParams();
        if (pContext->IsStarving())
        {
            delete pContext;
            i = Threads.erase(i);
        }
    }
}

void NSBInterpreter::HandleEvent(SDL_Event Event)
{
    if (Event.type == SDL_MOUSEBUTTONDOWN)
        pContext->TryWake();
}

void NSBInterpreter::FunctionDeclaration()
{
    Params.Begin(pContext->GetNumParams() - 1);
    for (int i = 1; i < pContext->GetNumParams(); ++i)
        Assign_(i);
}

void NSBInterpreter::CallFunction()
{
    CallFunction_(pContext, pContext->GetParam(0));
}

void NSBInterpreter::CallScene()
{
    CallScriptSymbol("scene.");
}

void NSBInterpreter::CallChapter()
{
    CallScriptSymbol("chapter.");
}

void NSBInterpreter::CmpLogicalAnd()
{
    IntBinaryOp(logical_and<int32_t>());
}

void NSBInterpreter::CmpLogicalOr()
{
    IntBinaryOp(logical_or<int32_t>());
}

void NSBInterpreter::LogicalGreaterEqual()
{
    IntBinaryOp(greater_equal<int32_t>());
}

void NSBInterpreter::CmpGreater()
{
    IntBinaryOp(greater<int32_t>());
}

void NSBInterpreter::CmpLess()
{
    IntBinaryOp(less<int32_t>());
}

void NSBInterpreter::LogicalLessEqual()
{
    IntBinaryOp(less_equal<int32_t>());
}

void NSBInterpreter::CmpEqual()
{
    PushVar(Variable::Equal(PopVar(), PopVar()));
}

void NSBInterpreter::LogicalNotEqual()
{
    CmpEqual();
    Call(MAGIC_LOGICAL_NOT);
}

void NSBInterpreter::LogicalNot()
{
    IntUnaryOp(logical_not<int32_t>());
}

void NSBInterpreter::AddExpression()
{
    Variable* pVar = PopVar();
    PushVar(Variable::Add(pVar, PopVar()));
}

void NSBInterpreter::SubExpression()
{
    IntBinaryOp(minus<int32_t>());
}

void NSBInterpreter::MulExpression()
{
    IntBinaryOp(multiplies<int32_t>());
}

void NSBInterpreter::DivExpression()
{
    IntBinaryOp(divides<int32_t>());
}

void NSBInterpreter::ModExpression()
{
    IntBinaryOp(modulus<int32_t>());
}

void NSBInterpreter::Increment()
{
    IntUnaryOp([](int32_t a) { return ++a; });
}

void NSBInterpreter::Decrement()
{
    IntUnaryOp([](int32_t a) { return --a; });
}

void NSBInterpreter::Literal()
{
    const string& Type = pContext->GetParam(0);
    const string& Val = pContext->GetParam(1);
    if (Type == "STRING")
        PushString(Val);
    else if (Type == "INT")
        PushInt(stoi(Val));
}

void NSBInterpreter::Assign()
{
    if (pContext->GetParam(0) == "__array_variable__")
    {
        Params.Begin(1);
        Variable* pVar = PopVar();
        Variable* pLit = PopVar();
        pVar->Set(pLit);
        Variable::Destroy(pLit);
    }
    else
        Assign_(0);
}

void NSBInterpreter::Get()
{
    PushVar(GetVar(pContext->GetParam(0)));
}

void NSBInterpreter::ScopeBegin()
{
}

void NSBInterpreter::ScopeEnd()
{
}

void NSBInterpreter::Return()
{
    pContext->Return();
}

void NSBInterpreter::If()
{
    if (!PopInt())
        Jump();
}

void NSBInterpreter::While()
{
    pContext->PushBreak();
    If();
}

void NSBInterpreter::WhileEnd()
{
    pContext->PopBreak();
}

void NSBInterpreter::Select()
{
}

void NSBInterpreter::Break()
{
    pContext->Break();
}

void NSBInterpreter::Jump()
{
    pContext->Jump(pContext->GetParam(0));
}

Variable* NSBInterpreter::PopVar()
{
    return Params.Pop();
}

ArrayVariable* NSBInterpreter::PopArr()
{
    return dynamic_cast<ArrayVariable*>(PopVar());
}

int32_t NSBInterpreter::PopInt()
{
    Variable* pVar = PopVar();
    int32_t Val = pVar->ToInt();
    Variable::Destroy(pVar);
    return Val;
}

string NSBInterpreter::PopString()
{
    Variable* pVar = PopVar();
    string Val = pVar->ToString();
    Variable::Destroy(pVar);
    return Val;
}

bool NSBInterpreter::PopBool()
{
    Variable* pVar = PopVar();
    bool Val = pVar->ToBool();
    Variable::Destroy(pVar);
    return Val;
}

uint32_t NSBInterpreter::PopColor()
{
    string ColorStr = PopString();
    transform(ColorStr.begin(), ColorStr.end(), ColorStr.begin(), ::tolower);

    if (ColorStr[0] == '#')
        return stoi(ColorStr.c_str() + 1, nullptr, 16);
    if (ColorStr == "black")
        return 0;
    if (ColorStr == "white")
        return 0xFFFFFF;
    if (ColorStr == "blue")
        return 0xFF;

    NSB_ERROR("Failed to transform string to color", ColorStr);
    return 0;
}

void NSBInterpreter::PushInt(int32_t Int)
{
    PushVar(Variable::MakeInt(Int));
}

void NSBInterpreter::PushString(string Str)
{
    PushVar(Variable::MakeString(Str));
}

void NSBInterpreter::PushVar(Variable* pVar)
{
    Params.Push(pVar);
}

void NSBInterpreter::Assign_(int Index)
{
    SetVar(pContext->GetParam(Index), PopVar());
}

void NSBInterpreter::IntUnaryOp(function<int32_t(int32_t)> Func)
{
    PushVar(PopVar()->IntUnaryOp(Func));
}

void NSBInterpreter::IntBinaryOp(function<int32_t(int32_t, int32_t)> Func)
{
    int32_t Val = PopInt();
    PushInt(Func(Val, PopInt()));
}

void NSBInterpreter::CallFunction_(NSBContext* pThread, const string& Symbol)
{
    string FuncNameFull = string("function.") + Symbol;

    // Find function locally
    if (pThread->Call(pContext->GetScript(), FuncNameFull))
        return;

    // Find function globally
    for (uint32_t i = 0; i < Scripts.size(); ++i)
        if (pThread->Call(Scripts[i], FuncNameFull))
            return;

    NSB_ERROR("Failed to call function", Symbol);
}

void NSBInterpreter::CallScriptSymbol(const string& Prefix)
{
    string ScriptName = GetString(pContext->GetParam(0)), Symbol;
    size_t i = ScriptName.find("->");
    if (i != string::npos)
    {
        Symbol = ScriptName.substr(i + 2);
        ScriptName.erase(i);
    }
    else
        Symbol = "main";
    CallScript(ScriptName == "@" ? pContext->GetScriptName() : ScriptName, Prefix + Symbol);
}

void NSBInterpreter::LoadScript(const string& Filename)
{
    if (ScriptFile* pScript = sResourceMgr->GetScriptFile(Filename))
        Scripts.push_back(pScript);
}

void NSBInterpreter::CallScript(const string& Filename, const string& Symbol)
{
    if (ScriptFile* pScript = sResourceMgr->GetScriptFile(Filename))
        pContext->Call(pScript, Symbol);
}

void NSBInterpreter::Call(uint16_t Magic)
{
    Params.Begin(Builtins[Magic].NumParams);
    if (Builtins[Magic].Func)
        (this->*Builtins[Magic].Func)();
}

string NSBInterpreter::GetString(const string& Name)
{
    if (Name[0] != '$' && Name[0] != '#')
        return Name;
    return GetVar(Name)->ToString();
}

Variable* NSBInterpreter::GetVar(const string& Name)
{
    return CacheHolder<Variable>::Read(Name);
}

ArrayVariable* NSBInterpreter::GetArr(const string& Name)
{
    return dynamic_cast<ArrayVariable*>(GetVar(Name));
}

Object* NSBInterpreter::GetObject(const string& Name)
{
    return CacheHolder<Object>::Read(Name);
}

void NSBInterpreter::SetVar(const string& Name, Variable* pVar)
{
    // Variable exists: Copy value
    if (Variable* pVar2 = GetVar(Name))
    {
        pVar2->Set(pVar);
        return;
    }

    // Variable doesnt exist: Create it
    Variable* pNew = nullptr;
    // Reuse literal as new variable
    if (pVar->Literal)
        pNew = pVar;
    // Not a literal, so make a copy
    else
        pNew = Variable::MakeCopy(pVar);
    pNew->Literal = false;
    CacheHolder<Variable>::Write(Name, pNew);
}

void NSBInterpreter::SetInt(const string& Name, int32_t Val)
{
    SetVar(Name, Variable::MakeInt(Val));
}

void NSBInterpreter::AddAssign()
{
    Variable* pVar = GetVar(pContext->GetParam(0));
    Variable* pNew = Variable::Add(pVar, PopVar());
    pVar->Set(pNew);
    Variable::Destroy(pNew);
}

void NSBInterpreter::SubAssign()
{
    Variable* pVar = GetVar(pContext->GetParam(0));
    Variable* pNew = Variable::MakeInt(pVar->ToInt() + PopInt());
    pVar->Set(pNew);
    Variable::Destroy(pNew);
}

void NSBInterpreter::WriteFile()
{
    string Filename = PopString();
    string Data = NpaFile::FromUtf8(PopString());
    uint32_t Size = Data.size();
    fs::WriteFileDirectory(Filename, NpaFile::Encrypt(&Data[0], Size), Size);
}

void NSBInterpreter::ReadFile()
{
    string Filename = PopString();
    uint32_t Size;
    char* pData = fs::ReadFile(Filename, Size);
    NpaFile::Decrypt(pData, Size);
    PushString(NpaFile::ToUtf8(pData));
    delete[] pData;
}

void NSBInterpreter::CreateTexture()
{
    string Handle = PopString();
    int32_t Priority = PopInt();
    int32_t X = PopInt();
    int32_t Y = PopInt();
    string Filename = PopString();

    Texture* pTexture = new Texture;
    pTexture->LoadFromFile(Filename);
    pTexture->SetPosition(X, Y);
    pTexture->SetPriority(Priority);

    pWindow->AddTexture(pTexture);
    CacheHolder<Object>::Write(Handle, pTexture);
}

void NSBInterpreter::ImageHorizon()
{
    Texture* pTexture = Get<Texture>(PopString());
    PushInt(pTexture->GetWidth());
}

void NSBInterpreter::ImageVertical()
{
    Texture* pTexture = Get<Texture>(PopString());
    PushInt(pTexture->GetHeight());
}

void NSBInterpreter::Time()
{
    PushInt(time(0));
}

void NSBInterpreter::StrStr()
{
    string Haystack = PopString();
    string Needle = PopString();
    PushInt(Haystack.find(Needle) + 1);
}

void NSBInterpreter::Exit()
{
    pWindow->Exit();
}

void NSBInterpreter::CursorPosition()
{
    int32_t X, Y;
    SDL_PumpEvents();
    SDL_GetMouseState(&X, &Y);
    SetInt(PopString(), X);
    SetInt(PopString(), Y);
}

void NSBInterpreter::MoveCursor()
{
    int32_t X = PopInt();
    pWindow->MoveCursor(X, PopInt());
}

void NSBInterpreter::Position()
{
    Texture* pTexture = Get<Texture>(PopString());
    SetInt(PopString(), pTexture->GetX());
    SetInt(PopString(), pTexture->GetY());
}

void NSBInterpreter::Wait()
{
    pContext->Wait(PopInt());
}

void NSBInterpreter::WaitKey()
{
    pContext->Wait(PopInt(), true);
}

void NSBInterpreter::NegaExpression()
{
    IntUnaryOp(negate<int32_t>());
}

void NSBInterpreter::System()
{
    static const string OpenStr = "OPEN:";
    string Command = PopString();
    string Parameters = PopString();
    string Directory = PopString();
    if (Command.substr(0, OpenStr.size()) != OpenStr)
        return;

    Command = Command.substr(OpenStr.size());

    if (fork() == 0)
        execlp("/usr/bin/xdg-open", "/usr/bin/xdg-open", Command.c_str(), NULL);
}

void NSBInterpreter::String()
{
    Params.Begin(pContext->GetNumParams());
    boost::format Fmt(PopString());
    for (int i = 1; i < pContext->GetNumParams(); ++i)
    {
        Variable* pVar = PopVar();
        if (pVar->IsString())
            Fmt % pVar->ToString();
        else if (pVar->IsInt())
            Fmt % pVar->ToInt();
        Variable::Destroy(pVar);
    }
    PushString(Fmt.str());
}

void NSBInterpreter::VariableValue()
{
    Params.Begin(pContext->GetNumParams());
    string Type = PopString();
    string Name = PopString();
    if (pContext->GetNumParams() == 3)
        SetVar(Type + Name, PopVar());
    else if (pContext->GetNumParams() == 2)
        PushVar(GetVar(Type + Name));
    else
        assert(false && "This will trigger when we get new season of Haruhi");
}

void NSBInterpreter::CreateProcess()
{
    string Handle = PopString();
    /*int32_t unk1 = */PopInt();
    /*int32_t unk2 = */PopInt();
    /*int32_t unk3 = */PopInt();
    string Symbol = PopString();

    NSBContext* pThread = new NSBContext(Handle);
    CallFunction_(pThread, Symbol);
    Threads.push_back(pThread);
    CacheHolder<Object>::Write(Handle, pThread);
}

void NSBInterpreter::Count()
{
    PushInt(PopArr()->Members.size());
}

void NSBInterpreter::Array()
{
    Params.Begin(pContext->GetNumParams());
    ArrayVariable* pArr = PopArr();
    if (!pArr)
    {
        pArr = new ArrayVariable;
        CacheHolder<Variable>::Write(pContext->GetParam(0), pArr);
    }

    for (int i = 1; i < pContext->GetNumParams(); ++i)
        pArr->Push(ArrayVariable::MakeCopy(PopVar()));
}

void NSBInterpreter::ArrayRead()
{
    ArrayVariable* pArr = GetArr(pContext->GetParam(0));
    int32_t Depth = stoi(pContext->GetParam(1));
    Params.Begin(Depth);
    while (Depth --> 0)
    {
        Variable* pVar = PopVar();
        if (pVar->IsInt())
            pArr = pArr->Find(pVar->ToInt());
        else
            pArr = pArr->Find(pVar->ToString());
        Variable::Destroy(pVar);
    }
    PushVar(pArr);
}

void NSBInterpreter::AssocArray()
{
    Params.Begin(pContext->GetNumParams());
    ArrayVariable* pArr = PopArr();
    for (auto i = pArr->Members.begin(); i != pArr->Members.end(); ++i)
        i->first = PopString();
}

void NSBInterpreter::GetModuleFileName()
{
    string Name = pContext->GetScriptName();
    PushString(Name.substr(4, Name.size() - 8)); // Remove nss/ and .nsb
}

void NSBInterpreter::Request()
{
    if (Object* pObj = GetObject(PopString()))
        pObj->Request(PopString());
}

void NSBInterpreter::SetVertex()
{
    Texture* pTexture = Get<Texture>(PopString());
    int32_t X = PopInt();
    int32_t Y = PopInt();

    if (pTexture)
        pTexture->SetVertex(X, Y);
}

void NSBInterpreter::Zoom()
{
    Texture* pTexture = Get<Texture>(PopString());
    int32_t Time = PopInt();
    int32_t X = PopInt();
    int32_t Y = PopInt();
    /*string Tempo = */PopString();
    bool Wait = PopBool();

    if (pTexture)
        pTexture->Zoom(Time, X, Y);

    if (Wait)
        pContext->Wait(Time);
}

void NSBInterpreter::Move()
{
    Texture* pTexture = Get<Texture>(PopString());
    int32_t Time = PopInt();
    int32_t X = PopInt();
    int32_t Y = PopInt();
    /*string Tempo = */PopString();
    bool Wait = PopBool();

    if (pTexture)
        pTexture->Move(Time, X, Y);

    if (Wait)
        pContext->Wait(Time);
}

void NSBInterpreter::ApplyBlur()
{
    if (Texture* pTexture = Get<Texture>(PopString()))
        pTexture->ApplyBlur(PopString());
}

void NSBInterpreter::DrawToTexture()
{
    Texture* pTexture = Get<Texture>(PopString());
    int32_t X = PopInt();
    int32_t Y = PopInt();
    string Filename = PopString();

    if (pTexture)
        pTexture->Draw(X, Y, Filename);
}

void NSBInterpreter::CreateRenderTexture()
{
    string Handle = PopString();
    int32_t Width = PopInt();
    int32_t Height = PopInt();
    uint32_t Color = PopColor();

    Texture* pTexture = new Texture;
    pTexture->LoadFromColor(Width, Height, Color);
    CacheHolder<Object>::Write(Handle, pTexture);
}

void NSBInterpreter::DrawTransition()
{
    Texture* pTexture = Get<Texture>(PopString());
    int32_t Time = PopInt();
    int32_t Start = PopInt();
    int32_t End = PopInt();
    /*int32_t unk1 = */PopInt();
    /*string Tempo = */PopString();
    string Filename = PopString();
    bool Wait = PopBool();

    if (pTexture)
        pTexture->DrawTransition(Time, Start, End, Filename);

    if (Wait)
        pContext->Wait(Time);
}

void NSBInterpreter::CreateColor()
{
    string Handle = PopString();
    int32_t Priority = PopInt();
    int32_t X = PopInt();
    int32_t Y = PopInt();
    int32_t Width = PopInt();
    int32_t Height = PopInt();
    uint32_t Color = PopColor();

    Texture* pTexture = new Texture;
    pTexture->LoadFromColor(Width, Height, Color);
    pTexture->SetPosition(X, Y);
    pTexture->SetPriority(Priority);

    pWindow->AddTexture(pTexture);
    CacheHolder<Object>::Write(Handle, pTexture);
}

void NSBInterpreter::LoadImage()
{
    string Handle = PopString();
    string Filename = PopString();

    Texture* pTexture = new Texture;
    pTexture->LoadFromFile(Filename);
    CacheHolder<Object>::Write(Handle, pTexture);
}

void NSBInterpreter::Fade()
{
    Texture* pTexture = Get<Texture>(PopString());
    int32_t Time = PopInt();
    int32_t Opacity = PopInt();
    /*string Tempo = */PopString();
    bool Wait = PopBool();

    if (pTexture)
        pTexture->Fade(Time, Opacity);

    if (Wait)
        pContext->Wait(Time);
}

void NSBInterpreter::Delete()
{
    if (Texture* pTexture = Get<Texture>(PopString()))
    {
        pWindow->RemoveTexture(pTexture);
        delete pTexture;
    }
}

void NSBInterpreter::ClearParams()
{
    Params.Reset();
}
