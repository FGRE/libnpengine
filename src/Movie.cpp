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
#include "Movie.hpp"
#include "Window.hpp"

Movie::Movie(const string& FileName, Window* pWindow, int32_t Priority, bool Alpha, bool Audio) :
Playable(FileName),
Alpha(Alpha)
{
    SetPriority(Priority);
    InitVideo(pWindow);
    if (Audio)
        InitAudio();
}

Movie::~Movie()
{
}

void Movie::UpdateSample()
{
    GstSample* sample = gst_app_sink_pull_sample(Appsink);
    if (!sample)
        return;

    GstCaps* caps = gst_sample_get_caps(sample);
    if (!caps)
        return;

    GstStructure* s = gst_caps_get_structure(caps, 0);
    gint width, height;
    gst_structure_get_int(s, "width", &width);
    gst_structure_get_int(s, "height", &height);
    GstBuffer* buffer = gst_sample_get_buffer(sample);
    GstMapInfo map;
    if (gst_buffer_map(buffer, &map, GST_MAP_READ))
    {
        Create(map.data, GL_RGB, width, height);
        gst_buffer_unmap(buffer, &map);
    }
    gst_sample_unref(sample);
}

void Movie::Draw(uint32_t Diff)
{
    if (Playing) UpdateSample();
    Texture::Draw(Diff);
}

void Movie::InitVideo(Window* pWindow)
{
    VideoBin = gst_bin_new("videobin");
    GstElement* VideoConv = gst_element_factory_make("videoconvert", "vconv");
    GstPad* VideoPad = gst_element_get_static_pad(VideoConv, "sink");
    Appsink = (GstAppSink*)gst_element_factory_make("appsink", "sink");

    GstCaps* caps = gst_caps_from_string("video/x-raw,format=RGB");
    gst_app_sink_set_caps(Appsink, caps);
    gst_caps_unref(caps);

    gst_bin_add(GST_BIN(VideoBin), VideoConv);
    gst_bin_add(GST_BIN(VideoBin), (GstElement*)Appsink);

    gst_element_link_many(VideoConv, (GstElement*)Appsink, nullptr);

    gst_element_add_pad(VideoBin, gst_ghost_pad_new("sink", VideoPad));
    gst_object_unref(VideoPad);
    gst_bin_add(GST_BIN(Pipeline), VideoBin);
}
