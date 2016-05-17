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
#ifndef TEXT_PARSER_HPP
#define TEXT_PARSER_HPP

#include <vector>
#include <string>
using namespace std;

namespace TextParser
{

class Line;
class StringSegment;
typedef vector<Line> LinesList;
typedef vector<StringSegment> String;
typedef vector<string> ArgumentList;
typedef ArgumentList Voice;

enum VoiceAttr
{
    ATTR_NAME,
    ATTR_CLASS,
    ATTR_SRC,
    ATTR_MODE
};

struct StringSegment
{
    string Segment, Ruby;
    string InColor, OutColor;
};

struct Line
{
    String StringSegs;
    Voice VoiceAttrs;
};

struct Text
{
    LinesList Lines;
};

}

#endif
