/* 
 * libnpengine: Nitroplus script interpreter
 * Copyright (C) 2014-2016 Mislav Blažević <krofnica996@gmail.com>
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
class Texture : public Object, public GLTexture
{
public:
    Texture();
    virtual ~Texture();

    void Request(int32_t State);
    void Draw(int X, int Y, const string& Filename);
    void SetPosition(int X, int Y);
    void SetVertex(int X, int Y);
    void Draw(uint32_t Diff);
    void SetPriority(int Priority);
    void Move(int X, int Y, int32_t Time = 0);
    void Zoom(int32_t Time, int X, int Y);
    void Fade(int32_t Time, int Opacity);
    void DrawTransition(int32_t Time, int32_t Start, int32_t End, int32_t Boundary, const string& Filename);
    void SetShade(int32_t Shade);
    void SetTone(int32_t Tone);

    int GetPriority() { return Priority; }
    int GetWidth() { return Width; }
    int GetHeight() { return Height; }
    int GetX() { return X; }
    int GetY() { return Y; }
    int GetOX() { return OX; }
    int GetOY() { return OY; }
    int32_t GetMX();
    int32_t GetMY();

private:
    MoveEffect* pMove;
    ZoomEffect* pZoom;
    FadeEffect* pFade;
    MaskEffect* pMask;
    BlurEffect* pBlur;
    int Priority;
    int X, Y;
    int OX, OY;
};

#endif
