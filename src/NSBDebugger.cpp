/* 
 * libnpengine: Nitroplus script interpreter
 * Copyright (C) 2014-2016 Mislav Blažević <krofnica996@gmail.com>
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
#include "Window.hpp"
#include "nsbmagic.hpp"
#include "scriptfile.hpp"
#include <boost/algorithm/string.hpp>

string NSBInterpreter::Disassemble(Line* pLine)
{
    string String = Nsb::StringifyMagic(pLine->Magic);
    String += "(";
    for (unsigned i = 0; i < pLine->Params.size(); ++i)
    {
        String += pLine->Params[i];
        String += ((i == pLine->Params.size() - 1) ? "" : ", ");
    }
    String += ");\n";
    return String;
}

void NSBInterpreter::StartDebugger()
{
    if (!pDebuggerThread)
        pDebuggerThread = new thread(bind(&NSBInterpreter::DebuggerMain, this));
    DbgBreak(true);
}

void NSBInterpreter::SetBreakpoint(const string& Script, int32_t LineNumber)
{
    if (ScriptFile* pScript = sResourceMgr->GetScriptFile(Script))
    {
        if (pScript->GetLine(LineNumber))
            Breakpoints.push_back(make_pair(Script, LineNumber));
    }
    else
        cout << "Cannot set breakpoint " << Script << ":" << LineNumber << endl;
}

void NSBInterpreter::PrintVariable(Variable* pVar, const string& Identifier)
{
    cout << Identifier << " = ";
    if (pVar->IsInt())
        cout << pVar->ToInt() << endl;
    else if (pVar->IsString())
        cout << pVar->ToString() << endl;
}

void NSBInterpreter::PrintArray(ArrayVariable* pArray, const string& Identifier, int Depth)
{
    PrintVariable(pArray, Identifier);
    for (auto i : pArray->Members)
    {
        cout << string(Depth * 2, ' ');
        PrintArray(i.second, i.first, Depth + 1);
    }
}

void NSBInterpreter::DebuggerTick()
{
    if (DbgStepping || LogCalls)
        cout << pContext->GetScriptName() << ":"
             << pContext->GetLineNumber() << " "
             << Disassemble(pContext->GetLine());

    if (DbgStepping)
    {
        RunInterpreter = false;
        return;
    }

    // Breakpoint
    for (auto i : Breakpoints)
    {
        if (i.first == pContext->GetScriptName() &&
            i.second == pContext->GetLineNumber())
        {
            DbgBreak(true);
            return;
        }
    }
}

void NSBInterpreter::DbgBreak(bool Break)
{
    if (Break) cout << "Breakpoint hit!" << endl;
    DbgStepping = Break;
    RunInterpreter = !Break;
}

void NSBInterpreter::Inspect(int32_t n)
{
    for (auto i : Threads)
    {
        cout << "\nThread " << i->GetName() << ":\n";
        ScriptFile* pScript = i->GetScript();
        uint32_t SourceIter = i->GetLineNumber();
        for (uint32_t i = SourceIter - n; i < SourceIter + n + 1; ++i)
            cout << ((i == SourceIter) ? " > " : "   ") << Disassemble(pScript->GetLine(i));
    }
}

void NSBInterpreter::DebuggerMain()
{
    string OldCommand;
    while (pWindow->IsRunning_())
    {
        string Command;
        getline(cin, Command);
        if (Command.empty())
            Command = OldCommand;
        else OldCommand = Command;

        // Step
        if (Command == "s")
            RunInterpreter = true;
        // Continue
        else if (Command == "c")
            DbgBreak(false);
        // Quit
        else if (Command == "q")
            Exit();
        // Wait (Activate debugger)
        else if (Command == "w")
            DbgBreak(true);
        // Log
        else if (Command == "l")
            LogCalls = !LogCalls;
        // Thread Trace
        else if (Command == "t")
        {
            for (auto i : Threads)
            {
                cout << "\nThread " << i->GetName() << ":\n";
                i->WriteTrace(cout);
            }
        }
        else
        {
            vector<string> Tokens;
            boost::split(Tokens, Command, boost::is_any_of(" :"));

            // Breakpoint
            if (Tokens.size() == 3 && Tokens[0] == "b")
            {
                try
                {
                    SetBreakpoint(Tokens[1], stoi(Tokens[2]));
                } catch (...) { cout << "Bad command!" << endl; }
            }
            // Breakpoint Clear
            else if (Tokens.size() == 2 && Tokens[0] == "b" && Tokens[1] == "c")
            {
                Breakpoints.clear();
            }
            // Print
            else if (Tokens.size() == 2 && Tokens[0] == "p")
            {
                if (Variable* pVar = GetVar(Tokens[1]))
                    PrintVariable(pVar, Tokens[1]);
                else cout << "Variable " << Tokens[1] << " not found!" << endl;
            }
            // Print Array
            else if (Tokens.size() == 3 && Tokens[0] == "p" && Tokens[1] == "a")
            {
                if (ArrayVariable* pVar = GetArr(Tokens[2]))
                    PrintArray(pVar, Tokens[2]);
                else cout << "Array " << Tokens[2] << " not found!" << endl;
            }
            // Inspect surrounding code
            else if (Tokens.size() == 2 && Tokens[0] == "i")
            {
                try
                {
                    Inspect(stoi(Tokens[1]));
                } catch (...) { cout << "Bad command!" << endl; }
            }
            // Dump Variables
            else if (Tokens.size() == 2 && Tokens[0] == "d" && Tokens[1] == "v")
            {
                for (auto i : VariableHolder.Cache)
                    PrintVariable(i.second, i.first);
            }
            else
                cout << "Bad command!" << endl;
        }
    }
}
