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
#ifndef TEXT_HPP
#define TEXT_HPP

#include "Texture.hpp"
#include "TextParser.hpp"

class Playable;
class Text : public Texture, private TextParser::Text
{
public:
    Text();
    ~Text();

    void CreateFromXML(const string& XML);
    void CreateFromString(const string& String);

    void SetCharacterSize(uint32_t Size);
    void SetColor(uint32_t Color);
    void SetWrap(int32_t Width);
    bool Advance();

    void Request(int32_t State);

    static string dFont;
    static int32_t dSize;
    static uint32_t dInColor;
    static uint32_t dOutColor;
    static int32_t dWeight;
    static string dAlign;
private:
    void SetString(const string& String);

    size_t Index, LayoutWidth;
    uint32_t Size, Color;
};


#endif
