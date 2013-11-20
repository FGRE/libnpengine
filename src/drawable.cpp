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
#include <SFML/Graphics/Sprite.hpp>

static const float FadeConvert = 0.255f;

Drawable::Drawable(sf::Drawable* pDrawable, int32_t Priority, uint8_t Type) :
pDrawable(pDrawable),
Priority(Priority),
Time(0),
Type(Type)
{
}

Drawable::~Drawable()
{
    if (Type == DRAWABLE_MOVIE)
        static_cast<sfe::Movie*>(pDrawable)->stop();
    else if (Type == DRAWABLE_TEXTURE);
        delete static_cast<sf::Sprite*>(pDrawable)->getTexture();
    delete pDrawable;
}

void Drawable::Update()
{
}

void Drawable::Fade(int32_t Opacity, int32_t Time)
{
    this->Opacity = Opacity;
    this->Time = Time;
    FadeClock.restart();

    // Hack: Move to update()
    float Alpha = Opacity / FadeConvert;
    sf::Uint8 UAlpha = static_cast<sf::Uint8>(Alpha);

    if (Type == DRAWABLE_TEXTURE)
        static_cast<sf::Sprite*>(pDrawable)->setColor(sf::Color(0xFF, 0xFF, 0xFF, UAlpha));
    else
        static_cast<sfe::Movie*>(pDrawable)->setColor(sf::Color(0xFF, 0xFF, 0xFF, UAlpha));

}

int32_t Drawable::GetPriority() const
{
    return Priority;
}

sf::Drawable* Drawable::Get() const
{
    return pDrawable;
}
