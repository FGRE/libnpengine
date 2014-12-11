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
#include "GLTexture.hpp"
#include "Image.hpp"
#include <cstring>

GLTexture::GLTexture() :
Width(0), Height(0),
GLTextureID(GL_INVALID_VALUE)
{
}

GLTexture::~GLTexture()
{
    glDeleteTextures(1, &GLTextureID);
}

void GLTexture::Draw(float X, float Y, float Width, float Height)
{
    glBindTexture(GL_TEXTURE_2D, GLTextureID);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(X, Y);
        glTexCoord2f(1.0f, 0.0f);
        glVertex2f(X + Width, Y);
        glTexCoord2f(1.0f, 1.0f);
        glVertex2f(X + Width, Y + Height);
        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(X, Y + Height);
    glEnd();
}

void GLTexture::CreateFromColor(int Width, int Height, uint32_t Color)
{
    Image Img;
    Img.LoadColor(Width, Height, Color);
    this->Width = Width;
    this->Height = Height;
    Create(Img.GetPixels(), GL_BGRA);
}

void GLTexture::CreateFromFile(const string& Filename, GLenum Format)
{
    Image Img;
    Img.LoadImage(Filename, Format);
    Width = Img.GetWidth();
    Height = Img.GetHeight();
    Create(Img.GetPixels(), Format);
}

void GLTexture::CreateFromImage(Image* pImage)
{
    Width = pImage->GetWidth();
    Height = pImage->GetHeight();
    Create(pImage->GetPixels(), GL_BGRA);
}

void GLTexture::CreateFromImageClip(Image* pImage, int ClipX, int ClipY, int ClipWidth, int ClipHeight)
{
    uint8_t* pClipped = new uint8_t[ClipWidth * ClipHeight * 4];
    for (int i = 0; i < ClipHeight; ++i)
        memcpy(pClipped + i * ClipWidth * 4, pImage->GetPixels() + (pImage->GetWidth() * (ClipY + i) + ClipX) * 4, ClipWidth * 4);

    Width = ClipWidth;
    Height = ClipHeight;
    Create(pClipped, GL_BGRA);
    delete[] pClipped;
}

void GLTexture::CreateFromFileClip(const string& Filename, int ClipX, int ClipY, int ClipWidth, int ClipHeight)
{
    Image Img;
    Img.LoadImage(Filename, GL_BGRA);
    CreateFromImageClip(&Img, ClipX, ClipY, ClipWidth, ClipHeight);
}

void GLTexture::Create(uint8_t* Pixels, GLenum Format)
{
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
