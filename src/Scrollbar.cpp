/* 
 * libnpengine: Nitroplus script interpreter
 * Copyright (C) 2018 Mislav Blažević <krofnica996@gmail.com>
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
#include "Scrollbar.hpp"
#include "Window.hpp"
#include "Texture.hpp"

Scrollbar::Scrollbar(Texture* pTexture, int32_t X1, int32_t Y1, int32_t X2, int32_t Y2, int32_t Min, int32_t Max, string Type, string Callback) :
pTexture(pTexture), X1(X1), Y1(Y1), X2(X2), Y2(Y2), Min(Min), Max(Max), Type(Type), Callback(Callback)
{
}

Scrollbar::~Scrollbar()
{
    delete pTexture;
    pWindow->RemoveTexture(pTexture);
}

void Scrollbar::SetWheelArea(int32_t X, int32_t Y, int32_t Width, int32_t Height)
{
    WX = X;
    WY = Y;
    WWidth = Width;
    WHeight = Height;
}

void Scrollbar::SetValue(int32_t NewValue)
{
    Value = NewValue;
}

int32_t Scrollbar::GetValue()
{
    return Value;
}
