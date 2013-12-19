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
#include "movie.hpp"
#include "game.hpp"
#include <gst/video/videooverlay.h>

static GstBusSyncReply CreateWindow(GstBus* bus, GstMessage* msg, gpointer Handle)
{
    if (!gst_is_video_overlay_prepare_window_handle_message(msg))
        return GST_BUS_PASS;

    gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(GST_MESSAGE_SRC(msg)), (sf::WindowHandle)Handle);
    gst_message_unref(msg);
    return GST_BUS_DROP;
}

Movie::Movie(const std::string& FileName, sf::WindowHandle Handle, int32_t Priority, bool Alpha, bool Audio, sf::IntRect* pBox) :
Playable(FileName),
Priority(Priority)
{
    GstBus* Bus = gst_pipeline_get_bus(GST_PIPELINE(Pipeline));
    gst_bus_set_sync_handler(Bus, (GstBusSyncHandler)CreateWindow, (gpointer)Handle, NULL);
    gst_object_unref(Bus);

    VideoBin = gst_bin_new("videobin");
    GstElement* VideoConv = gst_element_factory_make("videoconvert", "vconv");
    GstPad* VideoPad = gst_element_get_static_pad(VideoConv, "sink");
    GstElement* VideoSink = gst_element_factory_make("autovideosink", "sink");

    GstElement* VideoBox = nullptr;
    if (pBox)
    {
        gint top = pBox->top;
        gint bottom = WINDOW_HEIGHT - top - pBox->height;
        gint left = pBox->left;
        gint right = WINDOW_WIDTH - left - pBox->width;
        VideoBox = gst_element_factory_make("videobox", "vfilter");
        g_object_set(G_OBJECT(VideoBox), "bottom", bottom, "top", top, "left", left, "right", right, NULL);
    }

    gst_bin_add(GST_BIN(VideoBin), VideoConv);
    if (VideoBox)
        gst_bin_add(GST_BIN(VideoBin), VideoBox);
    gst_bin_add(GST_BIN(VideoBin), VideoSink);

    gst_element_link_many(VideoConv, VideoBox ? VideoBox : VideoSink, VideoBox ? VideoSink : nullptr, nullptr);

    gst_element_add_pad(VideoBin, gst_ghost_pad_new("sink", VideoPad));
    gst_object_unref(VideoPad);
    gst_bin_add(GST_BIN(Pipeline), VideoBin);

    if (Audio)
        InitAudio();
}

Movie::~Movie()
{
}

