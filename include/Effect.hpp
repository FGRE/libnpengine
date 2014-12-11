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
        "   Pixel.a *= Alpha;"
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

class MaskEffect : public FadeEffect, GLTexture
{
    const string MaskShader = \
        "uniform sampler2D Texture;"
        "uniform sampler2D Mask;"
        "uniform float Alpha;"
        "uniform float Boundary;"
        "void main()"
        "{"
        "   vec4 Pixel = texture2D(Texture, gl_TexCoord[0].xy);"
        "   vec4 MaskPixel = texture2D(Mask, gl_TexCoord[0].xy);"
        "   if (MaskPixel.r - Alpha <= 0.0f) Pixel.a = 1.0f;"
        "   else if (MaskPixel.r - Alpha - Boundary <= 0.0f) Pixel.a = -(MaskPixel.r - Alpha - Boundary) / Boundary;"
        "   else Pixel.a = 0.0f;"
        "   gl_FragColor = Pixel;"
        "}";
public:
    MaskEffect(const string& Filename, int32_t StartOpacity, int32_t EndOpacity, int32_t Time, int32_t Boundary)
    {
        CompileShader(MaskShader.c_str());
        Reset(Filename, StartOpacity, EndOpacity, Time, Boundary);
    }

    void Reset(const string& Filename, int32_t StartOpacity, int32_t EndOpacity, int32_t Time, int32_t Boundary)
    {
        CreateFromFile(Filename, GL_LUMINANCE);
        LerpEffect::Reset(StartOpacity, EndOpacity, 0, 0, Time);

        if (!Program)
            return;

        glUseProgramObjectARB(Program);
        glActiveTextureARB(GL_TEXTURE1_ARB);
        glBindTexture(GL_TEXTURE_2D, GLTextureID);
        glUniform1iARB(glGetUniformLocationARB(Program, "Mask"), 1);
        glActiveTextureARB(GL_TEXTURE0_ARB);
        glUniform1fARB(glGetUniformLocationARB(Program, "Boundary"), Boundary * 0.001f);
    }
};

class BlurEffect : public Effect
{
    const string BlurShader = \
        "uniform float Sigma;"
        "uniform sampler2D Texture;"
        "uniform vec2 Pass;"
        "uniform float BlurSize;"
        "const float NumSamples = 4.0f;"
        "const float PI = 3.14159265f;"
        "void main()"
        "{"
        "   vec3 Gaussian = vec3(1.0f / (sqrt(2.0f * PI) * Sigma), exp(-0.5f / (Sigma * Sigma)), 0.0f);"
        "   Gaussian.z = Gaussian.y * Gaussian.y;"
        "   vec4 Average = texture2D(Texture, gl_TexCoord[0].xy) * Gaussian.x;"
        "   float CoeffSum = Gaussian.x;"
        "   Gaussian.xy *= Gaussian.yz;"
        "   for (float i = 1.0f; i <= NumSamples; ++i)"
        "   {"
        "       Average += texture2D(Texture, gl_TexCoord[0].xy + i * BlurSize * Pass) * Gaussian.x;"
        "       Average += texture2D(Texture, gl_TexCoord[0].xy - i * BlurSize * Pass) * Gaussian.x;"
        "       CoeffSum += 2.0f * Gaussian.x;"
        "       Gaussian.xy *= Gaussian.yz;"
        "   }"
        "   gl_FragColor = Average / CoeffSum;"
        "}";
public:
    BlurEffect(const string& Heaviness)
    {
        CompileShader(BlurShader.c_str());

        if (!Program)
            return;

        glUseProgramObjectARB(Program);
        glUniform1fARB(glGetUniformLocationARB(Program, "Sigma"), 3.0f); // Guess for SEMIHEAVY
        glUniform1iARB(glGetUniformLocationARB(Program, "Texture"), 0);
    }

    void OnDraw(GLTexture* pTexture, float X, float Y, float Width, float Height)
    {
        if (!Program)
            return;

        glUseProgramObjectARB(Program);

        glUniform1fARB(glGetUniformLocationARB(Program, "BlurSize"), 1.0f / Width);
        glUniform2fARB(glGetUniformLocationARB(Program, "Pass"), 1.0f, 0.0f);
        pTexture->Draw(X, Y, Width, Height);
        // TODO: Multipass doesn't work
        glUniform1fARB(glGetUniformLocationARB(Program, "BlurSize"), 1.0f / Height);
        glUniform2fARB(glGetUniformLocationARB(Program, "Pass"), 0.0f, 1.0f);
        pTexture->Draw(X, Y, Width, Height);
    }
};

#endif
