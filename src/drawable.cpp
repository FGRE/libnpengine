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
#include <SFML/Graphics/RenderWindow.hpp>

const float FadeConvert = 0.255f;

const std::string FadeShader = \
    "uniform sampler2D Texture;"
    "uniform sampler2D Mask;"
    "uniform float Alpha;"
    "uniform float Target;"
    "void main()" \
    "{" \
    "   vec4 Pixel = texture2D(Texture, gl_TexCoord[0].xy);"
    "   vec4 MaskPixel = texture2D(Mask, gl_TexCoord[0].xy);"
    "   Pixel.a = MaskPixel.x * (Alpha / Target);"
    "   gl_FragColor = Pixel;" \
    "}";

Drawable::Drawable(sf::Drawable* pDrawable, int32_t Priority, uint8_t Type) :
pDrawable(pDrawable),
Priority(Priority),
Type(Type),
pMask(nullptr)
{
    for (uint8_t i = 0; i < FADE_MAX; ++i)
        if (Type == DRAWABLE_TEXTURE || Type == DRAWABLE_MOVIE)
            Fades[i] = new FadeEffect;
        else
            Fades[i] = nullptr;
}

Drawable::~Drawable()
{
    if (Type == DRAWABLE_MOVIE)
        static_cast<sfe::Movie*>(pDrawable)->stop();
    else if (Type == DRAWABLE_TEXTURE);
        delete static_cast<sf::Sprite*>(pDrawable)->getTexture();
    delete pDrawable;
    delete pMask;
}

void Drawable::Update()
{
    for (uint8_t i = 0; i < FADE_MAX; ++i)
        UpdateFade(i);
}

void Drawable::Draw(sf::RenderWindow* pWindow)
{
    if (pMask)
        pWindow->draw(*pDrawable, &Shader);
    else
        pWindow->draw(*pDrawable);
}

void Drawable::UpdateFade(uint8_t Index)
{
    FadeEffect* pEffect = Fades[Index];
    if (!pEffect || pEffect->FadeTime == 0)
        return;

    float Alpha;
    int32_t Elapsed = pEffect->FadeClock.getElapsedTime().asMilliseconds();

    if (Elapsed >= pEffect->FadeTime)
    {
        pEffect->FadeTime = 0;
        Alpha = pEffect->TargetOpacity;
        pEffect->Opacity = pEffect->TargetOpacity;
    }
    else
    {
        float Progress = float(Elapsed) / float(pEffect->FadeTime);
        Alpha = float(pEffect->Opacity);
        if (pEffect->TargetOpacity > pEffect->Opacity)
            Alpha += float(pEffect->TargetOpacity - pEffect->Opacity) * Progress;
        else
            Alpha -= float(pEffect->Opacity - pEffect->TargetOpacity) * Progress;
    }

    Alpha *= FadeConvert;
    if (Index == FADE_MASK)
        Shader.setParameter("Alpha", Alpha);
    else
        SetAlpha(Alpha);
}

void Drawable::SetAlpha(sf::Uint8 Alpha)
{
    if (Type == DRAWABLE_TEXTURE)
        static_cast<sf::Sprite*>(pDrawable)->setColor(sf::Color(0xFF, 0xFF, 0xFF, Alpha));
    else if (Type == DRAWABLE_MOVIE)
        static_cast<sfe::Movie*>(pDrawable)->setColor(sf::Color(0xFF, 0xFF, 0xFF, Alpha));
}

void Drawable::SetOpacity(int32_t NewOpacity, int32_t Time, uint8_t Index)
{
    FadeEffect* pEffect = Fades[Index];
    pEffect->Opacity = pEffect->TargetOpacity;
    pEffect->TargetOpacity = NewOpacity;
    pEffect->FadeTime = Time;
    pEffect->FadeClock.restart();

    if (Time == 0)
    {
        float Alpha = NewOpacity * FadeConvert;
        SetAlpha(Alpha);
    }
}

void Drawable::SetMask(sf::Texture* pTexture, int32_t Start, int32_t End, int32_t Time)
{
    pMask = pTexture;
    Shader.loadFromMemory(FadeShader, sf::Shader::Fragment);
    Shader.setParameter("Mask", *pMask);
    Shader.setParameter("Texture", sf::Shader::CurrentTexture);
    Shader.setParameter("Target", End * FadeConvert);
    Fades[FADE_MASK]->TargetOpacity = Start; // Will be flipped to Opacity in SetOpacity
    SetOpacity(End, Time, FADE_MASK);
}

int32_t Drawable::GetPriority() const
{
    return Priority;
}

sf::Drawable* Drawable::Get() const
{
    return pDrawable;
}
