/* 
 * libnpengine: Nitroplus script interpreter
 * Copyright (C) 2013-2016,2018 Mislav Blažević <krofnica996@gmail.com>
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
#ifndef MOVIE_HPP
#define MOVIE_HPP

#include "Playable.hpp"
#include "Texture.hpp"
#include <gst/app/gstappsink.h>

class Movie : public Playable, public Texture
{
    friend void LinkPad(GstElement* DecodeBin, GstPad* SourcePad, gpointer Data);
public:
    Movie(const string& FileName, Window* pWindow, int32_t Priority, bool Alpha, bool Audio);
    ~Movie();

    virtual void Request(int32_t State) { Playable::Request(State); }
    bool Action() { return true; }
    void Draw(uint32_t Diff);
private:
    void InitVideo(Window* pWindow);
    void UpdateSample();

    bool Alpha;
    GstElement* VideoBin;
    GstAppSink* Appsink;
};

#endif
