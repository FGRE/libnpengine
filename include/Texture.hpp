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

#include <SDL2/SDL_opengl.h>
#include <string>
using namespace std;

class Texture
{
public:
    Texture();
    virtual ~Texture();

    void LoadFromFile(const string& Filename);
    void SetPosition(int X, int Y);
    void SetSmoothing(bool Set = true);
    void Draw();
    void SetPriority(int Priority);
    int GetPriority();
    int GetWidth();
    int GetHeight();
    int GetX();
    int GetY();

private:
    void LoadPNG(uint8_t* pMem, uint32_t Size);
    void LoadJPEG(uint8_t* pMem, uint32_t Size);
    void Create(uint8_t* Pixels);

    int Priority;
    int X, Y;
    int Width, Height;
    GLuint GLTextureID;
};

#endif
