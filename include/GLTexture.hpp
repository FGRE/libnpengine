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
#ifndef GL_TEXTURE_HPP
#define GL_TEXTURE_HPP

#include <SDL2/SDL_opengl.h>
#include "Object.hpp"

class Image;
class Window;
class Texture;
class GLTexture : virtual public Object
{
    friend class Texture;
public:
    GLTexture();
    virtual ~GLTexture();

    void Draw(int X, int Y, const string& Filename);
    void Draw(float X, float Y, float Width, float Height);
    void Draw(const float* xa, const float* ya);

    void CreateFromScreen(Window* pWindow);
    void CreateFromImage(Image* pImage);
    void CreateFromImageClip(Image* pImage, int ClipX, int ClipY, int ClipWidth, int ClipHeight);
    void CreateFromColor(int Width, int Height, uint32_t Color);
    void CreateFromFile(const string& Filename, bool Mask = false);
    void CreateFromFileClip(const string& Filename, int ClipX, int ClipY, int ClipWidth, int ClipHeight);
    void CreateEmpty(int Width, int Height);
    void Create(uint8_t* Pixels, GLenum Format, int W, int H);

protected:
    void SetSmoothing(bool Set);

    int Width, Height;
    GLuint GLTextureID;
};

#endif
