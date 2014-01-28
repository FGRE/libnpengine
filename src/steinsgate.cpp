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

#include <SFML/Graphics/Texture.hpp>

const int16_t PHONE_ANIM_SPEED = 40; // TODO: guess
const int8_t PHONE_ANIM_ROW_MAX = 1;
const int8_t PHONE_ANIM_COLUMN_MAX = 4;
const int8_t PHONE_ANIM_UNDERFLOW = -1;
const int16_t PHONE_WIDTH = 313;
const int16_t PHONE_HEIGHT = 576;
const int16_t PHONE_POS_X = 700; // TODO: guess
const int16_t PHONE_POS_Y = 10;
const int32_t PHONE_PRIORITY = 20000 - 1; // BoxImage01.Priority - 1 (function.nss)

// cg/sys/phone/phone_01.png
const int16_t PHONE_TEX_X = 95; // TODO: guess
const int16_t PHONE_TEX_Y = 0; // TODO: guess
const int16_t PHONE_HEADER_TEX_X = 670;
const int16_t PHONE_HEADER_TEX_Y = 384;
const int16_t PHONE_HEADER_WIDTH = 220;
const int16_t PHONE_HEADER_HEIGHT = 24;

const int16_t PHONE_HEADER_POS_X = PHONE_POS_X + 48; // TODO: guess
const int16_t PHONE_HEADER_POS_Y = PHONE_POS_Y + 87; // TODO: guess
const int16_t PHONE_WALLPAPER_X = PHONE_HEADER_POS_X; // TODO: guess
const int16_t PHONE_WALLPAPER_Y = PHONE_HEADER_POS_Y + PHONE_HEADER_HEIGHT; // TODO: guess

enum PhoneMode
{
    MODE_ADDRESS_BOOK = 0,
    MODE_ADDRESS_CONFIRM_DIAL,
    MODE_ADDRESS_CONFIRM_MAIL,
    MODE_CALLING,
    MODE_COMPLETE_RECEIVE_MAIL,
    MODE_COMPLETE_SEND_MAIL,
    MODE_DEFAULT,
    MODE_DEFAULT_OPERATABLE,
    MODE_DIALOG_SEND_MAIL_EDIT,
    MODE_ENGAGE_NORMAL,
    MODE_ENGAGE_VISUAL,
    MODE_MAIL_MENU,
    MODE_MAIL_SUB_MENU,
    MODE_POWER_OFF,
    MODE_RECEIVE_BOX,
    MODE_RECEIVED_MAIL,
    MODE_RECEIVING_MAIL,
    MODE_SEND_BOX,
    MODE_SENDING,
    MODE_SEND_MAIL_EDIT,

    MODE_INVALID = 0xFF // Custom
};

const string PhoneModeString[] =
{
    "PhoneMode_AddressBook",
    "PhoneMode_AddressConfirmDial",
    "PhoneMode_AddressConfirmMail",
    "PhoneMode_Calling",
    "PhoneMode_CompleteReceiveMail",
    "PhoneMode_CompleteSendMail",
    "PhoneMode_Default",
    "PhoneMode_DefaultOperatable",
    "PhoneMode_DialogSendMailEdit",
    "PhoneMode_EngageNormal",
    "PhoneMode_EngageVisual",
    "PhoneMode_MailMenu",
    "PhoneMode_MailSubMenu",
    "PhoneMode_PowerOff",
    "PhoneMode_ReceiveBox",
    "PhoneMode_ReceivedMail",
    "PhoneMode_ReceivingMail",
    "PhoneMode_SendBox",
    "PhoneMode_Sending",
    "PhoneMode_SendMailEdit"
};

Phone::Phone(sf::Drawable* pDrawable) :
DrawableBase(pDrawable, PHONE_PRIORITY, DRAWABLE_TEXTURE)
{
    Header.setPosition(PHONE_HEADER_POS_X, PHONE_HEADER_POS_Y);
    Wallpaper.setPosition(PHONE_WALLPAPER_X, PHONE_WALLPAPER_Y);
    ToSprite()->setPosition(PHONE_POS_X, PHONE_POS_Y);
    pPhoneTex = LoadTextureFromFile("cg/sys/phone/phone_01.png", sf::IntRect());
    pPhoneOpenTex = LoadTextureFromFile("cg/sys/phone/phone_open_anim.png", sf::IntRect());
}

Phone::~Phone()
{
    delete pPhoneTex;
    delete pPhoneOpenTex;
    delete Header.getTexture();
    delete Wallpaper.getTexture();
}

void Phone::UpdateOpenMode(int32_t OpenMode)
{
    // TODO: Don't "jump" to end of animation if it didn't finish
    ToSprite()->setTexture(*pPhoneOpenTex);
    State = OpenMode;
    switch (State)
    {
        case PHONE_OPENING: AnimRow = PHONE_ANIM_ROW_MAX; AnimColumn = PHONE_ANIM_COLUMN_MAX; break;
        case PHONE_CLOSING: AnimRow = 0; AnimColumn = 0; break;
    }
    UpdateAnim();
    AnimClock.restart();
}

void Phone::Draw(sf::RenderWindow* pWindow)
{
    DrawableBase::Draw(pWindow);
    if (Mode != MODE_POWER_OFF && State == PHONE_OPEN)
    {
        pWindow->draw(Wallpaper);
        pWindow->draw(Header);
    }
}

void Phone::Update()
{
    if (AnimClock.getElapsedTime().asMilliseconds() < PHONE_ANIM_SPEED
        || State == PHONE_OPEN || State == PHONE_CLOSED)
        return;

    UpdateAnim();
    AnimClock.restart();
}

void Phone::UpdateAnim()
{
    //
    // See: cg/sys/phone/phone_open_anim.png
    //

    // Animation finished in last call
    if (State == PHONE_OPENING_DONE)
    {
        sf::IntRect ClipArea(PHONE_TEX_X, PHONE_TEX_Y, PHONE_WIDTH, PHONE_HEIGHT);
        ToSprite()->setTexture(*pPhoneTex);
        ToSprite()->setTextureRect(ClipArea);
        State = PHONE_OPEN;
        return;
    }
    else if (State == PHONE_CLOSING_DONE)
    {
        delete pDrawable;
        pDrawable = new sf::Sprite;
        ToSprite()->setPosition(PHONE_POS_X, PHONE_POS_Y);
        State = PHONE_CLOSED;
        return;
    }

    // Animation is not done: set next frame
    sf::IntRect ClipArea(AnimColumn * PHONE_WIDTH, AnimRow * PHONE_HEIGHT, PHONE_WIDTH, PHONE_HEIGHT);
    ToSprite()->setTextureRect(ClipArea);

    // Check if animation is done
    switch (State)
    {
        case PHONE_OPENING:
            if (AnimColumn == 0 && AnimRow == 0)
                State = PHONE_OPENING_DONE;
            break;
        case PHONE_CLOSING:
            if (AnimColumn == PHONE_ANIM_COLUMN_MAX && AnimRow == PHONE_ANIM_ROW_MAX)
                State = PHONE_CLOSING_DONE;
            break;
    }

    // Advance animation progress
    switch (State)
    {
        case PHONE_OPENING: --AnimColumn; break;
        case PHONE_CLOSING: ++AnimColumn; break;
        case PHONE_OPENING_DONE: case PHONE_CLOSING_DONE: return;
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

void Phone::UpdateMode(uint8_t NewMode)
{
    if (NewMode == Mode)
        return;

    Mode = NewMode;
    switch (Mode)
    {
        case MODE_DEFAULT:
            if (sf::Texture* pTexture = LoadTextureFromFile("cg/sys/phone/pwcg101.png", sf::IntRect()))
                SetWallpaper(pTexture);

            if (!Header.getTexture())
            {
                sf::IntRect ClipArea(PHONE_HEADER_TEX_X, PHONE_HEADER_TEX_Y, PHONE_HEADER_WIDTH, PHONE_HEADER_HEIGHT);
                Header.setTexture(*pPhoneTex);
                Header.setTextureRect(ClipArea);
            }
            break;
        case MODE_POWER_OFF:
            break;
    }
}

void Phone::SetWallpaper(sf::Texture* pTexture)
{
    delete Wallpaper.getTexture();
    Wallpaper.setTexture(*pTexture);
}

void NsbInterpreter::SGPhoneOpen()
{
    pPhone->UpdateOpenMode(GetVariable<int32_t>("$SF_Phone_Open"));
    pGame->RemoveDrawable(pPhone);
    pGame->AddDrawable(pPhone);
}

void NsbInterpreter::SGPhoneMode()
{
    string StringMode = GetVariable<string>("$SW_PHONE_MODE");
    uint8_t Mode = MODE_INVALID;

    for (uint8_t i = 0; i <= MODE_SEND_MAIL_EDIT; ++i)
    {
        if (StringMode == PhoneModeString[i])
        {
            Mode = i;
            break;
        }
    }

    if (NsbAssert(Mode != MODE_INVALID, "Invalid phone mode specified: %", StringMode.c_str()))
        return;

    pPhone->UpdateMode(Mode);
}
