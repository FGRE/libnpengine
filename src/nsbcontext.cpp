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

bool NsbContext::CallSubroutine(ScriptFile* pDestScript, string Symbol)
{
    if (!pDestScript)
        return false;

    uint32_t CodeLine = pDestScript->GetSymbol(Symbol);
    if (CodeLine != NSB_INVALIDE_LINE)
    {
        if (pScript)
            Returns.push({pScript, pScript->GetNextLineEntry()});
        pScript = pDestScript;
        pScript->SetSourceIter(CodeLine);
        return true;
    }
    return false;
}

void NsbContext::ReturnSubroutine()
{
    if (!Returns.empty())
    {
        pScript = Returns.top().pScript;
        pScript->SetSourceIter(Returns.top().SourceLine);
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
    pLine = pScript->GetNextLine();
    return pLine != nullptr;
}
