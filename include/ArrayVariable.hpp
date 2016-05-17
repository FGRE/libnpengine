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
#ifndef ARRAY_VARIABLE_HPP
#define ARRAY_VARIABLE_HPP

#include "Variable.hpp"
#include <list>
#include <utility>

class ArrayVariable : public Variable
{
    ArrayVariable();
public:
    ~ArrayVariable();

    ArrayVariable* Find(const string& Key);
    ArrayVariable* Find(int32_t Index);
    void Push(ArrayVariable* pVar);

    static ArrayVariable* MakeNull();
    static ArrayVariable* MakeCopy(Variable* pVar);

    list<pair<string, ArrayVariable*>> Members;
};

#endif
