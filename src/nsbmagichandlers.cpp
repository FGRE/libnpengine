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
#include "nsbinterpreter.hpp"
#include "resourcemgr.hpp"
#include "game.hpp"
#include "drawable.hpp"
#include "playable.hpp"
#include "text.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

static const std::string SpecialPos[SPECIAL_POS_NUM] =
{
    "Center", "InBottom", "Middle",
    "OnLeft", "OutTop", "InTop",
    "OutRight"
};

void NsbInterpreter::Time()
{
    Stack.push(new Variable(time(0)));
}

void NsbInterpreter::Shake()
{
}

void NsbInterpreter::CreateScrollbar()
{
}

void NsbInterpreter::LoadImage()
{
    string File = Pop<string>();
    HandleName = Pop<string>();
    pGame->GLCallback(std::bind(&NsbInterpreter::GLLoadImage, this, File));
}

void NsbInterpreter::TextureWidth()
{
    if (DrawableBase* pDrawable = CacheHolder<DrawableBase>::Read(Pop<string>()))
        Stack.push(new Variable(pDrawable->ToSprite()->getTexture()->getSize().x));
}

void NsbInterpreter::TextureHeight()
{
    if (DrawableBase* pDrawable = CacheHolder<DrawableBase>::Read(Pop<string>()))
        Stack.push(new Variable(pDrawable->ToSprite()->getTexture()->getSize().y));
}

void NsbInterpreter::LoadTextureClip()
{
    string File = Pop<string>();
    int32_t Height = Pop<int32_t>();
    int32_t Width = Pop<int32_t>();
    int32_t Ty = Pop<int32_t>();
    int32_t Tx = Pop<int32_t>();
    int32_t X = Pop<int32_t>();
    int32_t Y = Pop<int32_t>();
    int32_t Priority = Pop<int32_t>();
    HandleName = Pop<string>();
    pGame->GLCallback(std::bind(&NsbInterpreter::GLLoadTextureClip, this, Priority, X, Y, Tx, Ty, Width, Height, File));
}

void NsbInterpreter::CreateProcess()
{
    string Function = Pop<string>();
    int32_t unk3 = Pop<int32_t>();
    int32_t unk2 = Pop<int32_t>();
    int32_t unk1 = Pop<int32_t>();
    HandleName = Pop<string>();
    NSBCreateProcess(unk1, unk2, unk3, Function);
}

void NsbInterpreter::WriteFile()
{
    string Data = Pop<string>();
    string FileName = Pop<string>();
    NSBWriteFile(Data, FileName);
}

void NsbInterpreter::ReadFile()
{
    string FileName = Pop<string>();
    NSBReadFile(FileName);
}

void NsbInterpreter::Increment()
{
    Variable* pVar = Stack.top();
    if (int32_t* pInt = boost::get<int32_t>(&pVar->Value))
        *pInt += 1;
}

void NsbInterpreter::Substract()
{
    BinaryOperator([](int32_t a, int32_t b) -> int32_t { return a - b; });
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

void NsbInterpreter::LogicalAnd()
{
    bool Val1 = Pop<bool>();
    bool Val2 = Pop<bool>();
    Push(Val1 && Val2);
}

void NsbInterpreter::LogicalOr()
{
    bool Val1 = Pop<bool>();
    bool Val2 = Pop<bool>();
    Push (Val1 || Val2);
}

void NsbInterpreter::LogicalNotEqual()
{
    LogicalEqual();
    LogicalNot();
}

void NsbInterpreter::LogicalEqual()
{
    Variable* pVar = Stack.top();
    if (boost::get<string>(&pVar->Value))
        LogicalOperator<string>([](const string& a, const string& b) { return a == b; });
    else
        LogicalOperator<int32_t>([](int32_t a, int32_t b) { return a == b; });
}

void NsbInterpreter::LogicalNot()
{
    bool Val = Pop<bool>();
    if (Val) Stack.push(new Variable("false"));
    else Stack.push(new Variable("true"));
}

void NsbInterpreter::LogicalGreaterEqual()
{
    LogicalOperator<int32_t>([](int32_t a, int32_t b) { return a >= b; });
}

void NsbInterpreter::LogicalGreater()
{
    LogicalOperator<int32_t>([](int32_t a, int32_t b) { return a > b; });
}

void NsbInterpreter::LogicalLess()
{
    LogicalOperator<int32_t>([](int32_t a, int32_t b) { return a < b; });
}

void NsbInterpreter::LogicalLessEqual()
{
    LogicalOperator<int32_t>([](int32_t a, int32_t b) { return a <= b; });
}

void NsbInterpreter::If()
{
    if (!Pop<bool>())
        Jump();
}

void NsbInterpreter::RegisterCallback()
{
    string Script = pContext->pLine->Params[1];
    Script.back() = 'b'; // .nss -> .nsb
    pGame->RegisterCallback(static_cast<sf::Keyboard::Key>(pContext->pLine->Params[0][0] - 'A'), Script);
}

void NsbInterpreter::Request()
{
    string State = Pop<string>();
    HandleName = Pop<string>();
    NSBRequest(State);
}

void NsbInterpreter::ParseText()
{
    string XML = Pop<string>();
    string Box = Pop<string>();
    HandleName = Pop<string>();
    pGame->GLCallback(std::bind(&NsbInterpreter::GLParseText, this, Box, XML));
}

void NsbInterpreter::SetLoop()
{
    bool Loop = Pop<bool>();
    HandleName = Pop<string>();
    if (Playable* pMusic = CacheHolder<Playable>::Read(HandleName))
        NSBSetLoop(pMusic, Loop);
}

void NsbInterpreter::Wait()
{
    int32_t Time = Pop<int32_t>();
    Sleep(Time);
}

void NsbInterpreter::Move()
{
    bool Wait = Pop<bool>();
    string Tempo = Pop<string>();
    int32_t Y = Pop<int32_t>();
    int32_t X = Pop<int32_t>();
    int32_t Time = Pop<int32_t>();
    HandleName = Pop<string>();
    if (DrawableBase* pDrawable = CacheHolder<DrawableBase>::Read(HandleName))
        pGame->GLCallback(std::bind(&NsbInterpreter::GLMove, this, pDrawable, Time, X, Y, Tempo, Wait));
}

void NsbInterpreter::WaitText()
{
    string unk = Pop<string>();
    HandleName = Pop<string>();
    if (Text* pText = (Text*)CacheHolder<DrawableBase>::Read(HandleName))
        NSBWaitText(pText, unk);
}

void NsbInterpreter::GetMovieTime()
{
    HandleName = Pop<string>();
    NSBGetMovieTime();
}

void NsbInterpreter::SetParam()
{
    if (pContext->pLine->Params[0] == "STRING")
        Stack.push(new Variable(pContext->pLine->Params[1]));
    else if (pContext->pLine->Params[0] == "INT")
        Stack.push(new Variable(boost::lexical_cast<int32_t>(pContext->pLine->Params[1])));
}

void NsbInterpreter::Get()
{
    Stack.push(Variables[pContext->pLine->Params[0]]);
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

void NsbInterpreter::GetScriptName()
{
    string Name = pContext->pScript->GetName();
    Name = Name.substr(4, Name.size() - 8); // Remove nss/ and .nsb
    Stack.push(new Variable(Name));
}

void NsbInterpreter::System()
{
    string Directory = Pop<string>();
    string Parameters = Pop<string>();
    string Command = Pop<string>();
    NSBSystem(Command, Parameters, Directory);
}

void NsbInterpreter::Negative()
{
    int32_t Val = Pop<int32_t>();
    Stack.push(new Variable(-Val));
    // Negative integers are incorrectly compiled by Nitroplus
    PlaceholderParam();
}

void NsbInterpreter::SetVolume()
{
    string Tempo = Pop<string>();
    int32_t Volume = Pop<int32_t>();
    int32_t NumSeconds = Pop<int32_t>();
    HandleName = Pop<string>();
    if (HandleName.back() == '*' && HandleName.size() > 2)
    {
        WildcardCall<Playable>(HandleName, [this, NumSeconds, Volume, Tempo] (Playable* pMusic)
        {
            NSBSetVolume(pMusic, NumSeconds, Volume, Tempo);
        });
    }
    else if (Playable* pMusic = CacheHolder<Playable>::Read(HandleName))
        NSBSetVolume(pMusic, NumSeconds, Volume, Tempo);
}

void NsbInterpreter::SetLoopPoint()
{
    int32_t End = Pop<int32_t>();
    int32_t Begin = Pop<int32_t>();
    HandleName = Pop<string>();
    if (Playable* pMusic = CacheHolder<Playable>::Read(HandleName))
        NSBSetLoopPoint(pMusic, Begin, End);
}

void NsbInterpreter::CreateSound()
{
    string File = Pop<string>();
    string Type = Pop<string>();
    HandleName = Pop<string>();
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
        pGame->GLCallback(std::bind(&NsbInterpreter::GLApplyBlur, this, pDrawable, Heaviness));
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

// CreateDialog, see: cg/sys/dialog/
void NsbInterpreter::UNK77()
{
}

void NsbInterpreter::PlaceholderParam()
{
    // Parameters with this flag are @ in MAGIC_FUNCTION_CALL argument list
    // This works around the issue: See: NsbInterpreter::GetParam<T>
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
    int32_t Index = pContext->pLine->Params[1]; // TODO: Sometimes a variable, not a literal
    HandleName = pContext->pLine->Params[0];
    NSBArrayRead(Index);
}

void NsbInterpreter::DrawToTexture()
{
    string File = Pop<string>();
    int32_t Y = Pop<int32_t>();
    int32_t X = Pop<int32_t>();
    HandleName = Pop<string>();
    if (sf::RenderTexture* pTexture = CacheHolder<sf::RenderTexture>::Read(HandleName))
        pGame->GLCallback(std::bind(&NsbInterpreter::GLDrawToTexture, this, pTexture, X, Y, File));
}

void NsbInterpreter::CreateRenderTexture()
{
    string Color = Pop<string>();
    int32_t Height = Pop<int32_t>();
    int32_t Width = Pop<int32_t>();
    HandleName = GetParam<string>(0);
    pGame->GLCallback(std::bind(&NsbInterpreter::GLCreateRenderTexture, this, Width, Height, Color));
}

void NsbInterpreter::ClearParams()
{
    Params.clear();
    ArrayParams.clear();
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
    bool Wait = Pop<bool>();
    string File = Pop<string>();
    string Tempo = Pop<string>();
    int32_t Range = Pop<int32_t>();
    int32_t End = Pop<int32_t>();
    int32_t Start = Pop<int32_t>();
    int32_t Time = Pop<int32_t>();
    string HandleName = Pop<string>();
    if (Drawable* pDrawable = (Drawable*)CacheHolder<DrawableBase>::Read(HandleName))
        pGame->GLCallback(std::bind(&NsbInterpreter::GLDrawTransition, this, pDrawable, Time, Start, End, Range, Tempo, File, Wait));
}

void NsbInterpreter::CreateMovie()
{
    bool Audio = Pop<bool>();
    string File = Pop<string>();
    bool Alpha = Pop<bool>();
    bool Loop = Pop<bool>();
    int32_t Y = Pop<int32_t>();
    int32_t X = Pop<int32_t>();
    int32_t Priority = Pop<int32_t>();
    HandleName = Pop<string>();
    pGame->GLCallback(std::bind(&NsbInterpreter::GLCreateMovie, this, Priority, X, Y, Loop, Alpha, File, Audio));
}

void NsbInterpreter::CreateColor()
{
    string Color = Pop<string>();
    int32_t Height = Pop<int32_t>();
    int32_t Width = Pop<int32_t>();
    int32_t Y = Pop<int32_t>();
    int32_t X = Pop<int32_t>();
    int32_t Priority = Pop<int32_t>();
    HandleName = Pop<string>();
    pGame->GLCallback(std::bind(&NsbInterpreter::GLCreateColor, this, Priority, X, Y, Width, Height, Color));
}

void NsbInterpreter::Fade()
{
    bool Wait = Pop<bool>();
    string Tempo = Pop<string>();
    int32_t Opacity = Pop<int32_t>();
    int32_t Time = Pop<int32_t>();
    HandleName = Pop<string>();
    if (HandleName.back() == '*')
    {
        WildcardCall<DrawableBase>(HandleName, [this, Time, Opacity, Tempo, Wait] (DrawableBase* pDrawable)
        {
            NSBFade(pDrawable, Time, Opacity, Tempo, Wait);
        });
    }
    else if (DrawableBase* pDrawable = CacheHolder<DrawableBase>::Read(HandleName))
        NSBFade(pDrawable, Time, Opacity, Tempo, Wait);
}

void NsbInterpreter::End()
{
    pContext->ReturnSubroutine();
}

void NsbInterpreter::CreateTexture()
{
    string File = Pop<string>();
    int32_t Y = Pop<int32_t>();
    int32_t X = Pop<int32_t>();
    int32_t Priority = Pop<int32_t>();

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

    HandleName = Pop<string>();
    pGame->GLCallback(std::bind(&NsbInterpreter::GLCreateTexture, this, Priority, Pos[0], Pos[1], File));
}

void NsbInterpreter::Delete()
{
    HandleName = Pop<string>();
    NSBDelete();
}

void NsbInterpreter::CallFunction()
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
        pGame->GLCallback(std::bind(&Game::RemoveDrawable, pGame, CacheHolder<DrawableBase>::Read("ムービー")));
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

void NsbInterpreter::CallScene()
{
    CallScriptSymbol("scene.");
}

void NsbInterpreter::CallChapter()
{
    CallScriptSymbol("chapter.");
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

void NsbInterpreter::ArraySize()
{
    Push(Arrays[pContext->pLine->Params[0]].Members.size());
}

void NsbInterpreter::Jump()
{
    pContext->pScript->SetSourceIter(pContext->pScript->GetSymbol(pContext->pLine->Params[0]));
}

void NsbInterpreter::SetVertex()
{
    int32_t Y = Pop<int32_t>();
    int32_t X = Pop<int32_t>();
    HandleName = Pop<string>();
    if (Drawable* pDrawable = (Drawable*)CacheHolder<DrawableBase>::Read(HandleName))
        pDrawable->SetCenter(X, Y);
}

void NsbInterpreter::StringToVariable()
{
    // Set
    if (pContext->pLine->Params.size() == 3)
    {
        Variable* pVar = Stack.top();
        Variable* pNew = new Variable;
        pNew->Value = pVar->Value;
        Pop<string>(); // Type doesn't matter, just cleanup propertly
        string Identifier = Pop<string>();
        string Type = Pop<string>();
        SetVariable(Type + Identifier, pNew);
    }
    // Get
    else if (pContext->pLine->Params.size() == 2)
    {
        string Identifier = Pop<string>();
        string Type = Pop<string>(); // "$" or "#"
        Push(Variables[Type + Identifier]);
    }
    else
        assert(false && "This will trigger when we get new season of Haruhi");
}
