/* 
 * libnpengine: Nitroplus script interpreter
 * Copyright (C) 2014-2015 Mislav Blažević <krofnica996@gmail.com>
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
#include "nsbconstants.hpp"
#include <cassert>

Variable::Variable() : Literal(true), Relative(false)
{
}

Variable::~Variable()
{
    Destroy();
}

void Variable::Initialize(int32_t Int)
{
    Val.Int = Int;
    Tag = NSB_INT;
}

void Variable::Initialize(const string& Str)
{
    Val.Str = new string(Str);
    Tag = NSB_STRING;
}

void Variable::Initialize()
{
    Val.Str = nullptr;
    Tag = NSB_NULL;
}

void Variable::Initialize(Variable* pVar)
{
    if (pVar->Tag == NSB_NULL)
        Initialize();
    else if (pVar->IsString())
        Initialize(pVar->ToString());
    else
        Initialize(pVar->ToInt());
}

void Variable::Destroy()
{
    if (Tag == NSB_STRING)
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
    if (Str[0] == '@') pVar->Relative = true;
    pVar->Initialize(Str);
    return pVar;
}

Variable* Variable::MakeNull()
{
    Variable* pVar = new Variable;
    pVar->Initialize();
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
    if (Tag == NSB_NULL) return 0;
    if (Tag == NSB_STRING && Relative) return stoi(Val.Str->c_str() + 1);
    assert(IsInt());
    return Val.Int;
}

string Variable::ToString()
{
    if (Tag == NSB_NULL) return "";
    if (Tag == NSB_INT && Relative) return string("@") + to_string(Val.Int);
    assert(IsString());
    return *Val.Str;
}

bool Variable::IsInt()
{
    return (Tag & NSB_INT) || Relative;
}

bool Variable::IsString()
{
    return (Tag & NSB_STRING) || Relative;
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
        pThird = MakeInt(pFirst->ToInt() + pSecond->ToInt());
    else if (pFirst->IsString())
        pThird = MakeString(pFirst->ToString() + (pSecond->IsString() ? pSecond->ToString() : to_string(pSecond->ToInt())));
    else assert(false);
    Destroy(pFirst);
    Destroy(pSecond);
    return pThird;
}

Variable* Variable::Equal(Variable* pFirst, Variable* pSecond)
{
    Variable* pThird = nullptr;
    if (pFirst->IsInt() && pFirst->Tag != NSB_NULL)
        pThird = MakeInt(pFirst->ToInt() == (pSecond->IsInt() ? pSecond->ToInt() : Nsb::ConstantToValue<Nsb::Boolean>(pSecond->ToString())));
    else if (pSecond->IsInt() && pSecond->Tag != NSB_NULL)
        pThird = MakeInt(pSecond->ToInt() == (pFirst->IsInt() ? pFirst->ToInt() : Nsb::ConstantToValue<Nsb::Boolean>(pFirst->ToString())));
    else
        pThird = MakeInt(pFirst->ToString() == pSecond->ToString());
    Destroy(pFirst);
    Destroy(pSecond);
    return pThird;
}

void Variable::Destroy(Variable* pVar)
{
    if (pVar->Literal)
        delete pVar;
}
