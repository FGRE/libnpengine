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
#ifndef TEXT_HPP
#define TEXT_HPP

#include "Texture.hpp"
#include "TextParser.hpp"

class Playable;
class Text : public Texture, private TextParser::Text
{
public:
    Text(const string& XML);
    ~Text();

    void SetWrap(int32_t Width);
    bool Advance();

private:
    void SetString(const string& String);

    size_t Index, LayoutWidth;
    Playable* pVoice;
};


#endif
