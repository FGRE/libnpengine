/* 
 * libnpengine: Nitroplus script interpreter
 * Copyright (C) 2014-2016,2018 Mislav Blažević <krofnica996@gmail.com>
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
#ifndef CHOICE_HPP
#define CHOICE_HPP

#include "Texture.hpp"
#include "Name.hpp"
#include <SDL2/SDL_events.h>

class Choice : public Name
{
    enum
    {
        FOCUS_UP,
        FOCUS_DOWN,
        FOCUS_RIGHT,
        FOCUS_LEFT
    };

public:
    Choice();

    bool IsSelected(const SDL_Event& Event);
    void SetNextFocus(Choice* pNext, const string& Key);
    void Reset() { ButtonUp = false; }

private:
    void Cursor(int x, int y, bool& Flag);
    void Wheel(int x, int y);
    void Arrow(SDL_Keycode sym);
    void ChangeFocus(int Index);
    int KeyToIndex(const string& Key);

    bool MouseOver;
    bool ButtonDown;
    bool ButtonUp;
    Choice* pNextFocus[4];
};

#endif
