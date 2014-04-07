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
#ifndef EFFECT_HPP
#define EFFECT_HPP

#include <cassert>

class Effect
{
public:
    virtual ~Effect() { }
    virtual bool OnDraw(sf::RenderWindow* pWindow, sf::Sprite* pSprite, int32_t diff) = 0;

protected:
    float Lerp(float Old, float New, float Progress)
    {
        if (New > Old)
            return Old + (New - Old) * Progress;
        else
            return Old - (Old - New) * Progress;
    }

    sf::Vector2f Lerp(sf::Vector2f Old, sf::Vector2f New, float Progress)
    {
        return sf::Vector2f(Lerp(Old.x, New.x, Progress), Lerp(Old.y, New.y, Progress));
    }
};

class LerpEffect : public Effect
{
public:
    virtual ~LerpEffect() { }
    bool OnDraw(sf::RenderWindow* pWindow, sf::Sprite* pSprite, int32_t diff) = 0;

protected:
    void Reset(sf::Vector2f Start, sf::Vector2f End, int32_t Time)
    {
        this->Start = Start;
        this->End = End;
        this->Time = Time;
        this->ElapsedTime = 0;
    }

    sf::Vector2f Update(int32_t diff)
    {
        ElapsedTime += diff;
        float Progress = float(ElapsedTime) / float(Time);
        if (Progress > 1.0f)
            Progress = 1.0f;
        return Lerp(Start, End, Progress);
    }

    sf::Vector2f Start;
    sf::Vector2f End;
    int32_t ElapsedTime;
    int32_t Time;
};

class MoveEffect : public LerpEffect
{
public:
    MoveEffect(sf::Vector2f EndPos, int32_t Time)
    {
        Reset(EndPos, Time);
    }

    void Reset(sf::Vector2f EndPos, int32_t Time)
    {
        LerpEffect::Reset(this->End, EndPos, Time);
    }

    bool OnDraw(sf::RenderWindow* pWindow, sf::Sprite* pSprite, int32_t diff)
    {
        pSprite->setPosition(pSprite->getPosition() + Start + Update(diff));
        return false;
    }
};

class ZoomEffect : public LerpEffect
{
public:
    ZoomEffect(sf::Vector2f EndScale, int32_t Time)
    {
        Reset(EndScale, Time);
    }

    void Reset(sf::Vector2f EndScale, int32_t Time)
    {
        LerpEffect::Reset(this->End, EndScale, Time);
    }

    bool OnDraw(sf::RenderWindow* pWindow, sf::Sprite* pSprite, int32_t diff)
    {
        pSprite->setScale(Update(diff));
        sf::Vector2f GlobalBounds = sf::Vector2f(pSprite->getGlobalBounds().width, pSprite->getGlobalBounds().height);
        sf::Vector2f LocalBounds = sf::Vector2f(pSprite->getLocalBounds().width, pSprite->getLocalBounds().height);
        sf::Vector2f PositionOffset = (GlobalBounds - LocalBounds) / -2.0f;
        pSprite->setPosition(pSprite->getPosition() + PositionOffset);
        return false;
    }
};

class FadeEffect : public LerpEffect
{
    const float FadeConvert = 0.255f;
public:
    FadeEffect(float EndOpacity, int32_t Time)
    {
        Reset(EndOpacity, Time);
        this->Start = sf::Vector2f(1000, 0); // Objects are visible by default
    }

    void Reset(float EndOpacity, int32_t Time)
    {
        LerpEffect::Reset(this->End, sf::Vector2f(EndOpacity, 0), Time);
    }

    bool OnDraw(sf::RenderWindow* pWindow, sf::Sprite* pSprite, int32_t diff)
    {
        float Alpha = Update(diff).x;
        pSprite->setColor(sf::Color(0xFF, 0xFF, 0xFF, Alpha * FadeConvert));
        return false;
    }
};

class MaskEffect : public LerpEffect
{
    const std::string MaskShader = \
        "uniform sampler2D Texture;"
        "uniform sampler2D Mask;"
        "uniform float Alpha;"
        "uniform float Target;"
        "void main()"
        "{"
        "   vec4 Pixel = texture2D(Texture, gl_TexCoord[0].xy);"
        "   vec4 MaskPixel = texture2D(Mask, gl_TexCoord[0].xy);"
        "   MaskPixel.x = 1.0f - MaskPixel.x;"
        "   Pixel.a = MaskPixel.x * (Alpha / Target);"
        "   gl_FragColor = Pixel;"
        "}";
public:
    MaskEffect(sf::Texture* pMask, int32_t StartOpacity, int32_t EndOpacity, int32_t Time)
    {
        if (!Shader.loadFromMemory(MaskShader, sf::Shader::Fragment))
            assert(false);
        Shader.setParameter("Texture", sf::Shader::CurrentTexture);
        Reset(pMask, StartOpacity, EndOpacity, Time);
    }

    ~MaskEffect()
    {
        delete pMask;
    }

    void Reset(sf::Texture* pMask, int32_t StartOpacity, int32_t EndOpacity, int32_t Time)
    {
        this->pMask = pMask;
        LerpEffect::Reset(sf::Vector2f(StartOpacity, 0), sf::Vector2f(EndOpacity, 0), Time);
        Shader.setParameter("Mask", *pMask);
        Shader.setParameter("Target", (End.x > Start.x ? End.x : Start.x) * 0.255f); // TODO: Not needed. Really.
    }

    bool OnDraw(sf::RenderWindow* pWindow, sf::Sprite* pSprite, int32_t diff)
    {
        Shader.setParameter("Alpha", Update(diff).x * 0.255f);
        pWindow->draw(*pSprite, &Shader);
        return true;
    }

private:
    sf::Shader Shader;
    sf::Texture* pMask;
};

class BlurEffect : public Effect
{
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
public:
    BlurEffect()
    {
        pBlur = new sf::RenderTexture;
        pBlur->create(1024, 576);
        if (!Shader.loadFromMemory(BlurShader, sf::Shader::Fragment))
            assert(false);
        Shader.setParameter("Sigma", 3.0f); // Guess for SEMIHEAVY
        Shader.setParameter("Texture", sf::Shader::CurrentTexture);
    }

    ~BlurEffect()
    {
        delete pBlur;
    }

    bool OnDraw(sf::RenderWindow* pWindow, sf::Sprite* pSprite, int32_t diff)
    {
        Shader.setParameter("BlurSize", 1.0f / 1024.0f);
        Shader.setParameter("Pass", sf::Vector2f(1.0f, 0.0f));
        pBlur->draw(*pSprite, &Shader);
        pBlur->display();
        Shader.setParameter("BlurSize", 1.0f / 576.0f);
        Shader.setParameter("Pass", sf::Vector2f(0.0f, 1.0f));
        pBlur->draw(sf::Sprite(pBlur->getTexture()), &Shader);
        pBlur->display();
        pWindow->draw(sf::Sprite(pBlur->getTexture()));
        return true;
    }

private:
    sf::RenderTexture* pBlur;
    sf::Shader Shader;
};

#endif
