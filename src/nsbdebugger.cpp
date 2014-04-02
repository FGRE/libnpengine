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
#include "nsbmagic.hpp"

#include <iostream>
#include <thread>
#include <boost/algorithm/string.hpp>

string NsbInterpreter::Disassemble(Line* pLine)
{
    string String = Nsb::StringifyMagic(pLine->Magic);
    String += "(";
    for (int i = 0; i < pLine->Params.size(); ++i)
    {
        String += pLine->Params[i];
        String += ((i != (pLine->Params.size() - 1)) ? ", " : "");
    }
    String += ");\n";
    return String;
}

void NsbInterpreter::StartDebugger()
{
    if (!pDebuggerThread)
        pDebuggerThread = new std::thread(std::bind(&NsbInterpreter::DebuggerMain, this));
    DbgStepping = true;
}

void NsbInterpreter::SetBreakpoint(const string& Script, int32_t LineNumber)
{
    if (sResourceMgr->GetScriptFile(Script))
        Breakpoints.push_back(std::make_pair(Script, LineNumber));
    else
        std::cout << "Cannot set breakpoint " << Script << ":" << LineNumber << std::endl;
}

bool NsbInterpreter::DebuggerTick()
{
    bool DoPause = false;

    if (!DbgStepping)
    {
        for (auto iter = Breakpoints.begin(); iter != Breakpoints.end(); ++iter)
        {
            if (iter->first == pContext->pScript->GetName() &&
                iter->second == pContext->pScript->GetNextLineEntry())
            {
                std::cout << "Breakpoint hit!" << std::endl;
                DbgStepping = true;
                DoPause = true;
                break;
            }
        }
    }
    else
        DoPause = true;

    if (DoPause)
        Pause();
    return DoPause;
}

void NsbInterpreter::DebuggerMain()
{
    string OldCommand;
    while (!StopInterpreter)
    {
        string Command;
        std::getline(std::cin, Command);
        if (Command.empty())
            Command = OldCommand;
        else OldCommand = Command;

        // Step
        if (Command == "s")
            RunInterpreter = true;
        // Continue
        else if (Command == "c")
            DbgStepping = false;
        else
        {
            std::vector<string> Tokens;
            boost::split(Tokens, Command, boost::is_any_of(" :"));
            // Breakpoint
            if (Tokens.size() == 3 && Tokens[0] == "b")
            {
                try
                {
                    SetBreakpoint(Tokens[1], boost::lexical_cast<int32_t>(Tokens[2]));
                } catch (...) { std::cout << "Bad command!" << std::endl; }
            }
        }
    }
}
