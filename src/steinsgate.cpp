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
#include "nsbinterpreter.hpp"
#include "steinsgate.hpp"
#include "game.hpp"

#include <SFML/Graphics/Sprite.hpp>

const int16_t PHONE_ANIM_SPEED = 40; // TODO: guess
const int8_t PHONE_ANIM_ROW_MAX = 1;
const int8_t PHONE_ANIM_COLUMN_MAX = 4;
const int8_t PHONE_ANIM_UNDERFLOW = -1;
const int16_t PHONE_WIDTH = 313;
const int16_t PHONE_HEIGHT = 600;
const int16_t PHONE_POS_X = 700; // TODO: guess
const int16_t PHONE_POS_Y = PHONE_HEIGHT - WINDOW_HEIGHT;
const int32_t PHONE_PRIORITY = 20000 - 1; // BoxImage01.Priority - 1 (function.nss)

Phone::Phone(sf::Drawable* pDrawable) :
DrawableBase(pDrawable, PHONE_PRIORITY, DRAWABLE_TEXTURE)
{
    ToSprite()->setPosition(PHONE_POS_X, PHONE_POS_Y);
}

Phone::~Phone()
{
}

void Phone::UpdateOpenMode(int32_t OpenMode)
{
    // TODO: Don't "jump" to end of animation if it didn't finish
    State = OpenMode;
    switch (State)
    {
        case PHONE_OPENING: AnimRow = PHONE_ANIM_ROW_MAX; AnimColumn = PHONE_ANIM_COLUMN_MAX; break;
        case PHONE_CLOSING: AnimRow = 0; AnimColumn = 0; break;
    }
    UpdateAnim();
    AnimClock.restart();
}

void Phone::Update()
{
    if (AnimClock.getElapsedTime().asMilliseconds() < PHONE_ANIM_SPEED || State == PHONE_NONE)
        return;

    UpdateAnim();
    AnimClock.restart();
}

void Phone::UpdateAnim()
{
    //
    // See: cg/sys/phone/phone_open_anim.png
    //
    sf::IntRect ClipArea(AnimColumn * PHONE_WIDTH, AnimRow * PHONE_HEIGHT, PHONE_WIDTH, PHONE_HEIGHT);
    ToSprite()->setTextureRect(ClipArea);

    // Check if animation is done
    switch (State)
    {
        case PHONE_OPENING:
            if (AnimColumn == 0 && AnimRow == 0)
                State = PHONE_NONE;
            break;
        case PHONE_CLOSING:
            if (AnimColumn == PHONE_ANIM_COLUMN_MAX && AnimRow == PHONE_ANIM_ROW_MAX)
                State = PHONE_NONE;
            break;
    }

    // Advance animation progress
    switch (State)
    {
        case PHONE_OPENING: --AnimColumn; break;
        case PHONE_CLOSING: ++AnimColumn; break;
        case PHONE_NONE: return;
    }

    // Go to previous row
    if (AnimColumn == PHONE_ANIM_UNDERFLOW)
    {
        AnimColumn = PHONE_ANIM_COLUMN_MAX;
        --AnimRow;
    }
    // Go to next row
    else if (AnimColumn > PHONE_ANIM_COLUMN_MAX)
    {
        AnimColumn = 0;
        ++AnimRow;
    }
}

void NsbInterpreter::SGPhoneOpen()
{
    static Phone* pPhone = nullptr;
    static sf::Texture* pPhoneOpenTex = nullptr;
    
    if (!pPhoneOpenTex)
        pPhoneOpenTex = LoadTextureFromFile("cg/sys/phone/phone_open_anim.png", sf::IntRect());

    if (!pPhone)
        pPhone = new Phone(new sf::Sprite(*pPhoneOpenTex));

    pPhone->UpdateOpenMode(GetVariable<int32_t>("$SF_Phone_Open"));

    pGame->RemoveDrawable(pPhone);
    pGame->AddDrawable(pPhone);
}