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
    pGame->GLCallback(std::bind(&NsbInterpreter::GLLoadTextureClip, this, X, Y, Tx, Ty, Width, Height, File));
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
    if (int32_t* pInt = boost::get<int32_t>(pVar->Value))
        *pInt += 1;
}

void NsbInterpreter::LogicalAnd()
{
    // TODO
}

void NsbInterpreter::LogicalOr()
{
    // TODO
}

void NsbInterpreter::LogicalNotEqual()
{
    LogicalEqual();
    LogicalNot();
}

void NsbInterpreter::LogicalEqual()
{
    Variable* pVar = Stack.top();
    if (boost::get<string>(pVar->Value))
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
    pGame->GLCallback(std::bind(&NsbInterpreter::GLParseText, this, Box, XML);
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
    if (strcmp(pContext->pLine->Params[0], "STRING") == 0)
        Stack.push(new Variable(pContext->pLine->Params[1]));
    else if (strcmp(pContext->pLine->Params[0], "INT") == 0)
        Stack.push(new Variable(boost::lexical_cast<int32_t>(pContext->pLine->Params[1])));
}

void NsbInterpreter::Get()
{
    Stack.push(Variables[pContext->pLine->Params[0]], true);
}
