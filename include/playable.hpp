/* 
 * libnpengine: Nitroplus script interpreter
 * Copyright (C) 2013 Mislav Blažević <krofnica996@gmail.com>
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
#ifndef PLAYABLE_HPP
#define PLAYABLE_HPP

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include "npaiterator.hpp"

struct AppSrc
{
    GstAppSrc* Appsrc;
    NpaIterator File;
    gsize Offset;
};

class Playable
{
    friend void LinkPad(GstElement* DecodeBin, GstPad* SourcePad, gpointer Data);
public:
    Playable(const std::string& FileName);
    Playable(NpaIterator File);
    ~Playable();

    void Update();
    void SetVolume(double Volume);
    void SetRange(int32_t Begin, int32_t End);
    void SetLoop(bool Loop);
    void Stop();
    void Play();
    int32_t GetDuration();

    AppSrc* Appsrc;
protected:
    void InitAudio();
    void InitPipeline(GstElement* Source);

    GstElement* Pipeline;
private:
    GstElement* AudioBin;
    GstElement* VolumeFilter;
    bool Loop;
    gint64 Begin, End;
};

#endif
