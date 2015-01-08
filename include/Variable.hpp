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
#ifndef VARIABLE_HPP
#define VARIABLE_HPP

#include <cstdint>
#include <string>
#include <functional>
using namespace std;

class Variable
{
protected:
    enum
    {
        NSB_INT = 1,
        NSB_STRING = 2,
        NSB_NULL = NSB_INT | NSB_STRING
    } Tag;
    union
    {
        int32_t Int;
        string* Str;
    } Val;

    Variable();

    void Initialize(int32_t Int);
    void Initialize(const string& Str);
    void Initialize();
    void Initialize(Variable* pVar);
    void Destroy();

public:
    virtual ~Variable();

    static Variable* MakeNull();
    static Variable* MakeInt(int32_t Int);
    static Variable* MakeString(const string& Str);
    static Variable* MakeCopy(Variable* pVar);

    int32_t ToInt();
    string ToString();
    bool IsInt();
    bool IsString();
    void Set(Variable* pVar);
    Variable* IntUnaryOp(function<int32_t(int32_t)> Func);

    static Variable* Add(Variable* pFirst, Variable* pSecond);
    static Variable* Equal(Variable* pFirst, Variable* pSecond);
    static void Destroy(Variable* pVar);

    bool Literal;
    bool Relative;
};

#endif
