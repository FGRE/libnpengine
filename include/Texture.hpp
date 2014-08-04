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
#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include "Object.hpp"
#include <SDL2/SDL_opengl.h>

class MoveEffect;
class ZoomEffect;
class FadeEffect;
class Texture : public Object
{
public:
    Texture();
    virtual ~Texture();

    void Request(const string& State);
    void LoadFromColor(int Width, int Height, uint32_t Color);
    void LoadFromFile(const string& Filename);
    void Draw(int X, int Y, const string& Filename);
    void SetPosition(int X, int Y);
    void SetVertex(int X, int Y);
    void Draw();
    void SetPriority(int Priority);
    void Move(int32_t Time, int X, int Y);
    void Zoom(int32_t Time, int X, int Y);
    void Fade(int32_t Time, int Opacity);

    int GetPriority() { return Priority; }
    int GetWidth() { return Width; }
    int GetHeight() { return Height; }
    int GetX() { return X; }
    int GetY() { return Y; }
    int GetOX() { return OX; }
    int GetOY() { return OY; }

private:
    void SetSmoothing(bool Set);
    uint8_t* LoadPixels(const string& Filename, int& Width, int& Height);
    uint8_t* LoadPNG(uint8_t* pMem, uint32_t Size, int& Width, int& Height);
    uint8_t* LoadJPEG(uint8_t* pMem, uint32_t Size, int& Width, int& Height);
    void Create(uint8_t* Pixels);

    MoveEffect* pMove;
    ZoomEffect* pZoom;
    FadeEffect* pFade;
    int Priority;
    int X, Y;
    int OX, OY;
    int Width, Height;
    GLuint GLTextureID;
};

#endif
