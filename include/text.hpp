/* 
 * libnpengine: Nitroplus script interpreter
 * Copyright (C) 2013 Mislav Blažević <krofnica996@gmail.com>
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

#include <SFML/Graphics/Text.hpp>

namespace sf
{
    class Music;
};

struct Text : sf::Text
{
    Text(const std::string& XML);
    ~Text();

    bool NextLine();
    std::vector<sf::Music*> Voices;
    std::vector<std::string> Lines;
    size_t LineIter;
};

#endif
