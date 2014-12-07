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
#include "Effect.hpp"
#include "Window.hpp"
#include "nsbconstants.hpp"
#include <vector>

Texture::Texture() :
pMove(nullptr),
pZoom(nullptr),
pFade(nullptr),
pMask(nullptr),
pBlur(nullptr),
X(0), Y(0), OX(0), OY(0)
{
}

Texture::~Texture()
{
    delete pMove;
    delete pZoom;
    delete pFade;
    delete pMask;
    delete pBlur;
}

void Texture::Delete(Window* pWindow)
{
    pWindow->RemoveTexture(this);
}

void Texture::Request(int32_t State)
{
    if (State == Nsb::SMOOTHING)
        SetSmoothing(true);
}

void Texture::LoadFromFile(const string& Filename)
{
    uint8_t* pPixels = LoadPixels(Filename, Width, Height, PNG_FORMAT_BGRA);
    Create(pPixels, GL_BGRA);
    delete[] pPixels;
    OX = Width / 2;
    OY = Height / 2;
}

void Texture::LoadFromColor(int Width, int Height, uint32_t Color)
{
    this->Width = Width;
    this->Height = Height;
    vector<uint32_t> Data(Width * Height, Color);
    Create((uint8_t*)&Data[0], GL_BGRA);
    OX = Width / 2;
    OY = Height / 2;
}

void Texture::Draw(int X, int Y, const string& Filename)
{
    int Width, Height;
    uint8_t* pPixels = LoadPixels(Filename, Width, Height, PNG_FORMAT_BGRA);
    glBindTexture(GL_TEXTURE_2D, GLTextureID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, X, Y, Width, Height, GL_BGRA, GL_UNSIGNED_BYTE, pPixels);
    delete[] pPixels;
}

void Texture::SetPosition(int X, int Y)
{
    this->X = X;
    this->Y = Y;
}

void Texture::SetVertex(int X, int Y)
{
    OX = X;
    OY = Y;
}

void Texture::SetPriority(int Priority)
{
    this->Priority = Priority;
}

void Texture::Move(int32_t Time, int X, int Y)
{
    if (!pMove)
        pMove = new MoveEffect(X, Y, Time);
    else
        pMove->Reset(X, Y, Time);
}

void Texture::Zoom(int32_t Time, int X, int Y)
{
    if (!pZoom)
        pZoom = new ZoomEffect(X, Y, Time);
    else
        pZoom->Reset(X, Y, Time);
}

void Texture::Fade(int32_t Time, int Opacity)
{
    if (!pFade)
        pFade = new FadeEffect(Opacity, Time);
    else
        pFade->Reset(Opacity, 0, Time);
}

void Texture::DrawTransition(int32_t Time, int32_t Start, int32_t End, int32_t Boundary, const string& Filename)
{
    if (!pMask)
        pMask = new MaskEffect(Filename, Start, End, Time, Boundary);
    else
        pMask->Reset(Filename, Start, End, Time, Boundary);
}

void Texture::ApplyBlur(const string& Heaviness)
{
    if (!pBlur)
        pBlur = new BlurEffect(Heaviness);
}

void Texture::Draw()
{
    float ScaleX = 1, ScaleY = 1;
    int32_t OffsetX = 0, OffsetY = 0;

    if (pMove) pMove->OnDraw(this, 16);
    if (pZoom) pZoom->OnDraw(this, 16, OffsetX, OffsetY, ScaleX, ScaleY);
    if (pFade) pFade->OnDraw(16);
    if (pMask) pMask->OnDraw(16);

    GLTexture::Draw(X + OffsetX, Y + OffsetY, Width * ScaleX, Height * ScaleY);

    if (pBlur) pBlur->OnDraw(this, X + OffsetX, Y + OffsetY, Width * ScaleX, Height * ScaleY);

    if (glUseProgramObjectARB)
        glUseProgramObjectARB(0);
}
