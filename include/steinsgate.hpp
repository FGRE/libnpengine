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
#include "drawable.hpp"
#include <SFML/Graphics/Sprite.hpp>

enum PhoneState
{
    PHONE_CLOSING = 0,
    PHONE_OPENING = 1,
    PHONE_OPENING_DONE, // Switch from last opening animation frame to open phone frame
    PHONE_CLOSING_DONE, // Same as above, except that phone needs to be removed
    PHONE_OPEN,
    PHONE_CLOSED
};

class Phone : public DrawableBase
{
public:
    Phone(sf::Drawable* pDrawable);
    virtual ~Phone();

    virtual void Draw(sf::RenderWindow* pWindow);
    virtual void Update();
    void SetDate(string Date);
    void SDDisplay(int32_t Show);
    void MailReceive(int32_t Show);
    void UpdateOpenMode(int32_t OpenMode);
    void UpdateMode(uint8_t NewMode);
    void SetPriority(int32_t Priority);
    void MouseMoved(sf::Vector2i Pos);
    void LeftMouseClicked();
    void RightMouseClicked(NsbInterpreter* pInterpreter);

private:
    void UpdateAnim();
    bool HighlightButton(int x, int y);

    bool ShowSD;
    bool ShowOverlay;
    uint8_t Mode;
    uint8_t State;
    int8_t AnimRow;
    int8_t AnimColumn;
    int8_t ButtonHighlightX, ButtonHighlightY; // Currently highlighed button
    sf::Clock AnimClock;
    sf::Texture* pWallpaper;
    sf::Texture* pPhoneOpenTex; // Open/Close animation frames
    sf::Texture* pPhoneTex;
    sf::Texture* pSDTex;
    sf::Sprite Wallpaper;
    sf::Sprite Header;
    sf::Sprite Overlay;
    sf::Sprite SD;
    sf::Sprite SDDate[6];
    sf::Sprite SDIcon[4];
    sf::Sprite Button[2][2];
    sf::Sprite BlueHeader;
};
