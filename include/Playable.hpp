/* 
 * libnpengine: Nitroplus script interpreter
 * Copyright (C) 2013-2014 Mislav Blažević <krofnica996@gmail.com>
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
#include "Object.hpp"
#include "ResourceMgr.hpp"

struct AppSrc
{
    AppSrc(Resource& Res) : File(Res) { }
    GstAppSrc* Appsrc;
    gsize Offset;
    Resource File;
};

class Playable : public Object
{
    friend void LinkPad(GstElement* DecodeBin, GstPad* SourcePad, gpointer Data);
public:
    Playable(const string& FileName);
    Playable(Resource Res);
    ~Playable();

    void SetVolume(int32_t Time, int32_t Volume);
    void SetFrequency(int32_t Time, int32_t Frequency);
    void SetPan(int32_t Time, int32_t Pan);
    void SetLoopPoint(int32_t Begin, int32_t End);
    void SetLoop(bool Loop);
    void Stop();
    void Play();
    int32_t RemainTime();
    int32_t DurationTime();
    void Request(const string& State);
    virtual void Delete(Window* pWindow);

    AppSrc* Appsrc;
protected:
    void InitAudio();
    void InitPipeline(GstElement* Source);

    GstElement* Pipeline;
private:
    bool Loop;
    GstElement* AudioBin;
    GstElement* VolumeFilter;
    gint64 Begin, End;
};

#endif
