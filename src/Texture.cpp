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
#include "Effect.hpp"
#include "Window.hpp"
#include "Image.hpp"
#include "nsbconstants.hpp"

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
    pWindow->RemoveTexture(this);
}

void Texture::Request(int32_t State)
{
    if (State == Nsb::SMOOTHING)
        SetSmoothing(true);
}

void Texture::Draw(int X, int Y, const string& Filename)
{
    Image Img;
    Img.LoadImage(Filename);
    glBindTexture(GL_TEXTURE_2D, GLTextureID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, X, Y, Img.GetWidth(), Img.GetHeight(), GL_RGBA, GL_UNSIGNED_BYTE, Img.GetPixels());
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

void Texture::Move(int X, int Y, int32_t Time)
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

void Texture::SetShade(int32_t Shade)
{
    /* TODO: These are wrong */
    static const float Sigma[] =
    {
        0.5, 1, 1.5, 2, 2.5, 3, 3.5
    };
    delete pBlur;
    pBlur = new BlurEffect;
    if (!pBlur->Create(Width, Height, Sigma[Shade]))
    {
        delete pBlur;
        pBlur = nullptr;
    }
}

void Texture::SetTone(int32_t Tone)
{
}

void Texture::Draw(uint32_t Diff)
{
    float ScaleX = 1, ScaleY = 1;
    int32_t OffsetX = 0, OffsetY = 0;

    if (pMove) pMove->OnDraw(this, Diff);
    if (pZoom) pZoom->OnDraw(this, Diff, OffsetX, OffsetY, ScaleX, ScaleY);
    if (pFade) pFade->OnDraw(Diff);
    if (pMask) pMask->OnDraw(Diff);

    if (pBlur) pBlur->OnDraw(this, X + OffsetX, Y + OffsetY, Width * ScaleX, Height * ScaleY);
    else GLTexture::Draw(X + OffsetX, Y + OffsetY, Width * ScaleX, Height * ScaleY);

    if (glUseProgramObjectARB)
        glUseProgramObjectARB(0);
}

int32_t Texture::GetMX()
{
    return pMove->EndX;
}

int32_t Texture::GetMY()
{
    return pMove->EndY;
}
