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
#include "Variable.hpp"
#include <cassert>

Variable::Variable() : Literal(true)
{
}

Variable::~Variable()
{
    Destroy();
}

void Variable::Initialize(int32_t Int)
{
    Val.Int = Int;
    Tag = INT;
}

void Variable::Initialize(const string& Str)
{
    Val.Str = new string(Str);
    Tag = STRING;
}

void Variable::Initialize(Variable* pVar)
{
    if (pVar->IsString())
        Initialize(pVar->ToString());
    else
        Initialize(pVar->ToInt());
}

void Variable::Destroy()
{
    if (IsString())
        delete Val.Str;
}

Variable* Variable::MakeInt(int32_t Int)
{
    Variable* pVar = new Variable;
    pVar->Initialize(Int);
    return pVar;
}

Variable* Variable::MakeString(const string& Str)
{
    Variable* pVar = new Variable;
    pVar->Initialize(Str);
    return pVar;
}

Variable* Variable::MakeCopy(Variable* pVar)
{
    Variable* pNew = new Variable;
    pNew->Initialize(pVar);
    return pNew;
}

int32_t Variable::ToInt()
{
    assert(IsInt());
    return Val.Int;
}

string Variable::ToString()
{
    assert(IsString());
    return *Val.Str;
}

bool Variable::IsInt()
{
    return Tag == INT;
}

bool Variable::IsString()
{
    return Tag == STRING;
}

void Variable::Set(Variable* pVar)
{
    Destroy();
    Initialize(pVar);
}

Variable* Variable::IntUnaryOp(function<int32_t(int32_t)> Func)
{
    Val.Int = Func(Val.Int);
    return this;
}

Variable* Variable::Add(Variable* pFirst, Variable* pSecond)
{
    Variable* pThird = nullptr;
    if (pFirst->IsInt() && pSecond->IsInt())
        pThird = MakeInt(pFirst->Val.Int + pSecond->Val.Int);
    else if (pFirst->IsString() && pSecond->IsString())
        pThird = MakeString(*pFirst->Val.Str + *pSecond->Val.Str);
    Destroy(pFirst);
    Destroy(pSecond);
    return pThird;
}

Variable* Variable::Equal(Variable* pFirst, Variable* pSecond)
{
    Variable* pThird = nullptr;
    if (pFirst->IsInt() && pSecond->IsInt())
        pThird = MakeInt(pFirst->Val.Int == pSecond->Val.Int);
    else if (pFirst->Tag == STRING && pSecond->Tag == STRING)
        pThird = MakeInt(*pFirst->Val.Str == *pSecond->Val.Str);
    Destroy(pFirst);
    Destroy(pSecond);
    return pThird;
}

void Variable::Destroy(Variable* pVar)
{
    if (pVar->Literal)
        delete pVar;
}
