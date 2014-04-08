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

#include "effect.hpp"
#include "game.hpp"

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
DrawableBase(pDrawable, Priority, Type)
{
    for (int i = 0; i < EFFECT_MAX; ++i)
        Effects[i] = nullptr;
}

Drawable::~Drawable()
{
    if (Type == DRAWABLE_TEXTURE)
        delete ToSprite()->getTexture();
    delete pDrawable;
    for (int i = 0; i < EFFECT_MAX; ++i)
        delete Effects[i];
}

void Drawable::Draw(sf::RenderWindow* pWindow)
{
    ToSprite()->setPosition(sf::Vector2f(0.0f, 0.0f));

    int32_t diff = Clock.getElapsedTime().asMilliseconds();
    Clock.restart();

    bool Drawn = false;
    for (int i = 0; i < EFFECT_MAX; ++i)
        if (Effect* pEffect = Effects[i])
            Drawn |= pEffect->OnDraw(pWindow, ToSprite(), diff);

    if (!Drawn)
        pWindow->draw(*pDrawable);
}

void Drawable::Fade(int32_t NewOpacity, int32_t Time)
{
    if (!Effects[EFFECT_FADE])
        Effects[EFFECT_FADE] = new FadeEffect(NewOpacity, Time);
    else
        ((FadeEffect*)Effects[EFFECT_FADE])->Reset(NewOpacity, Time);
}

void Drawable::SetMask(sf::Texture* pTexture, int32_t Start, int32_t End, int32_t Time)
{
    if (!Effects[EFFECT_MASK])
        Effects[EFFECT_MASK] = new MaskEffect(pTexture, Start, End, Time);
    else
        ((MaskEffect*)Effects[EFFECT_MASK])->Reset(pTexture, Start, End, Time);
}

void Drawable::SetBlur(const std::string& Heaviness)
{
    if (!Effects[EFFECT_BLUR])
        Effects[EFFECT_BLUR] = new BlurEffect;
}

void Drawable::Move(int32_t x, int32_t y, int32_t Time)
{
    sf::Vector2f NewPos = sf::Vector2f(x, y);
    if (!Effects[EFFECT_MOVE])
        Effects[EFFECT_MOVE] = new MoveEffect(NewPos, Time);
    else
        ((MoveEffect*)Effects[EFFECT_MOVE])->Reset(NewPos, Time);
}

void Drawable::Zoom(int32_t x, int32_t y, int32_t Time)
{
    sf::Vector2f NewScale = sf::Vector2f(float(x) / 1000.0f, float(y) / 1000.0f);
    if (!Effects[EFFECT_ZOOM])
        Effects[EFFECT_ZOOM] = new ZoomEffect(NewScale, Time);
    else
        ((ZoomEffect*)Effects[EFFECT_ZOOM])->Reset(NewScale, Time);
}

void Drawable::SetCenter(int32_t x, int32_t y)
{
    Center = sf::Vector2f(x, y);
}

void Drawable::Request(Game* pGame, const string& State)
{
    if (State == "Smoothing")
    {
        pGame->GLCallback([this]()
        {
            sf::Texture* pTexture = const_cast<sf::Texture*>(ToSprite()->getTexture());
            pTexture->setSmooth(true);
        });
    }
}

void Drawable::Delete(Game* pGame, NsbInterpreter* pInterpreter)
{
    pGame->GLCallback(std::bind(&NsbInterpreter::GLDelete, pInterpreter, this));
}
