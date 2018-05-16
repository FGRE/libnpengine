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
#include "GLTexture.hpp"
#include "Image.hpp"
#include <cstring>

size_t GLFormatToVals(GLenum Format)
{
    switch (Format)
    {
        case GL_RGBA:
        case GL_BGRA:
            return 4;
        case GL_RGB:
        case GL_BGR:
            return 3;
        case GL_LUMINANCE:
            return 1;
    }
    assert(false);
}

GLTexture::GLTexture() :
Width(0), Height(0),
GLTextureID(GL_INVALID_VALUE)
{
}

GLTexture::~GLTexture()
{
    glDeleteTextures(1, &GLTextureID);
}

void GLTexture::Draw(int X, int Y, const string& Filename)
{
    Image Img;
    Img.LoadImage(Filename);
    glBindTexture(GL_TEXTURE_2D, GLTextureID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, X, Y, Img.GetWidth(), Img.GetHeight(), Img.GetFormat(), GL_UNSIGNED_BYTE, Img.GetPixels());
}

void GLTexture::Draw(float X, float Y, float Width, float Height)
{
    static const float x[4] = {0, X, X, 0}, y[4] = {0, 0, Y, Y};
    Draw(x, y);
}

void GLTexture::Draw(const float* xa, const float* ya)
{
    static const float x[4] = {0, 1, 1, 0}, y[4] = {0, 0, 1, 1};
    glBindTexture(GL_TEXTURE_2D, GLTextureID);
    glBegin(GL_QUADS);
    for (int i = 0; i < 4; ++i)
    {
        glTexCoord2f(x[i], y[i]);
        glVertex2f(xa[i], ya[i]);
    }
    glEnd();
}

void GLTexture::CreateFromScreen(Window* pWindow)
{
    Image Img;
    Img.LoadScreen(pWindow);
    CreateFromImage(&Img);
}

void GLTexture::CreateFromColor(int Width, int Height, uint32_t Color)
{
    Image Img;
    Img.LoadColor(Width, Height, Color);
    CreateFromImage(&Img);
}

void GLTexture::CreateFromFile(const string& Filename, bool Mask)
{
    Image Img;
    Img.LoadImage(Filename, Mask);
    CreateFromImage(&Img);
}

void GLTexture::CreateFromImage(Image* pImage)
{
    Create(pImage->GetPixels(), pImage->GetFormat(), pImage->GetWidth(), pImage->GetHeight());
}

void GLTexture::CreateFromImageClip(Image* pImage, int ClipX, int ClipY, int ClipWidth, int ClipHeight)
{
    size_t NumVals = GLFormatToVals(pImage->GetFormat());
    uint8_t* pClipped = new uint8_t[ClipWidth * ClipHeight * NumVals];
    for (int i = 0; i < ClipHeight; ++i)
        memcpy(pClipped + i * ClipWidth * NumVals, pImage->GetPixels() + (pImage->GetWidth() * (ClipY + i) + ClipX) * NumVals, ClipWidth * NumVals);

    Create(pClipped, pImage->GetFormat(), ClipWidth, ClipHeight);
    delete[] pClipped;
}

void GLTexture::CreateFromFileClip(const string& Filename, int ClipX, int ClipY, int ClipWidth, int ClipHeight)
{
    Image Img;
    Img.LoadImage(Filename);
    CreateFromImageClip(&Img, ClipX, ClipY, ClipWidth, ClipHeight);
}

void GLTexture::CreateEmpty(int Width, int Height)
{
    Create(0, GL_RGB, Width, Height);
}

void GLTexture::Create(uint8_t* Pixels, GLenum Format, int W, int H)
{
    Width = W;
    Height = H;
    glDeleteTextures(1, &GLTextureID);
    glGenTextures(1, &GLTextureID);
    glBindTexture(GL_TEXTURE_2D, GLTextureID);
    SetSmoothing(false);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, Format, GL_UNSIGNED_BYTE, Pixels);
}

void GLTexture::SetSmoothing(bool Set)
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Set ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Set ? GL_LINEAR : GL_NEAREST);
}
