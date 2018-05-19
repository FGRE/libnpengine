/*
 * libnpengine: Nitroplus script interpreter
 * Copyright (C) 2014-2016,2018 Mislav Blažević <krofnica996@gmail.com>
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
}

void Variable::Set(int32_t Int)
{
    Val.Int = Int;
    Tag = NSB_INT;
}

void Variable::Set(const string& Str)
{
    Val.Str = Str;
    // hack
    if (Str[0] == '@')
    {
        Relative = true;
        try
        {
            stoi(Val.Str.c_str() + 1);
        }
        catch(...)
        {
            Relative = false;
        }
    }
    Tag = NSB_STRING;
}

void Variable::Initialize()
{
    Tag = NSB_NULL;
}

void Variable::Initialize(Variable* pVar)
{
    if (pVar->Tag == NSB_NULL)
        Initialize();
    else if (pVar->IsString())
        Set(pVar->ToString());
    else
        Set(pVar->ToInt());
}

Variable* Variable::MakeInt(int32_t Int)
{
    Variable* pVar = new Variable;
    pVar->Set(Int);
    return pVar;
}

Variable* Variable::MakeString(const string& Str)
{
    Variable* pVar = new Variable;
    pVar->Set(Str);
    return pVar;
}

Variable* Variable::MakeNull(const string& Name)
{
    Variable* pVar = new Variable;
    pVar->Literal = false;
    pVar->Name = Name;
    pVar->Initialize();
    return pVar;
}

Variable* Variable::MakeCopy(Variable* pVar, const string& Name)
{
    Variable* pNew = new Variable;
    pNew->Literal = false;
    pNew->Name = Name;
    pNew->Initialize(pVar);
    Variable::Destroy(pVar);
    return pNew;
}

int32_t Variable::ToInt()
{
    if (Tag == NSB_NULL) return 0;
    if (Tag == NSB_STRING && Relative) return stoi(Val.Str.c_str() + 1);
    assert(IsInt());
    return Val.Int;
}

string Variable::ToString()
{
    if (Tag == NSB_NULL)
        return "";

    if (Tag == NSB_INT)
        return Relative ? string("@") + to_string(Val.Int) : to_string(Val.Int);

    return Val.Str;
}

bool Variable::IsInt()
{
    return (Tag == NSB_INT || Tag == NSB_NULL) || Relative;
}

bool Variable::IsString()
{
    return (Tag == NSB_STRING || Tag == NSB_NULL) || Relative;
}

bool Variable::IsNull()
{
    return Tag == NSB_NULL;
}

void Variable::Set(Variable* pVar)
{
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
