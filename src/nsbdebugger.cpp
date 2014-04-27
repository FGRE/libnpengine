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
#include "nsbcontext.hpp"

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

void NsbInterpreter::PrintVariable(Variable* pVar)
{
    if (string* pString = boost::get<string>(&pVar->Value))
        std::cout << *pString << std::endl;
    else if (int32_t* pInt = boost::get<int32_t>(&pVar->Value))
        std::cout << *pInt << std::endl;
    else assert(false);
}

void NsbInterpreter::DebuggerTick(uint16_t Magic)
{
    if (!DbgStepping)
    {
        // Breakpoint on builtin
        if (MagicBreakpoints[Magic])
        {
            DbgBreak();
            return;
        }
        // Breakpoint on specific line
        for (auto iter = Breakpoints.begin(); iter != Breakpoints.end(); ++iter)
        {
            if (iter->first == pContext->GetScriptName() &&
                iter->second == pContext->GetNextLineEntry())
            {
                DbgBreak();
                return;
            }
        }
    }
    else
        Pause();
}

void NsbInterpreter::DbgBreak()
{
    std::cout << "Breakpoint hit!" << std::endl;
    DbgStepping = true;
    Pause();
}

void NsbInterpreter::Inspect(int32_t n)
{
    ScriptFile* pScript = pContext->GetScript();
    uint32_t SourceIter = pContext->GetNextLineEntry();
    for (uint32_t i = SourceIter - n - 1; i < SourceIter + n; ++i)
    {
        if (i == SourceIter - 1) std::cout << " > ";
        else std::cout << "   ";
        std::cout << Disassemble(pScript->GetLine(i));
    }
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
        // Quit
        else if (Command == "q")
            Stop();
        // Wait (Activate debugger)
        else if (Command == "w")
            DbgStepping = true;
        // Dump (Param stack)
        else if (Command == "d")
        {
            for (size_t i = 0; i < Stack.size(); ++i)
            {
                std::cout << "Stack[" << i << "] = ";
                PrintVariable(Stack[i]);
            }
        }
        // Thread Trace
        else if (Command == "t")
        {
            for (auto iter = Threads.begin(); iter != Threads.end(); ++iter)
            {
                std::cout << "\nThread " << (*iter)->GetName() << ":\n";
                (*iter)->WriteTrace(std::cout);
            }
        }
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
            // Breakpoint on builtin call
            else if (Tokens.size() == 2 && Tokens[0] == "b")
            {
                if (uint16_t Magic = Nsb::MagicifyString(Tokens[1].c_str()))
                    MagicBreakpoints[Magic] = true;
            }
            // Breakpoint Clear
            else if (Tokens.size() == 2 && Tokens[0] == "b" && Tokens[1] == "c")
            {
                Breakpoints.clear();
                BreakOnAssert = false;
            }
            // Breakpoint on Assertion failure
            else if (Tokens.size() == 2 && Tokens[0] == "b" && Tokens[1] == "a")
            {
                BreakOnAssert = true;
            }
            // Jump
            else if (Tokens.size() == 2 && Tokens[0] == "j")
            {
                try
                {
                    pMainContext->Jump(boost::lexical_cast<int32_t>(Tokens[1]));
                } catch (...) { std::cout << "Bad command!" << std::endl; }
            }
            // Print
            else if (Tokens.size() == 2 && Tokens[0] == "p")
            {
                auto iter = Variables.find(Tokens[1]);
                if (iter != Variables.end())
                {
                    std::cout << Tokens[1] << " = ";
                    PrintVariable(iter->second);
                }
                else std::cout << "Variable " << Tokens[1] << " not found!" << std::endl;
            }
            // Inspect surrounding code
            else if (Tokens.size() == 2 && Tokens[0] == "i")
            {
                try
                {
                    Inspect(boost::lexical_cast<int32_t>(Tokens[1]));
                } catch (...) { std::cout << "Bad command!" << std::endl; }
            }
            // Dump Variables
            else if (Tokens.size() == 2 && Tokens[0] == "d" && Tokens[1] == "v")
            {
                for (auto iter = Variables.begin(); iter != Variables.end(); ++iter)
                {
                    std::cout << iter->first << " = ";
                    PrintVariable(iter->second);
                }
            }
        }
    }
}
