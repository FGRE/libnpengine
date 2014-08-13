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
#ifndef EFFECT_HPP
#define EFFECT_HPP

#include <GL/glew.h>
#include <png.h>
#include "Texture.hpp"

class Effect
{
public:
    Effect() : Program(0) { }
    ~Effect() { glDeleteObjectARB(Program); }

protected:
    int32_t Lerp(int32_t Old, int32_t New, float Progress)
    {
        if (New > Old)
            return Old + (New - Old) * Progress;
        else
            return Old - (Old - New) * Progress;
    }

    void CompileShader(const char* String)
    {
        if (!GLEW_ARB_fragment_shader)
            return;

        GLuint Shader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
        glShaderSourceARB(Shader, 1, &String, NULL);
        glCompileShaderARB(Shader);

        Program = glCreateProgramObjectARB();
        glAttachObjectARB(Program, Shader);

        glLinkProgramARB(Program);
        glDeleteObjectARB(Shader);
    }

    GLuint Program;
};

class LerpEffect : public Effect
{
public:
    LerpEffect() { }
    LerpEffect(int32_t StartX, int32_t StartY) : EndX(StartX), EndY(StartY) { }

    void Reset(int32_t StartX, int32_t EndX, int32_t StartY, int32_t EndY, int32_t Time)
    {
        this->StartX = StartX;
        this->StartY = StartY;
        this->EndX = EndX;
        this->EndY = EndY;
        this->Time = Time;
        this->ElapsedTime = 0;
    }

    void Reset(int32_t EndX, int32_t EndY, int32_t Time)
    {
        Reset(this->EndX, EndX, this->EndY, EndY, Time);
    }

protected:
    float GetProgress()
    {
        if (ElapsedTime >= Time)
            return 1.0f;
        return float(ElapsedTime) / float(Time);
    }

    void Update(int32_t diff, int32_t& x, int32_t& y)
    {
        ElapsedTime += diff;
        x = Lerp(StartX, EndX, GetProgress());
        y = Lerp(StartY, EndY, GetProgress());
    }

    int32_t StartX, StartY;
    int32_t EndX, EndY;
    int32_t ElapsedTime;
    int32_t Time;
};

class MoveEffect : public LerpEffect
{
public:
    MoveEffect(int32_t EndX, int32_t EndY, int32_t Time) : LerpEffect(0, 0)
    {
        Reset(EndX, EndY, Time);
    }

    void OnDraw(Texture* pTexture, int32_t diff)
    {
        int32_t x, y;
        Update(diff, x, y);
        pTexture->SetPosition(x, y);
    }
};

class ZoomEffect : public LerpEffect
{
public:
    ZoomEffect(int32_t EndX, int32_t EndY, int32_t Time) : LerpEffect(1000, 1000)
    {
        Reset(EndX, EndY, Time);
    }

    void OnDraw(Texture* pTexture, int32_t diff, int32_t& OffsetX, int32_t& OffsetY, float& ScaleX, float& ScaleY)
    {
        int32_t x, y;
        Update(diff, x, y);
        ScaleX = x / 1000.f;
        ScaleY = y / 1000.f;
        OffsetX = pTexture->GetWidth() * ScaleX - pTexture->GetWidth();
        OffsetX /= -(float(pTexture->GetWidth()) / float(pTexture->GetOX()));
        OffsetY = pTexture->GetHeight() * ScaleY - pTexture->GetHeight();
        OffsetY /= -(float(pTexture->GetHeight()) / float(pTexture->GetOY()));
    }
};

class FadeEffect : public LerpEffect
{
    const string MaskShader = \
        "uniform sampler2D Texture;"
        "uniform float Alpha;"
        "void main()"
        "{"
        "   vec4 Pixel = texture2D(Texture, gl_TexCoord[0].xy);"
        "   Pixel.a = Alpha;"
        "   gl_FragColor = Pixel;"
        "}";
public:
    FadeEffect()
    {
    }

    FadeEffect(int32_t EndOpacity, int32_t Time) : LerpEffect(1000, 0)
    {
        CompileShader(MaskShader.c_str());
        Reset(EndOpacity, 0, Time);
    }

    void OnDraw(int32_t diff)
    {
        int32_t x, y;
        Update(diff, x, y);

        if (!Program)
            return;

        glUseProgramObjectARB(Program);
        glUniform1fARB(glGetUniformLocationARB(Program, "Alpha"), x * 0.001f);
        glUniform1iARB(glGetUniformLocationARB(Program, "Texture"), 0);
    }
};

// TODO: Maybe apply on texture instead of drawing it separately
class MaskEffect : public FadeEffect, GLTexture
{
    const string MaskShader = \
        "uniform sampler2D Texture;"
        "uniform float Alpha;"
        "void main()"
        "{"
        "   vec4 Pixel = texture2D(Texture, gl_TexCoord[0].xy);"
        "   vec4 Black = vec4(0, 0, 0, 1.0f - Pixel.r);"
        "   gl_FragColor = Black;"
        "}";
public:
    MaskEffect(const string& Filename, int32_t StartOpacity, int32_t EndOpacity, int32_t Time)
    {
        CompileShader(MaskShader.c_str());
        Reset(Filename, StartOpacity, EndOpacity, Time);
    }

    void Reset(const string& Filename, int32_t StartOpacity, int32_t EndOpacity, int32_t Time)
    {
        uint8_t* pPixels = LoadPixels(Filename, Width, Height, PNG_COLOR_TYPE_GRAY);
        Create(pPixels, GL_LUMINANCE);
        delete[] pPixels;
        LerpEffect::Reset(StartOpacity, EndOpacity, 0, 0, Time);
    }

    void OnDraw(int32_t diff)
    {
        FadeEffect::OnDraw(diff);
        Draw(0, 0, Width, Height);
    }
};

class BlurEffect : public Effect
{
public:
    void OnDraw()
    {
    }
};

#endif
