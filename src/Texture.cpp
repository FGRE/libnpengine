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

Texture::Texture() :
pMove(nullptr),
pZoom(nullptr),
pFade(nullptr),
pMask(nullptr),
pBlur(nullptr),
pRotate(nullptr),
pTone(nullptr),
X(0), Y(0), OX(0), OY(0),
Angle(0),
XScale(1000), YScale(1000),
XShake(0), YShake(0), ShakeTime(0), ShakeTick(false)
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
    switch (State)
    {
        case Nsb::SMOOTHING:
            SetSmoothing(true);
            break;
        case Nsb::ERASE:
            pWindow->RemoveTexture(this);
            break;
        case Nsb::ENTER:
            pWindow->AddTexture(this);
            break;
    }
}

void Texture::CreateFromGLTexture(GLTexture* pTexture)
{
    GLTextureID = pTexture->GLTextureID;
    Width = pTexture->Width;
    Height = pTexture->Height;
}

void Texture::SetPosition(int X, int Y)
{
    this->X = X;
    this->Y = Y;
}

void Texture::SetAngle(int Angle)
{
    this->Angle = Angle;
}

void Texture::SetScale(int XScale, int YScale)
{
    this->XScale = XScale;
    this->YScale = YScale;
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

void Texture::SetTone(int32_t Tonei)
{
    delete pTone;
    pTone = new Tone(Tonei);
}

void Texture::Rotate(int32_t Angle, int32_t Time)
{
    if (!pRotate)
        pRotate = new RotateEffect(Angle, Time);
    else
        pRotate->Reset(Angle, 0, Time);
}

void Texture::Shake(int32_t XWidth, int32_t YWidth, int32_t Time)
{
    XShake = XWidth;
    YShake = YWidth;
    ShakeTime = Time;
}

void Texture::Draw(uint32_t Diff)
{
    if (pMove) pMove->OnDraw(this, Diff);
    if (pRotate) pRotate->OnDraw(this, Diff);
    if (pZoom) pZoom->OnDraw(this, Diff);
    if (pFade) pFade->OnDraw(Diff);
    if (pMask) pMask->OnDraw(Diff);
    if (pTone) pTone->OnDraw();
    ShakeTime = max(0, ShakeTime - (int32_t)Diff);
    ShakeTick = ShakeTime ? !ShakeTick : false;

    float sx = XScale / 1000.f;
    float sy = YScale / 1000.f;
    float ox = OX * (1.f - sx);
    float oy = OY * (1.f - sy);
    float s = sin(Angle * M_PI / 180.);
    float c = cos(Angle * M_PI / 180.);
    float XA[4] = {X, X + Width * sx, X + Width * sx, X};
    float YA[4] = {Y, Y, Y + Height * sy, Y + Height * sy};
    for (int i = 0; i < 4; ++i)
    {
        XA[i] -= X + OX * sx;
        YA[i] -= Y + OY * sy;
        float xn = XA[i] * c - YA[i] * s;
        float yn = XA[i] * s + YA[i] * c;
        XA[i] = xn + X + OX * sx + ox;
        YA[i] = yn + Y + OY * sy + oy;
        XA[i] += ShakeTick * XShake;
        YA[i] += ShakeTick * YShake;
    }

    if (pBlur) pBlur->OnDraw(this, XA, YA, Width * sx, Height * sy);
    else GLTexture::Draw(XA, YA);

    if (glUseProgramObjectARB)
        glUseProgramObjectARB(0);
}

int32_t Texture::GetMX()
{
    return pMove ? pMove->EndX : 0;
}

int32_t Texture::GetMY()
{
    return pMove ? pMove->EndY : 0;
}

int32_t Texture::RemainFade()
{
    return max(0, pFade->Time - pFade->ElapsedTime);
}
