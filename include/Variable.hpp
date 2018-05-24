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
#ifndef VARIABLE_HPP
#define VARIABLE_HPP

#include <cstdint>
#include <string>
#include <functional>
#include <map>
using namespace std;

class Variable
{
protected:
    enum
    {
        NSB_NULL = 0,
        NSB_INT = 1,
        NSB_FLOAT = 2,
        NSB_STRING = 3,
        NSB_BOOL = 4
    } Tag;

    struct
    {
        int32_t Int;
        float Float;
        string Str;
        bool Bool;
    } Val;

    Variable();

    void Initialize();
    void Initialize(Variable* pVar);

public:
    virtual ~Variable();

    static Variable* MakeNull(const string& Name);
    static Variable* MakeFloat(float Float);
    static Variable* MakeInt(int32_t Int);
    static Variable* MakeString(const string& Str);
    static Variable* MakeCopy(Variable* pVar, const string& Name);

    int GetTag();
    float ToFloat();
    int32_t ToInt();
    string ToString();
    bool IsFloat();
    bool IsInt();
    bool IsString();
    bool IsNull();
    void Set(Variable* pVar);
    void Set(float Float);
    void Set(int32_t Int);
    void Set(const string& Str);
    Variable* IntUnaryOp(function<int32_t(int32_t)> Func);

    static Variable* Add(Variable* pFirst, Variable* pSecond);
    static void Destroy(Variable* pVar);

    bool Literal;
    bool Relative;
    map<string, int> Assoc;
    string Name;
};

#endif
