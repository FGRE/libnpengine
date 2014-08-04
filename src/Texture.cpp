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
#include "Effect.hpp"
#include "ResourceMgr.hpp"
#include <jpeglib.h>
#include <png.h>
#include <new>
#include <cstring>

Texture::Texture() : pMove(nullptr), pZoom(nullptr), pFade(nullptr), X(0), Y(0), OX(0), OY(0), Width(0), Height(0), GLTextureID(GL_INVALID_VALUE)
{
}

Texture::~Texture()
{
    glDeleteTextures(1, &GLTextureID);
    delete pMove;
    delete pZoom;
}

void Texture::Request(const string& State)
{
    if (State == "Smoothing")
        SetSmoothing(true);
}

void Texture::LoadFromFile(const string& Filename)
{
    uint8_t* pPixels = LoadPixels(Filename, Width, Height);
    Create(pPixels);
    delete[] pPixels;
    OX = Width / 2;
    OY = Height / 2;
}

void Texture::LoadFromColor(int Width, int Height, uint32_t Color)
{
    this->Width = Width;
    this->Height = Height;
    vector<uint32_t> Data(Width * Height, Color);
    Create((uint8_t*)&Data[0]);
    OX = Width / 2;
    OY = Height / 2;
}

void Texture::Draw(int X, int Y, const string& Filename)
{
    int Width, Height;
    uint8_t* pPixels = LoadPixels(Filename, Width, Height);
    glBindTexture(GL_TEXTURE_2D, GLTextureID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, X, Y, Width, Height, GL_BGRA, GL_UNSIGNED_BYTE, pPixels);
    delete[] pPixels;
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

void Texture::Move(int32_t Time, int X, int Y)
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

void Texture::Draw()
{
    float ScaleX = 1, ScaleY = 1;
    int32_t OffsetX = 0, OffsetY = 0;
    if (pMove) pMove->OnDraw(this, 16);
    if (pZoom) pZoom->OnDraw(this, 16, OffsetX, OffsetY, ScaleX, ScaleY);

    glBindTexture(GL_TEXTURE_2D, GLTextureID);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(X + OffsetX, Y + OffsetY);
        glTexCoord2f(1.0f, 0.0f);
        glVertex2f(X + OffsetX + Width * ScaleX, Y + OffsetY);
        glTexCoord2f(1.0f, 1.0f);
        glVertex2f(X + OffsetX + Width * ScaleX, Y + OffsetY + Height * ScaleY);
        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(X + OffsetX, Y + OffsetY + Height * ScaleY);
    glEnd();

    if (pFade) pFade->OnDraw(this, 16);
}

uint8_t* Texture::LoadPixels(const string& Filename, int& Width, int& Height)
{
    uint32_t Size;
    uint8_t* pData = (uint8_t*)sResourceMgr->Read(Filename, Size);
    if (!pData)
        return nullptr;

    uint8_t* pPixels = nullptr;
    if (Filename.substr(Filename.size() - 3) == "jpg")
        pPixels = LoadJPEG(pData, Size, Width, Height);
    else if (Filename.substr(Filename.size() - 3) == "png")
        pPixels = LoadPNG(pData, Size, Width, Height);

    delete[] pData;
    return pPixels;
}

uint8_t* Texture::LoadPNG(uint8_t* pMem, uint32_t Size, int& Width, int& Height)
{
    png_image png;
    memset(&png, 0, sizeof(png_image));
    png.version = PNG_IMAGE_VERSION;

    if (!png_image_begin_read_from_memory(&png, pMem, Size))
        return nullptr;

    uint8_t* pData = new (nothrow) uint8_t[PNG_IMAGE_SIZE(png)];
    if (!pData)
    {
        png_image_free(&png);
        return nullptr;
    }

    png.format = PNG_FORMAT_BGRA;
    if (!png_image_finish_read(&png, NULL, pData, 0, NULL))
    {
        delete[] pData;
        return nullptr;
    }

    Width = png.width;
    Height = png.height;
    return pData;
}

uint8_t* Texture::LoadJPEG(uint8_t* pMem, uint32_t Size, int& Width, int& Height)
{
    struct jpeg_decompress_struct jpeg;
    struct jpeg_error_mgr err;

    jpeg.err = jpeg_std_error(&err);
    jpeg_create_decompress(&jpeg);
    jpeg_mem_src(&jpeg, pMem, Size);
    jpeg_read_header(&jpeg, 1);
    jpeg_start_decompress(&jpeg);

    Width = jpeg.output_width;
    Height = jpeg.output_height;
    uint8_t* pData = new (nothrow) uint8_t[Width * Height * 4];
    if (!pData)
    {
        jpeg_abort_decompress(&jpeg);
        jpeg_destroy_decompress(&jpeg);
        return nullptr;
    }

    uint8_t* pRow = new (nothrow) uint8_t[Width * jpeg.output_components];
    if (!pRow)
    {
        delete[] pData;
        jpeg_abort_decompress(&jpeg);
        jpeg_destroy_decompress(&jpeg);
        return nullptr;
    }

    uint32_t* pPixel = (uint32_t*)pData;
    for (int y = 0; y < Height; ++y)
    {
        jpeg_read_scanlines(&jpeg, &pRow, 1);
        for (int x = 0; x < Width; ++x, ++pPixel)
            *pPixel = 0xFF << 24 | pRow[x * 3] << 16 | pRow[x * 3 + 1] << 8 | pRow[x * 3 + 2];
    }

    jpeg_finish_decompress(&jpeg);
    jpeg_destroy_decompress(&jpeg);
    delete[] pRow;
    return pData;
}

void Texture::Create(uint8_t* Pixels)
{
    glGenTextures(1, &GLTextureID);
    glBindTexture(GL_TEXTURE_2D, GLTextureID);
    SetSmoothing(false);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_BGRA, GL_UNSIGNED_BYTE, Pixels);
}

void Texture::SetSmoothing(bool Set)
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Set ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Set ? GL_LINEAR : GL_NEAREST);
}
