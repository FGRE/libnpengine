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
#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include "Object.hpp"
#include "GLTexture.hpp"

class MoveEffect;
class ZoomEffect;
class FadeEffect;
class MaskEffect;
class BlurEffect;
class RotateEffect;
class Tone;
class Texture : public GLTexture
{
public:
    Texture();
    virtual ~Texture();

    void CreateFromGLTexture(GLTexture* pTexture);

    void Request(int32_t State);
    void SetPosition(int X, int Y);
    void SetAngle(int Angle);
    void SetScale(int XScale, int YScale);
    void SetVertex(int X, int Y);
    virtual void Draw(uint32_t Diff);
    void SetPriority(int Priority);
    void Move(int X, int Y, int32_t Time = 0);
    void Zoom(int32_t Time, int X, int Y);
    void Fade(int32_t Time, int Opacity);
    void DrawTransition(int32_t Time, int32_t Start, int32_t End, int32_t Boundary, const string& Filename);
    void SetShade(int32_t Shade);
    void SetTone(int32_t Tone);
    void Rotate(int32_t Angle, int32_t Time);
    void Shake(int32_t XWidth, int32_t YWidth, int32_t Time);

    int GetXScale() { return XScale; }
    int GetYScale() { return YScale; }
    int GetAngle() { return Angle; }
    int GetPriority() { return Priority; }
    int GetWidth() { return Width; }
    int GetHeight() { return Height; }
    int GetX() { return X; }
    int GetY() { return Y; }
    int GetOX() { return OX; }
    int GetOY() { return OY; }
    int32_t GetMX();
    int32_t GetMY();
    int32_t RemainFade();

private:
    MoveEffect* pMove;
    ZoomEffect* pZoom;
    FadeEffect* pFade;
    MaskEffect* pMask;
    BlurEffect* pBlur;
    RotateEffect* pRotate;
    Tone* pTone;
    int Priority;
    int X, Y;
    int OX, OY;
    int Angle;
    int XScale, YScale;
    int XShake, YShake, ShakeTime;
    bool ShakeTick;
};

#endif
