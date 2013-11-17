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
#ifndef DRAWABLE_HPP
#define DRAWABLE_HPP

#include <cstdint>
#include <SFML/System/Clock.hpp>

enum
{
    DRAWABLE_MOVIE      = 0,
    DRAWABLE_TEXTURE    = 1
};

namespace sf
{
    class Drawable;
};

class Drawable
{
public:
    Drawable(sf::Drawable* pDrawable, int32_t Priority, uint8_t Type);
    ~Drawable();

    void Update();
    void Fade(int32_t Opacity, int32_t Time);
    int32_t GetPriority() const;
    sf::Drawable* Get() const;

private:
    sf::Drawable* pDrawable;
    int32_t Priority;
    int32_t Opacity;
    int32_t Time;
    sf::Clock FadeClock;
public: // hack
    uint8_t Type;
};

#endif
