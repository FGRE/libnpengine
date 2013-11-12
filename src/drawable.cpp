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
#include "drawable.hpp"

#include <sfeMovie/Movie.hpp>

Drawable::Drawable(sf::Drawable* pDrawable, int32_t Priority, uint8_t Type) :
pDrawable(pDrawable),
Priority(Priority),
Type(Type)
{
}

Drawable::~Drawable()
{
    delete pDrawable;
}

int32_t Drawable::GetPriority() const
{
    return Priority;
}

sf::Drawable* Drawable::Get() const
{
    return pDrawable;
}

bool Drawable::ShouldRemove() const
{
    if (Type == DRAWABLE_MOVIE)
        return static_cast<sfe::Movie*>(pDrawable)->getStatus() != sfe::Movie::Playing;
    return false;
}
