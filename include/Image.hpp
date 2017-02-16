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
#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <SDL2/SDL_opengl.h>
#include "Object.hpp"

class Image : public Object
{
public:
    Image();
    ~Image();

    int GetWidth() const { return Width; }
    int GetHeight() const { return Height; }
    uint8_t* GetPixels() const { return pPixels; }
    void LoadColor(int Width, int Height, uint32_t Color);
    GLenum LoadImage(const string& Filename, bool Mask = false);

private:
    uint8_t* LoadPNG(uint8_t* pMem, uint32_t Size, uint8_t Format);
    uint8_t* LoadJPEG(uint8_t* pMem, uint32_t Size);

    int Width, Height;
    uint8_t* pPixels;
};

#endif
