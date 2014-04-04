/* 
 * libnpengine: Nitroplus script interpreter
 * Copyright (C) 2013-2014 Mislav Blažević <krofnica996@gmail.com>
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

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RenderTexture.hpp>

const float FadeConvert = 0.255f;

const std::string FadeShader = \
    "uniform sampler2D Texture;"
    "uniform sampler2D Mask;"
    "uniform float Alpha;"
    "uniform float Target;"
    "void main()"
    "{"
    "   vec4 Pixel = texture2D(Texture, gl_TexCoord[0].xy);"
    "   vec4 MaskPixel = texture2D(Mask, gl_TexCoord[0].xy);"
    "   Pixel.a = MaskPixel.x * (Alpha / Target);"
    "   gl_FragColor = Pixel;"
    "}";

const std::string BlurShader = \
    "uniform float Sigma;"
    "uniform sampler2D Texture;"
    "uniform vec2 Pass;"
    "uniform float BlurSize;"
    "const float NumSamples = 4.0f;"
    "const float PI = 3.14159265f;"
    "void main()"
    "{"
    "   vec3 Gaussian = vec3(1.0f / (sqrt(2.0f * PI) * Sigma), exp(-0.5f / (Sigma * Sigma)), 0.0f);"
    "   Gaussian.z = Gaussian.y * Gaussian.y;"
    "   vec4 Average = texture2D(Texture, gl_TexCoord[0].xy) * Gaussian.x;"
    "   float CoeffSum = Gaussian.x;"
    "   Gaussian.xy *= Gaussian.yz;"
    "   for (float i = 1.0f; i <= NumSamples; ++i)"
    "   {"
    "       Average += texture2D(Texture, gl_TexCoord[0].xy + i * BlurSize * Pass) * Gaussian.x;"
    "       Average += texture2D(Texture, gl_TexCoord[0].xy - i * BlurSize * Pass) * Gaussian.x;"
    "       CoeffSum += 2.0f * Gaussian.x;"
    "       Gaussian.xy *= Gaussian.yz;"
    "   }"
    "   gl_FragColor = Average / CoeffSum;"
    "}";

float Lerp(float Old, float New, float Progress)
{
    if (New > Old)
        return Old + (New - Old) * Progress;
    else
        return Old - (Old - New) * Progress;
}

DrawableBase::DrawableBase(sf::Drawable* pDrawable, int32_t Priority, uint8_t Type) :
pDrawable(pDrawable),
Priority(Priority),
Type(Type)
{
}

DrawableBase::~DrawableBase()
{
}

void DrawableBase::Draw(sf::RenderWindow* pWindow)
{
    pWindow->draw(*pDrawable);
}

Drawable::Drawable(sf::Drawable* pDrawable, int32_t Priority, uint8_t Type) :
DrawableBase(pDrawable, Priority, Type),
pMask(nullptr),
pBlur(nullptr)
{
    for (uint8_t i = 0; i < FADE_MAX; ++i)
        Fades[i] = nullptr;
    for (uint8_t i = 0; i < LERP_MAX; ++i)
        Lerps[i] = nullptr;
}

Drawable::~Drawable()
{
    if (Type == DRAWABLE_TEXTURE)
        delete ToSprite()->getTexture();
    delete pDrawable;
    delete pMask;

    for (uint8_t i = 0; i < FADE_MAX; ++i)
        delete Fades[i];
    for (uint8_t i = 0; i < LERP_MAX; ++i)
        delete Lerps[i];
}

void Drawable::Update()
{
    sf::Vector2f NewPosition;

    if (Lerps[LERP_ZOOM])
    {
        ToSprite()->setScale(UpdateLerp(LERP_ZOOM));
        NewPosition = sf::Vector2f(ToSprite()->getGlobalBounds().width,
                                   ToSprite()->getGlobalBounds().height) -
                      sf::Vector2f(ToSprite()->getLocalBounds().width,
                                   ToSprite()->getLocalBounds().height);
        NewPosition /= -2.0f;
        ToSprite()->setPosition(NewPosition);
    }

    if (Lerps[LERP_ANIM])
    {
        Position = UpdateLerp(LERP_ANIM);
        if (Type == DRAWABLE_TEXTURE)
            ToSprite()->setPosition(NewPosition + Position);
    }

    for (uint8_t i = 0; i < FADE_MAX; ++i)
        UpdateFade(i);
}

sf::Vector2f Drawable::UpdateLerp(uint8_t i)
{
    LerpEffect* pLerp = Lerps[i];
    sf::Vector2f RetVal;
    float Progress = float(pLerp->Clock.getElapsedTime().asMilliseconds()) /
                     float(pLerp->Time);

    if (Progress > 1.0f)
        Progress = 1.0f;

    RetVal = sf::Vector2f(Lerp(pLerp->Old.x, pLerp->NewX, Progress),
                          Lerp(pLerp->Old.y, pLerp->NewY, Progress));

    if (pLerp->Clock.getElapsedTime().asMilliseconds() >= pLerp->Time)
    {
        delete pLerp;
        Lerps[i] = nullptr;
    }

    return RetVal;
}

void Drawable::Draw(sf::RenderWindow* pWindow)
{
    if (pBlur)
    {
        Shader.setParameter("BlurSize", 1.0f / 1024.0f);
        Shader.setParameter("Pass", sf::Vector2f(1.0f, 0.0f));
        pBlur->draw(*pDrawable, &Shader);
        pBlur->display();
        Shader.setParameter("BlurSize", 1.0f / 576.0f);
        Shader.setParameter("Pass", sf::Vector2f(0.0f, 1.0f));
        pBlur->draw(sf::Sprite(pBlur->getTexture()), &Shader);
        pBlur->display();
        pWindow->draw(sf::Sprite(pBlur->getTexture()));
    }
    else if (pMask)
        pWindow->draw(*pDrawable, &Shader);
    else
        pWindow->draw(*pDrawable);
}

void Drawable::UpdateFade(uint8_t Index)
{
    FadeEffect* pEffect = Fades[Index];
    if (!pEffect || pEffect->FadeTime == -1)
        return;

    float Alpha;
    int32_t Elapsed = pEffect->FadeClock.getElapsedTime().asMilliseconds();

    if (Elapsed >= pEffect->FadeTime)
    {
        pEffect->FadeTime = -1;
        Alpha = pEffect->TargetOpacity;
        pEffect->Opacity = pEffect->TargetOpacity;
    }
    else
    {
        float Progress = float(Elapsed) / float(pEffect->FadeTime);
        Alpha = Lerp(pEffect->Opacity, pEffect->TargetOpacity, Progress);
    }

    Alpha *= FadeConvert;
    if (Index == FADE_MASK)
        Shader.setParameter("Alpha", Alpha);
    else
    {
        if (Type == DRAWABLE_TEXTURE)
            ToSprite()->setColor(sf::Color(0xFF, 0xFF, 0xFF, Alpha));
    }
}

void Drawable::SetOpacity(int32_t NewOpacity, int32_t Time, uint8_t Index)
{
    if (!Fades[Index])
        Fades[Index] = new FadeEffect;
    FadeEffect* pEffect = Fades[Index];
    pEffect->Opacity = pEffect->TargetOpacity;
    pEffect->TargetOpacity = NewOpacity;
    pEffect->FadeTime = Time;
    pEffect->FadeClock.restart();
}

void Drawable::SetMask(sf::Texture* pTexture, int32_t Start, int32_t End, int32_t Time)
{
    if (!Shader.loadFromMemory(FadeShader, sf::Shader::Fragment))
    {
        delete pTexture;
        return;
    }

    if (pMask)
        delete pMask;

    pMask = pTexture;
    Shader.setParameter("Mask", *pMask);
    Shader.setParameter("Texture", sf::Shader::CurrentTexture);
    Shader.setParameter("Target", (End > Start ? End : Start) * FadeConvert);
    Fades[FADE_MASK]->TargetOpacity = Start; // Will be flipped to Opacity in SetOpacity
    SetOpacity(End, Time, FADE_MASK);
}

void Drawable::SetBlur(const std::string& Heaviness)
{
    pBlur = new sf::RenderTexture;
    pBlur->create(1024, 576);
    Shader.loadFromMemory(BlurShader, sf::Shader::Fragment);
    Shader.setParameter("Sigma", 3.0f); // Guess for SEMIHEAVY
    Shader.setParameter("Texture", sf::Shader::CurrentTexture);
}

void Drawable::AddLerpEffect(uint8_t EffIndex, int32_t x, int32_t y, int32_t Time)
{
    if (Lerps[EffIndex])
        delete Lerps[EffIndex];

    LerpEffect* pEffect = new LerpEffect;

    switch (EffIndex)
    {
        case LERP_ANIM:
            if (Type == DRAWABLE_TEXTURE)
                pEffect->Old = Position;
            break;
        case LERP_ZOOM:
            pEffect->Old = ToSprite()->getScale();
            break;
    }

    pEffect->NewX = x;
    pEffect->NewY = y;
    pEffect->Time = Time;
    Lerps[EffIndex] = pEffect;
}

void Drawable::SetCenter(int32_t x, int32_t y)
{
}
