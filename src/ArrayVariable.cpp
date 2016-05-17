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
#include "ArrayVariable.hpp"

ArrayVariable::ArrayVariable()
{
    Literal = false;
    Initialize(0);
}

ArrayVariable::~ArrayVariable()
{
    for (auto i = Members.begin(); i != Members.end(); ++i)
        delete i->second;
}

ArrayVariable* ArrayVariable::Find(const string& Key)
{
    for (auto i = Members.begin(); i != Members.end(); ++i)
        if (i->first == Key)
            return i->second;
    return nullptr;
}

ArrayVariable* ArrayVariable::Find(int32_t Index)
{
    while (Index >= Members.size())
        Push(MakeNull());

    auto i = Members.begin();
    advance(i, Index);
    return i->second;
}

void ArrayVariable::Push(ArrayVariable* pVar)
{
    Members.push_back(make_pair(string(), pVar));
}

ArrayVariable* ArrayVariable::MakeNull()
{
    ArrayVariable* pVar = new ArrayVariable;
    pVar->Initialize();
    return pVar;
}

ArrayVariable* ArrayVariable::MakeCopy(Variable* pVar)
{
    ArrayVariable* pNew = new ArrayVariable;
    pNew->Initialize(pVar);
    Variable::Destroy(pVar);
    return pNew;
}
