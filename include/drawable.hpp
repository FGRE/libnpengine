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
#ifndef DRAWABLE_HPP
#define DRAWABLE_HPP

#include <cstdint>
#include <SFML/System/Clock.hpp>
#include <SFML/Graphics/Shader.hpp>

enum
{
    DRAWABLE_TEXTURE    = 0,
    DRAWABLE_TEXT       = 1
};

enum
{
    EFFECT_MOVE,
    EFFECT_ZOOM,
    EFFECT_FADE,
    EFFECT_MASK,
    EFFECT_BLUR,
    EFFECT_MAX
};

namespace sf
{
    class Sprite;
    class Text;
    class Drawable;
    class Texture;
    class RenderWindow;
    class RenderTexture;
};

class Effect;

class DrawableBase
{
public:
    DrawableBase(sf::Drawable* pDrawable, int32_t Priority, uint8_t Type);
    virtual ~DrawableBase() = 0;

    virtual void Draw(sf::RenderWindow* pWindow);
    virtual void Update() {}

    int32_t GetPriority() const { return Priority; }
    sf::Text* ToText() const { return (sf::Text*)pDrawable; }
    sf::Sprite* ToSprite() const { return (sf::Sprite*)pDrawable; }

    uint8_t Type;
protected:
    int32_t Priority;
    sf::Drawable* pDrawable;
};

class Drawable : public DrawableBase
{
public:
    Drawable(sf::Drawable* pDrawable, int32_t Priority, uint8_t Type);
    virtual ~Drawable();

    virtual void Draw(sf::RenderWindow* pWindow);

    void SetBlur(const std::string& Heaviness);
    void Fade(int32_t NewOpacity, int32_t Time);
    void SetMask(sf::Texture* pTexture, int32_t Start, int32_t End, int32_t Time);
    void Move(int32_t x, int32_t y, int32_t Time);
    void Zoom(int32_t x, int32_t y, int32_t Time);
    void SetCenter(int32_t x, int32_t y);

private:
    sf::Vector2f Center;
    Effect* Effects[EFFECT_MAX];
    sf::Clock Clock;
};

#endif
