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
#include <SFML/Graphics/Shader.hpp>

enum
{
    DRAWABLE_MOVIE      = 0,
    DRAWABLE_TEXTURE    = 1,
    DRAWABLE_TEXT       = 2
};

enum
{
    FADE_TEX    = 0,
    FADE_MASK   = 1,
    FADE_MAX    = 2
};

enum
{
    LERP_ZOOM   = 0,
    LERP_ANIM   = 1,
    LERP_MAX    = 2
};

namespace sf
{
    class Drawable;
    class Texture;
    class RenderWindow;
    class RenderTexture;
};

struct FadeEffect // TODO: Also LerpEffect
{
    FadeEffect() : TargetOpacity(0), Opacity(0), FadeTime(0) { }
    int32_t TargetOpacity;
    int32_t Opacity;
    int32_t FadeTime;
    sf::Clock FadeClock;
};

struct LerpEffect
{
    float OldX;
    float OldY;
    float NewX;
    float NewY;
    int32_t Time;
    sf::Clock Clock;
};

class Drawable
{
public:
    Drawable(sf::Drawable* pDrawable, int32_t Priority, uint8_t Type);
    virtual ~Drawable();

    void Draw(sf::RenderWindow* pWindow);
    void Update();
    void Zoom(float x, float y, int32_t Time);
    void SetBlur(const std::string& Heaviness);
    void SetOpacity(int32_t NewOpacity, int32_t Time, uint8_t Index);
    void SetMask(sf::Texture* pTexture, int32_t Start, int32_t End, int32_t Time);
    virtual void Animate(int32_t x, int32_t y, int32_t Time);
    int32_t GetPriority() const;
    sf::Drawable* Get() const;

    uint8_t Type;
protected:
    sf::Vector2f UpdateLerp(uint8_t i);
    LerpEffect* Lerps[LERP_MAX];
    FadeEffect* Fades[FADE_MAX];
    sf::Drawable* pDrawable;
    int32_t Priority;
private:
    void UpdateFade(uint8_t Index);
    void SetAlpha(sf::Uint8 Alpha);
    sf::Shader Shader;
    sf::Texture* pMask;
    sf::RenderTexture* pBlur;
};

extern const float FadeConvert;

#endif
