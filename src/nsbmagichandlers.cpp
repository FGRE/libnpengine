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
#include "game.hpp"
#include "drawable.hpp"
#include "playable.hpp"
#include "text.hpp"
#include "nsbcontext.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <boost/format.hpp>
#include <fstream>

void NsbInterpreter::UNK20()
{
    int32_t unk = Pop<int32_t>();
}

void NsbInterpreter::UNK61()
{
    int32_t unk = Pop<int32_t>();
}

void NsbInterpreter::UNK66()
{
    bool unk = Pop<bool>();
}

void NsbInterpreter::UNK101()
{
    string unk = Pop<string>();
}

void NsbInterpreter::UNK103()
{
    int32_t unk = Pop<int32_t>();
}

void NsbInterpreter::UNK104()
{
    int32_t unk = Pop<int32_t>();
}

void NsbInterpreter::UNK115()
{
    string unk0 = Pop<string>();
    string unk1 = Pop<string>();
}

void NsbInterpreter::UNK161()
{
    string unk0 = Pop<string>();
    string unk1 = Pop<string>();
    string unk2 = Pop<string>();
    Push<string>("false");
}

void NsbInterpreter::ClearScore()
{
    string unk = Pop<string>();
}

void NsbInterpreter::SetFrequency()
{
    string unk0 = Pop<string>();
    int32_t unk1 = Pop<int32_t>();
    int32_t unk2 = Pop<int32_t>();
    HandleName = Pop<string>();
}

void NsbInterpreter::SetPan()
{
    string unk0 = Pop<string>();
    int32_t unk1 = Pop<int32_t>();
    int32_t unk2 = Pop<int32_t>();
    HandleName = Pop<string>();
}

void NsbInterpreter::SetNextFocus()
{
    string Direction = Pop<string>();
    string unk1 = Pop<string>();
    string unk2 = Pop<string>();
}

void NsbInterpreter::CreateChoice()
{
    for (int i = 0; i < pContext->GetLineArgs().size() - 1; ++i)
        Pop<int32_t>(); // Unused optional params, always 0
    HandleName = Pop<string>();
}

void NsbInterpreter::SetAlias()
{
    string unk0 = Pop<string>();
    string unk1 = Pop<string>();
}

void NsbInterpreter::SoundAmplitude()
{
    string unk0 = Pop<string>();
    string unk1 = Pop<string>();
    Push(0);
}

void NsbInterpreter::Time()
{
    Stack.push(new Variable(time(0)));
}

void NsbInterpreter::ClearBacklog()
{
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
    PosFunc YFunc = boost::apply_visitor(SpecialPosVisitor(), Stack.top()->Value);
    Pop();
    PosFunc XFunc = boost::apply_visitor(SpecialPosVisitor(), Stack.top()->Value);
    Pop();
    int32_t Priority = Pop<int32_t>();
    HandleName = Pop<string>();
    pGame->GLCallback(std::bind(&NsbInterpreter::GLLoadTextureClip, this, Priority, XFunc, YFunc, Tx, Ty, Width, Height, File));
}

void NsbInterpreter::CreateProcess()
{
    string Function = Pop<string>();
    int32_t unk3 = Pop<int32_t>();
    int32_t unk2 = Pop<int32_t>();
    int32_t unk1 = Pop<int32_t>();
    HandleName = Pop<string>();
    NSBCreateProcess(unk1, unk2, unk3, "function." + Function);
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
    int32_t* pInt = boost::get<int32_t>(&Stack.top()->Value);
    assert(pInt); // Do not increment strings
    *pInt += 1;
    Pop<int32_t>();
}

void NsbInterpreter::Substract()
{
    BinaryOperator<int32_t, int32_t>([](int32_t a, int32_t b) -> int32_t { return a - b; });
}

void NsbInterpreter::Add()
{
    // TODO: STRING + INT = STRING
    if (string* pString = boost::get<string>(&Stack.top()->Value))
        BinaryOperator<string, string>([](const string& a, const string& b) -> string { return a + b; });
    else
        BinaryOperator<int32_t, int32_t>([](int32_t a, int32_t b) -> int32_t { return a + b; });
}

void NsbInterpreter::Divide()
{
    BinaryOperator<int32_t, int32_t>([](int32_t a, int32_t b) -> int32_t
    {
        if (b == 0)
            return 0;
        return a / b;
    });
}

void NsbInterpreter::Multiply()
{
    BinaryOperator<int32_t, int32_t>([](int32_t a, int32_t b) -> int32_t { return a * b; });
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
        BinaryOperator<string, bool>([](const string& a, const string& b) { return a == b; });
    else
        BinaryOperator<int32_t, bool>([](int32_t a, int32_t b) { return a == b; });
}

void NsbInterpreter::LogicalNot()
{
    bool Val = Pop<bool>();
    if (Val) Stack.push(new Variable("false"));
    else Stack.push(new Variable("true"));
}

void NsbInterpreter::LogicalGreaterEqual()
{
    BinaryOperator<int32_t, bool>([](int32_t a, int32_t b) { return a >= b; });
}

void NsbInterpreter::LogicalGreater()
{
    BinaryOperator<int32_t, bool>([](int32_t a, int32_t b) { return a > b; });
}

void NsbInterpreter::LogicalLess()
{
    BinaryOperator<int32_t, bool>([](int32_t a, int32_t b) { return a < b; });
}

void NsbInterpreter::LogicalLessEqual()
{
    BinaryOperator<int32_t, bool>([](int32_t a, int32_t b) { return a <= b; });
}

void NsbInterpreter::If()
{
    if (!Pop<bool>())
        Jump();
}

void NsbInterpreter::Break()
{
    pContext->Break();
}

void NsbInterpreter::RegisterCallback()
{
    string Script = Pop<string>();
    string Key = Pop<string>();
    Script.back() = 'b';
    Callbacks.push_back({static_cast<sf::Keyboard::Key>(Key[0] - 'A'), Script});
}

void NsbInterpreter::Request()
{
    string State = Pop<string>();
    HandleName = Pop<string>();
    NSBRequest(State);
}

void NsbInterpreter::ParseText()
{
    string XML = pContext->GetLineArgs()[2];
    string Box = pContext->GetLineArgs()[1];
    HandleName = pContext->GetLineArgs()[0];
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
    if (pContext->GetLineArgs()[0] == "STRING")
        Stack.push(new Variable(pContext->GetLineArgs()[1]));
    else if (pContext->GetLineArgs()[0] == "INT")
        Stack.push(new Variable(boost::lexical_cast<int32_t>(pContext->GetLineArgs()[1])));
}

void NsbInterpreter::Get()
{
    const string& Identifier = pContext->GetLineArgs()[0];
    auto iter = Variables.find(Identifier);

    // HACK: If variable doesn't exist, set it to itself for CreateArray?
    if (iter == Variables.end())
    {
        ArrayVariable* pVar = new ArrayVariable;
        pVar->Value = Identifier;
        SetVariable(Identifier, pVar);
        iter = Variables.find(Identifier);
    }

    Stack.push(iter->second);
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
    string Name = pContext->GetScriptName();
    Push(Name.substr(4, Name.size() - 8)); // Remove nss/ and .nsb
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
    Push(-Pop<int32_t>());
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
    if (pContext->GetLineArgs().back() == "__array_variable__")
    {
        Variable* pSecond = Stack.top(); Stack.pop();
        Variable* pFirst = Stack.top(); Stack.pop();
        pFirst->Value = pSecond->Value;

        // Cannot change value of literal!
        if (NsbAssert(pFirst->IsPtr, "Changing value of a literal"))
            return;

        // Manual cleanup :(
        if (!pSecond->IsPtr)
            delete pSecond;
    }
    else
    {
        Variable* pVar = Stack.top();
        if (pVar->IsPtr)
        {
            Variable* pNew = new Variable;
            pNew->Value = pVar->Value;
            pVar = pNew;
        }
        SetVariable(pContext->GetLineArgs()[0], pVar);
        Stack.pop();
    }
}

void NsbInterpreter::ArrayRead()
{
    int32_t Index = GetVariable<int32_t>(pContext->GetLineArgs()[1]);
    HandleName = pContext->GetLineArgs()[0];
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
    HandleName = Pop<string>();
    pGame->GLCallback(std::bind(&NsbInterpreter::GLCreateRenderTexture, this, Width, Height, Color));
}

void NsbInterpreter::ClearParams()
{
    NsbAssert(Stack.empty(), "Not all parameters were used!");
    while (!Stack.empty()) Pop<string>();
}

void NsbInterpreter::Begin()
{
    // Turn params into global variables
    // TODO: Should scope be respected instead?
    for (int i = pContext->GetLineArgs().size() - 1; i > 0; --i)
    {
        Variable* pVar = Stack.top();
        SetVariable(pContext->GetLineArgs()[i], pVar);
        Stack.pop();
    }
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
    PosFunc YFunc = boost::apply_visitor(SpecialPosVisitor(), Stack.top()->Value);
    Pop();
    PosFunc XFunc = boost::apply_visitor(SpecialPosVisitor(), Stack.top()->Value);
    Pop();
    int32_t Priority = Pop<int32_t>();
    HandleName = Pop<string>();
    pGame->GLCallback(std::bind(&NsbInterpreter::GLCreateTexture, this, Priority, XFunc, YFunc, File));
}

void NsbInterpreter::Delete()
{
    HandleName = Pop<string>();
    NSBDelete();
}

void NsbInterpreter::CallFunction()
{
    string FuncName = pContext->GetLineArgs()[0];
    string FuncNameFull = string("function.") + FuncName;

    // Find function override (i.e. a hack)
    if (FuncName == "DeleteAllSt")
    {
        ClearParams();
        HandleName = "StNameSTBUF1/STBUF100";
        NSBDelete();
        return;
    }

    // Find function locally
    if (pContext->CallSubroutine(pContext->GetScript(), FuncNameFull))
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
    size_t Index = Stack.size() - pContext->GetLineArgs().size();
    boost::format Fmt(boost::get<string>(Stack[Index]->Value));
    for (size_t i = Index + 1; i < Stack.size(); ++i)
    {
        if (string* pString = boost::get<string>(&Stack[i]->Value))
            Fmt % *pString;
        else if (int32_t* pInt = boost::get<int32_t>(&Stack[i]->Value))
            Fmt % *pInt;
        else assert(false);
    }

    // Cleanup
    for (size_t i = Index; i < Stack.size(); ++i)
        Pop();
}

void NsbInterpreter::ArraySize()
{
    auto iter = Arrays.find(Pop<string>());
    if (NsbAssert(iter != Arrays.end(), "Trying to get size of non-existing array"))
        Push(0); // TODO: HACK!
    else
        Push(iter->second->Members.size());
}

void NsbInterpreter::Jump()
{
    pContext->Jump(pContext->GetLineArgs()[0]);
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
    if (pContext->GetLineArgs().size() == 3)
    {
        Variable* pVar = Stack.top();
        Variable* pNew = new Variable;
        pNew->Value = pVar->Value;
        Pop();
        string Identifier = Pop<string>();
        string Type = Pop<string>();
        SetVariable(Type + Identifier, pNew);
    }
    // Get
    else if (pContext->GetLineArgs().size() == 2)
    {
        string Identifier = Pop<string>();
        string Type = Pop<string>(); // "$" or "#"
        if (Variables.find(Type + Identifier) != Variables.end())
            Stack.push(Variables[Type + Identifier]);
        else Stack.push(new Variable); // TODO: Hack!
    }
    else
        assert(false && "This will trigger when we get new season of Haruhi");
}

void NsbInterpreter::CreateArray()
{
    size_t Index = Stack.size() - pContext->GetLineArgs().size();
    ArrayVariable* pArr = dynamic_cast<ArrayVariable*>(Stack[Index]);
    const string& Identifier = pContext->GetLineArgs()[0];

    assert(pArr);

    // Map root
    if (Arrays.find(Identifier) == Arrays.end())
        Arrays[Identifier] = pArr;

    for (uint32_t i = 1; i < pContext->GetLineArgs().size(); ++i)
    {
        Variable* pVar = Stack.top();
        ArrayVariable* pNew = new ArrayVariable;
        pNew->Value = pVar->Value;
        pNew->IsPtr = true;
        Pop();
        pArr->Members.push_front(std::make_pair(string(), pNew)); // Params are in reverse order, so push_front
    }

    Stack.pop(); // Identifier
}

void NsbInterpreter::BindIdentifier()
{
    size_t Index = Stack.size() - pContext->GetLineArgs().size();
    ArrayVariable* pArr = dynamic_cast<ArrayVariable*>(Stack[Index]);

    if (!pArr)
    {
        const string& Identifier = pContext->GetLineArgs()[0];

        // Check if tree exists
        auto iter = Arrays.find(Identifier);
        if (NsbAssert(iter != Arrays.end(), "Cannot bind identifiers to tree because it doesn't exist"))
            return;

        // Check if identifiers are already bound
        if (NsbAssert(Arrays[Identifier]->Members.begin()->first.empty(),
            "Cannot bind identifiers to tree because they are already bound"))
            return;

        // Bind to first level of tree
        pArr = Arrays[Identifier];
    }

    // Parameters are in reverse order, so start from rbegin
    for (auto iter = pArr->Members.rbegin(); iter != pArr->Members.rend(); ++iter)
        iter->first = Pop<string>();

    Stack.pop(); // Identifier
}
