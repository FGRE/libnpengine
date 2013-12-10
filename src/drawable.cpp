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

Drawable::Drawable(sf::Drawable* pDrawable, int32_t Priority, uint8_t Type) :
pDrawable(pDrawable),
Priority(Priority),
Type(Type),
pMask(nullptr),
pBlur(nullptr)
{
    for (uint8_t i = 0; i < FADE_MAX; ++i)
        if (Type == DRAWABLE_TEXTURE || Type == DRAWABLE_MOVIE)
            Fades[i] = new FadeEffect; // TODO: allocate on demand
        else
            Fades[i] = nullptr;
    Lerps[0] = Lerps[1] = nullptr;
}

Drawable::~Drawable()
{
    if (Type == DRAWABLE_MOVIE)
        static_cast<sfe::Movie*>(pDrawable)->stop();
    else if (Type == DRAWABLE_TEXTURE)
        delete static_cast<sf::Sprite*>(pDrawable)->getTexture();
    delete pDrawable;
    delete pMask;
}

void Drawable::Update()
{
    if (Lerps[LERP_ZOOM])
    {
        sf::Sprite* pSprite = static_cast<sf::Sprite*>(Get());
        pSprite->setScale(UpdateLerp(LERP_ZOOM));
        pSprite->setPosition(-(sf::Vector2f(pSprite->getGlobalBounds().width,
                                            pSprite->getGlobalBounds().height) -
                               sf::Vector2f(pSprite->getLocalBounds().width,
                                            pSprite->getLocalBounds().height)) / 2.f);
    }
    if (Lerps[LERP_ANIM])
    {
        sf::Vector2f NewPos = UpdateLerp(LERP_ANIM);
        if (Type == DRAWABLE_TEXTURE)
            ((sf::Sprite*)pDrawable)->setPosition(NewPos);
        else if (Type == DRAWABLE_MOVIE)
            ((sfe::Movie*)pDrawable)->setPosition(NewPos);
    }
    for (uint8_t i = 0; i < FADE_MAX; ++i)
        UpdateFade(i);
}

sf::Vector2f Drawable::UpdateLerp(uint8_t i)
{
    LerpEffect* pLerp = Lerps[i];
    float Progress = float(pLerp->Clock.getElapsedTime().asMilliseconds()) /
                     float(pLerp->Time);
    if (Progress > 1.0f)
        Progress = 1.0f;

    float NewX = Lerp(pLerp->OldX, pLerp->NewX, Progress);
    float NewY = Lerp(pLerp->OldY, pLerp->NewY, Progress);

    if (pLerp->Clock.getElapsedTime().asMilliseconds() >= pLerp->Time)
    {
        delete pLerp;
        Lerps[i] = nullptr;
    }
    return sf::Vector2f(NewX, NewY);
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
        Alpha = Lerp(pEffect->Opacity, pEffect->TargetOpacity, Progress);
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

void Drawable::Animate(int32_t x, int32_t y, int32_t Time)
{
    if (Lerps[LERP_ANIM])
        delete Lerps[LERP_ANIM];

    LerpEffect* pAnimation = new LerpEffect;
    sf::Vector2f CurrPos = (Type == DRAWABLE_TEXTURE ? static_cast<sf::Sprite*>(pDrawable)->getPosition() :
                                                       static_cast<sfe::Movie*>(pDrawable)->getPosition());
    pAnimation->OldX = CurrPos.x;
    pAnimation->OldY = CurrPos.y;
    pAnimation->NewX = x;
    pAnimation->NewY = y;
    pAnimation->Time = Time;
    Lerps[LERP_ANIM] = pAnimation;
}

void Drawable::Zoom(float x, float y, int32_t Time)
{
    LerpEffect* pZoom = new LerpEffect;
    sf::Vector2f CurrScale = static_cast<sf::Sprite*>(pDrawable)->getScale();
    pZoom->OldX = CurrScale.x;
    pZoom->OldY = CurrScale.y;
    pZoom->NewX = x / 1000.0f;
    pZoom->NewY = y / 1000.0f;
    pZoom->Time = Time;
    Lerps[LERP_ZOOM] = pZoom;
}

int32_t Drawable::GetPriority() const
{
    return Priority;
}

sf::Drawable* Drawable::Get() const
{
    return pDrawable;
}
