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
#include "Choice.hpp"
#include "Window.hpp"

Choice::Choice() : MouseOver(false), ButtonDown(false), ButtonUp(false)
{
    Write("MouseUsual", new Name);
    Write("MouseOver", new Name);
    Write("MouseClick", new Name);
    for (int i = 0; i < 4; ++i)
        pNextFocus[i] = nullptr;
}

bool Choice::IsSelected(const SDL_Event& Event)
{
    switch (Event.type)
    {
        case SDL_MOUSEBUTTONUP: Cursor(Event.button.x, Event.button.y, ButtonUp); break;
        case SDL_MOUSEBUTTONDOWN: Cursor(Event.button.x, Event.button.y, ButtonDown); break;
        case SDL_MOUSEMOTION: Cursor(Event.motion.x, Event.motion.y, MouseOver); break;
        case SDL_MOUSEWHEEL: Wheel(Event.wheel.x, Event.wheel.y); break;
        case SDL_KEYDOWN: Arrow(Event.key.keysym.sym); break;
    }

    Texture* pMouseOver = dynamic_cast<Texture*>(Read("MouseOver/img"));
    Texture* pMouseClick = dynamic_cast<Texture*>(Read("MouseClick/img"));
    Texture* pMouseUsual = dynamic_cast<Texture*>(Read("MouseUsual/img"));

    if (pMouseOver) pMouseOver->Fade(0, 0);
    if (pMouseClick) pMouseClick->Fade(0, 0);
    if (pMouseUsual) pMouseUsual->Fade(0, 0);

    if (MouseOver && pMouseOver) pMouseOver->Fade(0, 1000);
    else if (ButtonDown && pMouseClick) pMouseClick->Fade(0, 1000);
    else if (pMouseUsual) pMouseUsual->Fade(0, 1000);

    return ButtonUp;
}

void Choice::SetNextFocus(Choice* pNext, const string& Key)
{
    pNextFocus[KeyToIndex(Key)] = pNext;
}

void Choice::Cursor(int x, int y, bool& Flag)
{
    Texture* pTexture = dynamic_cast<Texture*>(Read("MouseUsual/img"));
    if (!pTexture) return;
    int x1 = pTexture->GetX();
    int x2 = x1 + pTexture->GetWidth();
    int y1 = pTexture->GetY();
    int y2 = y1 + pTexture->GetHeight();
    Flag = x > x1 && x < x2 && y > y1 && y < y2;
}

void Choice::Wheel(int x, int y)
{
    if (y > 0) ChangeFocus(FOCUS_UP);
    else if (y < 0) ChangeFocus(FOCUS_DOWN);
    else if (x > 0) ChangeFocus(FOCUS_RIGHT);
    else if (x < 0) ChangeFocus(FOCUS_LEFT);
}

void Choice::Arrow(SDL_Keycode sym)
{
    switch (sym)
    {
        case SDLK_UP: ChangeFocus(FOCUS_UP); break;
        case SDLK_DOWN: ChangeFocus(FOCUS_DOWN); break;
        case SDLK_RIGHT: ChangeFocus(FOCUS_RIGHT); break;
        case SDLK_LEFT: ChangeFocus(FOCUS_LEFT); break;
    }
}

void Choice::ChangeFocus(int Index)
{
    if (Choice* pChoice = pNextFocus[Index])
        if (Texture* pTexture = dynamic_cast<Texture*>(pChoice->Read("MouseOver/img")))
            Window::PushMoveCursorEvent(pTexture->GetX() + pTexture->GetWidth() / 2, pTexture->GetY() + pTexture->GetHeight() / 2);
}

int Choice::KeyToIndex(const string& Key)
{
    if (Key == "UP") return FOCUS_UP;
    if (Key == "DOWN") return FOCUS_DOWN;
    if (Key == "RIGHT") return FOCUS_RIGHT;
    if (Key == "LEFT") return FOCUS_LEFT;
    assert(false);
}
