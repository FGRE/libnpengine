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
#include "nsbcontext.hpp"
#include "nsbmagic.hpp"

NsbContext::NsbContext(const string& Name) :
Name(Name),
pScript(nullptr),
pLine(nullptr),
SourceIter(0),
Active(false)
{
}

void NsbContext::Run(NsbInterpreter* pInterpreter)
{
    if (!Active)
        return;

    if (SleepClock.getElapsedTime() < SleepTime)
        return;
    else
        SleepTime = sf::Time::Zero;

    do
    {
        if (!pScript || !NextLine())
        {
            pInterpreter->Threads.remove(this);
            ObjectHolder::Write(Name, nullptr);
            delete this;
            return;
        }

        if (pInterpreter->DbgStepping)
        {
            std::cout << GetScriptName() << ":"
                       << SourceIter - 1
                       << " "
                       << pInterpreter->Disassemble(pLine);
        }

        if (pLine->Magic < pInterpreter->Builtins.size())
            if (NsbInterpreter::BuiltinFunc pFunc = pInterpreter->Builtins[pLine->Magic])
                (pInterpreter->*pFunc)();

        // Break before Magic is executed
        pInterpreter->DebuggerTick(pScript->GetLine(SourceIter)->Magic);
        pInterpreter->WaitForResume();
    } while (!pInterpreter->Stack.empty()); // When stack is empty, it is safe to switch context
}

void NsbContext::Jump(const string& Symbol)
{
    uint32_t CodeLine = pScript->GetSymbol(Symbol);
    if (CodeLine != NSB_INVALIDE_LINE)
        SourceIter = CodeLine;
}

void NsbContext::Break()
{
    Jump(ScopeEndLabel);
}

void NsbContext::SetScopeEndLabel(const string& Label)
{
    ScopeEndLabel = Label;
}

bool NsbContext::CallSubroutine(ScriptFile* pDestScript, const string& Symbol)
{
    assert(pDestScript);
    uint32_t CodeLine = pDestScript->GetSymbol(Symbol);
    if (CodeLine != NSB_INVALIDE_LINE)
    {
        // Save return spot
        if (pScript)
            Returns.push({pScript, GetNextLineEntry()});
        pScript = pDestScript;
        SourceIter = CodeLine;
        return true;
    }
    return false;
}

void NsbContext::ReturnSubroutine()
{
    if (!Returns.empty())
    {
        pScript = Returns.top().pScript;
        SourceIter = Returns.top().SourceLine;
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
    pLine = pScript->GetLine(SourceIter++);
    return pLine != nullptr;
}

void NsbContext::WriteTrace(std::ostream& Stream)
{
    if (!pScript)
        return;

    std::stack<FuncReturn> Stack = Returns;
    Stack.push({pScript, SourceIter});
    while (!Stack.empty())
    {
        Stream << Stack.top().pScript->GetName() << " at " << Stack.top().SourceLine << std::endl;
        Stack.pop();
    }
}

void NsbContext::Request(Game* pGame, const string& State)
{
    if (State == "Start")
        Start();
}
