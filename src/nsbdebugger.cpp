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

void NsbInterpreter::DebuggerMain()
{
    while (!StopInterpreter)
    {
        string Command;
        std::cin >> Command;
        if (Command == "s")
            RunInterpreter = true;
        else if (Command == "c")
            DbgStepping = false;
    }
}
