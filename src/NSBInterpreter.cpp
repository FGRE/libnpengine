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
#include "Movie.hpp"
#include "Text.hpp"
#include "nsbmagic.hpp"
#include "nsbconstants.hpp"
#include "scriptfile.hpp"
#include "npafile.hpp"
#include "fscommon.hpp"
#include <boost/format.hpp>
#include <iostream>
#include <memory>
#include <algorithm>

#define NSB_ERROR(MSG1, MSG2) cout << __PRETTY_FUNCTION__ << ": " << MSG1 << " " << MSG2 << endl;
#define NSB_VARARGS 0xFF

extern "C" { void gst_init(int* argc, char** argv[]); }

NSBInterpreter::NSBInterpreter(Window* pWindow) :
pWindow(pWindow),
pContext(nullptr),
Builtins(MAGIC_UNK119 + 1, {nullptr, 0})
{
    gst_init(nullptr, nullptr);

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
    Builtins[MAGIC_SELECT_END] = { &NSBInterpreter::SelectEnd, 0 };
    Builtins[MAGIC_SELECT_BREAK_END] = { &NSBInterpreter::SelectBreakEnd, 0 };
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
    Builtins[MAGIC_WAIT_KEY] = { &NSBInterpreter::WaitKey, NSB_VARARGS };
    Builtins[MAGIC_NEGA_EXPRESSION] = { &NSBInterpreter::NegaExpression, 1 };
    Builtins[MAGIC_SYSTEM] = { &NSBInterpreter::System, 3 };
    Builtins[MAGIC_STRING] = { &NSBInterpreter::String, NSB_VARARGS };
    Builtins[MAGIC_VARIABLE_VALUE] = { &NSBInterpreter::VariableValue, NSB_VARARGS };
    Builtins[MAGIC_CREATE_PROCESS] = { &NSBInterpreter::CreateProcess, 5 };
    Builtins[MAGIC_COUNT] = { &NSBInterpreter::Count, 1 };
    Builtins[MAGIC_ARRAY] = { &NSBInterpreter::Array, NSB_VARARGS };
    Builtins[MAGIC_ARRAY_READ] = { &NSBInterpreter::ArrayRead, 0 };
    Builtins[MAGIC_ASSOC_ARRAY] = { &NSBInterpreter::AssocArray, NSB_VARARGS };
    Builtins[MAGIC_MODULE_FILE_NAME] = { &NSBInterpreter::ModuleFileName, 0 };
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
    Builtins[MAGIC_SET_LOOP] = { &NSBInterpreter::SetLoop, 2 };
    Builtins[MAGIC_SET_VOLUME] = { &NSBInterpreter::SetVolume, 4 };
    Builtins[MAGIC_SET_LOOP_POINT] = { &NSBInterpreter::SetLoopPoint, 3 };
    Builtins[MAGIC_CREATE_SOUND] = { &NSBInterpreter::CreateSound, 3 };
    Builtins[MAGIC_REMAIN_TIME] = { &NSBInterpreter::RemainTime, 1 };
    Builtins[MAGIC_CREATE_MOVIE] = { &NSBInterpreter::CreateMovie, 8 };
    Builtins[MAGIC_DURATION_TIME] = { &NSBInterpreter::DurationTime, 1 };
    Builtins[MAGIC_SET_FREQUENCY] = { &NSBInterpreter::SetFrequency, 4 };
    Builtins[MAGIC_SET_PAN] = { &NSBInterpreter::SetPan, 4 };
    Builtins[MAGIC_SET_ALIAS] = { &NSBInterpreter::SetAlias, 2 };
    Builtins[MAGIC_CREATE_NAME] = { &NSBInterpreter::CreateName, 1 };
    Builtins[MAGIC_CREATE_WINDOW] = { &NSBInterpreter::CreateWindow, 7 };
    Builtins[MAGIC_CREATE_CHOICE] = { &NSBInterpreter::CreateChoice, NSB_VARARGS };
    Builtins[MAGIC_CASE] = { &NSBInterpreter::Case, 0 };
    Builtins[MAGIC_CASE_END] = { &NSBInterpreter::CaseEnd, 0 };
    Builtins[MAGIC_SET_NEXT_FOCUS] = { &NSBInterpreter::SetNextFocus, 3 };
    Builtins[MAGIC_PASSAGE_TIME] = { &NSBInterpreter::PassageTime, 1 };
    Builtins[MAGIC_PARSE_TEXT] = { &NSBInterpreter::ParseText, 0 };
    Builtins[MAGIC_LOAD_TEXT] = { &NSBInterpreter::LoadText, 7 };
    Builtins[MAGIC_WAIT_TEXT] = { &NSBInterpreter::WaitText, 2 };
    Builtins[MAGIC_LOCK_VIDEO] = { &NSBInterpreter::LockVideo, 1 };
    Builtins[MAGIC_SAVE] = { &NSBInterpreter::Save, 1 };
    Builtins[MAGIC_DELETE_SAVE_FILE] = { &NSBInterpreter::DeleteSaveFile, 1};
    Builtins[MAGIC_CONQUEST] = { &NSBInterpreter::Conquest, 3 };
    Builtins[MAGIC_CLEAR_SCORE] = { &NSBInterpreter::ClearScore, 1 };
    Builtins[MAGIC_CLEAR_BACKLOG] = { &NSBInterpreter::ClearBacklog, 0 };
    Builtins[MAGIC_SET_FONT] = { &NSBInterpreter::SetFont, 6 };
    Builtins[MAGIC_SET_SHORTCUT] = { &NSBInterpreter::SetShortcut, 2 };
    Builtins[MAGIC_CREATE_CLIP_TEXTURE] = { &NSBInterpreter::CreateClipTexture, 9 };
    Builtins[MAGIC_EXIST_SAVE] = { &NSBInterpreter::ExistSave, 1 };

    pContext = new NSBContext("__nitroscript_main__");
    pContext->Start();
    Threads.push_back(pContext);
}

NSBInterpreter::~NSBInterpreter()
{
}

void NSBInterpreter::ExecuteLocalScript(const string& Filename)
{
    pContext->Call(new ScriptFile(Filename, ScriptFile::NSS), "chapter.main");
}

void NSBInterpreter::ExecuteScript(const string& Filename)
{
    CallScript(Filename, "chapter.main");
}

void NSBInterpreter::Run(int NumCommands)
{
    sResourceMgr->GetScriptFile("nss/0_boot.nsb")->GetLine(626)->Magic = MAGIC_CLEAR_PARAMS;
    sResourceMgr->GetScriptFile("nss/0_boot.nsb")->GetLine(633)->Magic = MAGIC_CLEAR_PARAMS;
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

void NSBInterpreter::PushEvent(const SDL_Event& Event)
{
    Events.push(Event);
}

void NSBInterpreter::HandleEvent(const SDL_Event& Event)
{
    if (Event.type == SDL_MOUSEBUTTONDOWN)
        for (auto pContext : Threads)
            pContext->TryWake();

    if (Event.type == SDL_KEYDOWN)
        for (NSBShortcut& Shortcut : Shortcuts)
            if (Shortcut.Key == Event.key.keysym.sym)
                CallScript(Shortcut.Script, "chapter.main");
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
    BoolBinaryOp(logical_and<bool>());
}

void NSBInterpreter::CmpLogicalOr()
{
    BoolBinaryOp(logical_or<bool>());
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
    PushVar(Variable::MakeInt(!PopBool()));
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
    {
        if (Variable* pVar = LocalVariableHolder.Read(Val))
        {
            PushVar(pVar);
            LocalVariableHolder.Write(Val, nullptr);
        }
        else if (Nsb::IsValidConstant(Val))
            PushInt(Nsb::ConstantToValue(Val));
        else
            PushString(Val);
    }
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
        if (pVar)
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
    if (!PopBool())
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

/*
 * Select an event for next iteration of event loop. (see: Case)
 * Valid events are: Mouse Up/Down/Wheel, Arrow Up/Down/Right/Left
 * */
void NSBInterpreter::Select()
{
    pWindow->Select(true);
    if (SelectEvent())
        pContext->PushBreak();
}

void NSBInterpreter::SelectEnd()
{
    SelectEvent();
}

void NSBInterpreter::SelectBreakEnd()
{
    pContext->PopBreak();
    pWindow->Select(false);
    Events = queue<SDL_Event>();
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
    // TODO: This is a nasty hack (see: ArrayVariable::Find(int32_t))
    Variable* pVar = Params.Pop();
    if (!pVar)
        return Variable::MakeInt(0);
    return pVar;
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
    string Val;
    if (!pVar->IsInt())
        Val = pVar->ToString();
    Variable::Destroy(pVar);
    return Val;
}

bool NSBInterpreter::PopBool()
{
    return static_cast<bool>(PopInt());
}

PosFunc NSBInterpreter::PopPos()
{
    const int32_t WIDTH = pWindow->WIDTH;
    const int32_t HEIGHT = pWindow->HEIGHT;
    static const size_t SPECIAL_POS_NUM = 9;
    static const PosFunc SpecialPosTable[SPECIAL_POS_NUM] =
    {
        [WIDTH] (int32_t x) { return WIDTH / 2 - x / 2; },
        [HEIGHT] (int32_t y) { return HEIGHT - y; },
        [HEIGHT] (int32_t y) { return HEIGHT / 2 - y / 2; },
        [] (int32_t x) { return 0; },
        [] (int32_t y) { return 0; },
        [] (int32_t y) { return 0; },
        [] (int32_t x) { return 0; },
        [] (int32_t x) { return 0; },
        [] (int32_t y) { return 0; }
    };
    static const string SpecialPos[SPECIAL_POS_NUM] =
    {
        "center", "inbottom", "middle",
        "onleft", "outtop", "intop",
        "outright", "left", "top"
    };

    PosFunc Func = nullptr;
    Variable* pVar = PopVar();
    if (pVar->IsString())
    {
        string Str = pVar->ToString();
        transform(Str.begin(), Str.end(), Str.begin(), ::tolower);
        size_t i = -1;
        while (++i < SPECIAL_POS_NUM)
            if (Str == SpecialPos[i])
                Func = SpecialPosTable[i];
    }
    else
    {
        int32_t Val = pVar->ToInt();
        Func = [Val] (int32_t) { return Val; };
    }
    Variable::Destroy(pVar);
    return Func;
}

uint32_t NSBInterpreter::PopColor()
{
    uint32_t Color;

    Variable* pVar = PopVar();
    if (pVar->IsString())
        Color = stoi(pVar->ToString().c_str() + 1, nullptr, 16) | (0xFF << 24);
    else
        Color = pVar->ToInt();

    Variable::Destroy(pVar);
    return Color;
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
    string Name = pContext->GetParam(Index);
    Variable* pVar = PopVar();
    if (Name[0] == '$')
        SetVar(Name, pVar);
    else
        LocalVariableHolder.Write(Name, pVar);
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

void NSBInterpreter::BoolBinaryOp(function<bool(bool, bool)> Func)
{
    bool Val = PopBool();
    PushInt(Func(Val, PopBool()));
}

void NSBInterpreter::CallFunction_(NSBContext* pThread, const string& Symbol)
{
    if (!pThread->Call(pContext->GetScript(), string("function.") + Symbol))
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

void NSBInterpreter::CallScript(const string& Filename, const string& Symbol)
{
    if (ScriptFile* pScript = sResourceMgr->GetScriptFile(Filename))
        pContext->Call(pScript, Symbol);
}

void NSBInterpreter::Call(uint16_t Magic)
{
    uint8_t NumParams = Builtins[Magic].NumParams;
    Params.Begin(NumParams == NSB_VARARGS ? pContext->GetNumParams() : NumParams);

    if (Builtins[Magic].Func)
        (this->*Builtins[Magic].Func)();
}

bool NSBInterpreter::SelectEvent()
{
    if (!Events.empty())
    {
        Event = Events.front();
        Events.pop();
        return true;
    }
    else
    {
        pContext->Rewind();
        pContext->Wait(20);
        return false;
    }
}

string NSBInterpreter::GetString(const string& Name)
{
    if (Name[0] != '$' && Name[0] != '#')
        return Name;
    return GetVar(Name)->ToString();
}

Variable* NSBInterpreter::GetVar(const string& Name)
{
    return VariableHolder.Read(Name);
}

ArrayVariable* NSBInterpreter::GetArr(const string& Name)
{
    return dynamic_cast<ArrayVariable*>(GetVar(Name));
}

Object* NSBInterpreter::GetObject(const string& Name)
{
    return ObjectHolder.Read(Name);
}

void NSBInterpreter::SetVar(const string& Name, Variable* pVar)
{
    // Variable exists: Copy value
    if (Variable* pVar2 = GetVar(Name))
    {
        pVar2->Set(pVar);
        Variable::Destroy(pVar);
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
    VariableHolder.Write(Name, pNew);
}

void NSBInterpreter::SetInt(const string& Name, int32_t Val)
{
    SetVar(Name, Variable::MakeInt(Val));
}

void NSBInterpreter::AddAssign()
{
    Variable* pVar = GetVar(pContext->GetParam(0));
    if (pVar)
    {
        Variable* pNew = Variable::Add(pVar, PopVar());
        pVar->Set(pNew);
        Variable::Destroy(pNew);
    }
    else
        SetVar(pContext->GetParam(0), PopVar());
}

void NSBInterpreter::SubAssign()
{
    Variable* pVar = GetVar(pContext->GetParam(0));
    Variable* pNew = Variable::MakeInt(pVar->ToInt() - PopInt());
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
    PosFunc X = PopPos();
    PosFunc Y = PopPos();
    string Source = PopString();

    Texture* pTexture = new Texture;
    pTexture->LoadFromFile(Source);
    pTexture->SetPosition(X(pTexture->GetWidth()), Y(pTexture->GetHeight()));
    pTexture->SetPriority(Priority);

    pWindow->AddTexture(pTexture);
    ObjectHolder.Write(Handle, pTexture);
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
    pContext->Wait(pContext->GetNumParams() == 1 ? PopInt() : ~0, true);
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
    ObjectHolder.Write(Handle, pThread);
}

void NSBInterpreter::Count()
{
    PushInt(PopArr()->Members.size());
}

void NSBInterpreter::Array()
{
    ArrayVariable* pArr = PopArr();
    if (!pArr)
    {
        pArr = new ArrayVariable;
        VariableHolder.Write(pContext->GetParam(0), pArr);
    }

    for (int i = 1; i < pContext->GetNumParams(); ++i)
        pArr->Push(ArrayVariable::MakeCopy(PopVar()));
}

void NSBInterpreter::ArrayRead()
{
    ArrayVariable* pArr = GetArr(pContext->GetParam(0));
    int32_t Depth = stoi(pContext->GetParam(1));
    Params.Begin(Depth);
    while (Depth --> 0 && pArr)
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
    ArrayVariable* pArr = PopArr();
    for (auto i = pArr->Members.begin(); i != pArr->Members.end(); ++i)
        i->first = PopString();
}

void NSBInterpreter::ModuleFileName()
{
    string Name = pContext->GetScriptName();
    PushString(Name.substr(4, Name.size() - 8)); // Remove nss/ and .nsb
}

void NSBInterpreter::Request()
{
    string Handle = PopString();
    int32_t Request = PopInt();

    ObjectHolder.Execute(Handle, [Request] (map<string, Object*>::iterator i)
    {
        if (i->second)
            i->second->Request(Request);
    });
}

void NSBInterpreter::SetVertex()
{
    Texture* pTexture = Get<Texture>(PopString());
    PosFunc X = PopPos();
    PosFunc Y = PopPos();

    if (pTexture)
        pTexture->SetVertex(X(pTexture->GetWidth()), Y(pTexture->GetHeight()));
}

void NSBInterpreter::Zoom()
{
    string Handle = PopString();
    int32_t Time = PopInt();
    int32_t X = PopInt();
    int32_t Y = PopInt();
    /*int32_t Tempo = */PopInt();
    bool Wait = PopBool();

    ObjectHolder.Execute(Handle, [Time, X, Y] (map<string, Object*>::iterator i)
    {
        if (Texture* pTexture = dynamic_cast<Texture*>(i->second))
            pTexture->Zoom(Time, X, Y);
    });

    if (Wait)
        pContext->Wait(Time);
}

void NSBInterpreter::Move()
{
    string Handle = PopString();
    int32_t Time = PopInt();
    int32_t X = PopInt();
    int32_t Y = PopInt();
    /*int32_t Tempo = */PopInt();
    bool Wait = PopBool();

    ObjectHolder.Execute(Handle, [Time, X, Y] (map<string, Object*>::iterator i)
    {
        if (Texture* pTexture = dynamic_cast<Texture*>(i->second))
            pTexture->Move(Time, X, Y);
    });

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
    ObjectHolder.Write(Handle, pTexture);
}

void NSBInterpreter::DrawTransition()
{
    Texture* pTexture = Get<Texture>(PopString());
    int32_t Time = PopInt();
    int32_t Start = PopInt();
    int32_t End = PopInt();
    /*int32_t unk1 = */PopInt();
    /*int32_t Tempo = */PopInt();
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
    PosFunc X = PopPos();
    PosFunc Y = PopPos();
    int32_t Width = PopInt();
    int32_t Height = PopInt();
    uint32_t Color = PopColor();

    Texture* pTexture = new Texture;
    pTexture->LoadFromColor(Width, Height, Color);
    pTexture->SetPosition(X(Width), Y(Height));
    pTexture->SetPriority(Priority);

    pWindow->AddTexture(pTexture);
    ObjectHolder.Write(Handle, pTexture);
}

void NSBInterpreter::LoadImage()
{
    string Handle = PopString();
    string Filename = PopString();

    Texture* pTexture = new Texture;
    pTexture->LoadFromFile(Filename);
    ObjectHolder.Write(Handle, pTexture);
}

void NSBInterpreter::Fade()
{
    string Handle = PopString();
    int32_t Time = PopInt();
    int32_t Opacity = PopInt();
    /*int32_t Tempo = */PopInt();
    bool Wait = PopBool();

    ObjectHolder.Execute(Handle, [Time, Opacity] (map<string, Object*>::iterator i)
    {
        if (Texture* pTexture = dynamic_cast<Texture*>(i->second))
            pTexture->Fade(Time, Opacity);
    });

    if (Wait)
        pContext->Wait(Time);
}

void NSBInterpreter::Delete()
{
    string Handle = PopString();

    ObjectHolder.Execute(Handle, [this] (map<string, Object*>::iterator i)
    {
        if (i->second)
        {
            i->second->Delete(pWindow);
            i->second = nullptr;
        }
        delete i->second;
    });
}

void NSBInterpreter::ClearParams()
{
    Params.Reset();
}

void NSBInterpreter::SetLoop()
{
    if (Playable* pPlayable = Get<Playable>(PopString()))
        pPlayable->SetLoop(PopBool());
}

void NSBInterpreter::SetVolume()
{
    string Handle = PopString();
    int32_t Time = PopInt();
    int32_t Volume = PopInt();
    /*int32_t Tempo = */PopInt();

    ObjectHolder.Execute(Handle, [Time, Volume] (map<string, Object*>::iterator i)
    {
        if (Playable* pPlayable = dynamic_cast<Playable*>(i->second))
            pPlayable->SetVolume(Time, Volume);
    });
}

void NSBInterpreter::SetLoopPoint()
{
    string Handle = PopString();
    int32_t Begin = PopInt();
    int32_t End = PopInt();

    if (Playable* pPlayable = Get<Playable>(Handle))
        pPlayable->SetLoopPoint(Begin, End);
}

void NSBInterpreter::CreateSound()
{
    string Handle = PopString();
    /*string Type = */PopString();
    string File = PopString();

    if (File.substr(File.size() - 4) != ".ogg")
        File += ".ogg";

    Resource Res = sResourceMgr->GetResource(File);
    if (Res.IsValid())
        ObjectHolder.Write(Handle, new Playable(Res));
}

void NSBInterpreter::RemainTime()
{
    Playable* pPlayable = Get<Playable>(PopString());
    PushInt(pPlayable ? pPlayable->RemainTime() : 0);
}

void NSBInterpreter::CreateMovie()
{
    string Handle = PopString();
    int32_t Priority = PopInt();
    /*PosFunc X = */PopPos();
    /*PosFunc Y = */PopPos();
    bool Loop = PopBool();
    bool Alpha = PopBool();
    string File = PopString();
    bool Audio = PopBool();

    Movie* pMovie = new Movie(File, pWindow, Priority, Alpha, Audio);
    pMovie->SetLoop(Loop);
    ObjectHolder.Write(Handle, pMovie);
}

void NSBInterpreter::DurationTime()
{
    Playable* pPlayable = Get<Playable>(PopString());
    PushInt(pPlayable ? pPlayable->DurationTime() : 0);
}

void NSBInterpreter::SetFrequency()
{
    string Handle = PopString();
    int32_t Time = PopInt();
    int32_t Frequency = PopInt();
    /*int32_t Tempo = */PopInt();

    if (Playable* pPlayable = Get<Playable>(Handle))
        pPlayable->SetFrequency(Time, Frequency);
}

void NSBInterpreter::SetPan()
{
    string Handle = PopString();
    int32_t Time = PopInt();
    int32_t Pan = PopInt();
    /*int32_t Tempo = */PopInt();

    if (Playable* pPlayable = Get<Playable>(Handle))
        pPlayable->SetPan(Time, Pan);
}

void NSBInterpreter::SetAlias()
{
    string Handle = PopString();
    ObjectHolder.WriteAlias(Handle, PopString());
}

void NSBInterpreter::CreateName()
{
    ObjectHolder.Write(PopString(), new Name);
}

void NSBInterpreter::CreateWindow()
{
    string Handle = PopString();
    /*int32_t unk = */PopInt();

    Window_t* pWindow = new Window_t;
    pWindow->X = PopInt();
    pWindow->Y = PopInt();
    pWindow->Width = PopInt();
    pWindow->Height = PopInt();
    /*bool unk = */PopBool();

    ObjectHolder.Write(Handle, pWindow);
}

void NSBInterpreter::CreateChoice()
{
    ObjectHolder.Write(PopString(), new Choice);
}

/*
 * Check if selected (see: Select) event satisfies a case.
 * In practise, this checks if button was clicked, and if so, jumps to
 * beginning of case code block. Otherwise, jump over the case block.
 * After the code block is executed, no other cases will be checked.
 *
 * NOTE: For unknown reason, MAGIC_CASE's last parameter is label which
 * points to beginning of the case, even though case code block always
 * begins on the next line of code.
 * */
void NSBInterpreter::Case()
{
    bool Choose = false;
    if (Choice* pChoice = Get<Choice>(pContext->GetParam(0)))
        Choose = pChoice->IsSelected(Event);

    pContext->Jump(Choose ? pContext->GetParam(2) : pContext->GetParam(1));
}

void NSBInterpreter::CaseEnd()
{
}

void NSBInterpreter::SetNextFocus()
{
    Choice* pFirst = Get<Choice>(PopString());
    Choice* pSecond = Get<Choice>(PopString());
    string Key = PopString();

    if (pFirst && pSecond)
        pFirst->SetNextFocus(pSecond, Key);
}

void NSBInterpreter::PassageTime()
{
    PushInt(0);
}

void NSBInterpreter::ParseText()
{
    string Handle = pContext->GetParam(0);
    string Box = pContext->GetParam(1);
    string XML = pContext->GetParam(2);

    if (Variable* pVar = GetVar("$SYSTEM_present_text"))
    {
        string OldHandle = pVar->ToString();
        if (Text* pText = Get<Text>(OldHandle))
        {
            ObjectHolder.Write(OldHandle, nullptr);
            delete pText;
        }
    }

    Text* pText = new Text(XML);
    Handle = Box + "/" + Handle;
    SetVar("$SYSTEM_present_text", Variable::MakeString(Handle));
    ObjectHolder.Write(Handle, pText);

    // [HACK]
    pWindow->SetText(pText);
}

void NSBInterpreter::LoadText()
{
    /*string unk = */PopString();
    /*string unk = */PopString();
    string TextHandle = PopString();
    int32_t Width = PopInt();
    /*int32_t unk = */PopInt();
    /*int32_t unk = */PopInt();
    /*int32_t unk = */PopInt();

    if (Text* pText = Get<Text>(TextHandle))
        pText->SetWrap(Width);
}

void NSBInterpreter::WaitText()
{
    string Handle = PopString();
    /*string unk = */PopString();

    if (Text* pText = Get<Text>(Handle))
    {
        pText->Advance();
        pContext->WaitText(pText);
    }
}

void NSBInterpreter::LockVideo()
{
    bool Lock = PopBool();
}

void NSBInterpreter::Save()
{
    int32_t Slot = PopInt();
}

void NSBInterpreter::DeleteSaveFile()
{
    int32_t Slot = PopInt();
}

void NSBInterpreter::Conquest()
{
    /*string unk = */PopString();
    /*string unk = */PopString();
    /*string unk = */PopBool();

    // [HACK]
    PushInt(0);
}

void NSBInterpreter::ClearScore()
{
    /*string unk = */PopString();
}

void NSBInterpreter::ClearBacklog()
{
}

void NSBInterpreter::SetFont()
{
    string Font = PopString();
    int32_t Size = PopInt();
    uint32_t InColor = PopColor();
    uint32_t OutColor = PopColor();
    int32_t Weight = PopInt();
    /*string unk = */PopString();
}

void NSBInterpreter::SetShortcut()
{
    string Key = PopString();
    string Script = PopString();
    Shortcuts.push_back({SDLK_a + Key[0] - 'A', Script});
}

void NSBInterpreter::CreateClipTexture()
{
    string Handle = PopString();
    int32_t Priority = PopInt();
    PosFunc X1 = PopPos();
    PosFunc Y1 = PopPos();
    PosFunc X2 = PopPos();
    PosFunc Y2 = PopPos();
    int32_t Width = PopInt();
    int32_t Height = PopInt();
    string Source = PopString();
}

void NSBInterpreter::ExistSave()
{
    int32_t Slot = PopInt();
}
