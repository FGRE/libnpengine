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

const float FadeConvert = 0.255f;

Drawable::Drawable(sf::Drawable* pDrawable, int32_t Priority, uint8_t Type) :
pDrawable(pDrawable),
Priority(Priority),
TargetOpacity(0),
Opacity(0),
FadeTime(0),
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
    if (FadeTime == 0)
        return;

    float Alpha;
    int32_t Elapsed = FadeClock.getElapsedTime().asMilliseconds();

    if (Elapsed >= FadeTime)
    {
        FadeTime = 0;
        Alpha = TargetOpacity;
        Opacity = TargetOpacity;
    }
    else
    {
        float Progress = float(Elapsed) / float(FadeTime);
        Alpha = float(Opacity);
        if (TargetOpacity > Opacity)
            Alpha += float(TargetOpacity - Opacity) * Progress;
        else
            Alpha -= float(Opacity - TargetOpacity) * Progress;
    }

    Alpha *= FadeConvert;
    SetAlpha(Alpha);
}

void Drawable::SetAlpha(sf::Uint8 Alpha)
{
    if (Type == DRAWABLE_TEXTURE)
        static_cast<sf::Sprite*>(pDrawable)->setColor(sf::Color(0xFF, 0xFF, 0xFF, Alpha));
    else if (Type == DRAWABLE_MOVIE)
        static_cast<sfe::Movie*>(pDrawable)->setColor(sf::Color(0xFF, 0xFF, 0xFF, Alpha));
}

void Drawable::SetOpacity(int32_t NewOpacity, int32_t Time)
{
    Opacity = TargetOpacity;
    TargetOpacity = NewOpacity;
    FadeTime = Time;
    FadeClock.restart();

    if (Time == 0)
    {
        float Alpha = NewOpacity * FadeConvert;
        SetAlpha(Alpha);
    }
}

int32_t Drawable::GetPriority() const
{
    return Priority;
}

sf::Drawable* Drawable::Get() const
{
    return pDrawable;
}
