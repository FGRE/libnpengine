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
#include "Movie.hpp"
#include "nsbconstants.hpp"
#include <gst/video/videooverlay.h>
#include <thread>

GstBusSyncReply SyncHandler(GstBus* bus, GstMessage* msg, gpointer Handle)
{
    if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_EOS)
        ((Playable*)Handle)->OnEOS();
    else if (gst_is_video_overlay_prepare_window_handle_message(msg))
        gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(GST_MESSAGE_SRC(msg)), (guintptr)((Movie*)Handle)->XWin);
    else
        return GST_BUS_PASS;

    gst_message_unref(msg);
    return GST_BUS_DROP;
}

void LinkPad(GstElement* DecodeBin, GstPad* SourcePad, gpointer Data)
{
    GstCaps* Caps = gst_pad_query_caps(SourcePad, NULL);
    GstStructure* Struct = gst_caps_get_structure(Caps, 0);

    GstPad* SinkPad;
    if (g_strrstr(gst_structure_get_name(Struct), "video"))
        SinkPad = gst_element_get_static_pad(((Movie*)Data)->VideoBin, "sink");
    else if (g_strrstr(gst_structure_get_name(Struct), "audio"))
        SinkPad = gst_element_get_static_pad(((Playable*)Data)->AudioBin, "sink");
    else
        SinkPad = nullptr;

    if (SinkPad && !GST_PAD_IS_LINKED(SinkPad))
        gst_pad_link(SourcePad, SinkPad);

    g_object_unref(SinkPad);
    gst_caps_unref(Caps);
}

static void FeedData(GstElement* Pipeline, guint size, AppSrc* pAppsrc)
{
    GstFlowReturn ret;
    gsize Size = 4096;
    if (pAppsrc->Offset + Size > pAppsrc->File.GetSize())
    {
        if (pAppsrc->Offset >= pAppsrc->File.GetSize())
        {
            g_signal_emit_by_name(pAppsrc->Appsrc, "end-of-stream", &ret);
            return;
        }
        Size = pAppsrc->File.GetSize() - pAppsrc->Offset;
    }
    char* pData = pAppsrc->File.ReadData(pAppsrc->Offset, Size);
    GstBuffer* Buffer = gst_buffer_new_wrapped(pData, Size);
    g_signal_emit_by_name(pAppsrc->Appsrc, "push-buffer", Buffer, &ret);
    gst_buffer_unref(Buffer);
    pAppsrc->Offset += Size;
}

Playable::Playable(const std::string& FileName) :
Appsrc(nullptr),
Loop(false),
AudioBin(nullptr),
Begin(0),
End(0)
{
    GstElement* Filesrc = gst_element_factory_make("filesrc", "source");
    g_object_set(G_OBJECT(Filesrc), "location", FileName.c_str(), NULL);
    InitPipeline(Filesrc);
}

Playable::Playable(Resource Res) :
Loop(false),
Begin(0)
{
    Appsrc = new AppSrc(Res);
    Appsrc->Appsrc = (GstAppSrc*)gst_element_factory_make("appsrc", "source");
    Appsrc->Offset = 0;
    gst_app_src_set_size(Appsrc->Appsrc, Res.GetSize());
    g_signal_connect(Appsrc->Appsrc, "need-data", G_CALLBACK(FeedData), Appsrc);
    InitPipeline((GstElement*)Appsrc->Appsrc);
    InitAudio();
}

Playable::~Playable()
{
    Stop();
    gst_object_unref(GST_OBJECT(Pipeline));
    delete Appsrc;
}

void Playable::InitPipeline(GstElement* Source)
{
    Pipeline = gst_pipeline_new("pipeline");
    GstElement* Decodebin = gst_element_factory_make("decodebin", "decoder");
    g_signal_connect(Decodebin, "pad-added", G_CALLBACK(LinkPad), this);
    gst_bin_add_many(GST_BIN(Pipeline), Source, Decodebin, NULL);
    gst_element_link(Source, Decodebin);

    // Set sync handler
    GstBus* Bus = gst_pipeline_get_bus(GST_PIPELINE(Pipeline));
    gst_bus_set_sync_handler(Bus, (GstBusSyncHandler)SyncHandler, this, NULL);
    gst_object_unref(Bus);
}

void Playable::InitAudio()
{
    AudioBin = gst_bin_new("audiobin");
    GstElement* AudioConv = gst_element_factory_make("audioconvert", "aconv");
    GstPad* AudioPad = gst_element_get_static_pad(AudioConv, "sink");
    GstElement* AudioSink = gst_element_factory_make("autoaudiosink", "sink");
    VolumeFilter = gst_element_factory_make("volume", "afilter");
    gst_bin_add_many(GST_BIN(AudioBin), AudioConv, VolumeFilter, AudioSink, NULL);
    gst_element_link_many(AudioConv, VolumeFilter, AudioSink, NULL);
    gst_element_add_pad(AudioBin, gst_ghost_pad_new("sink", AudioPad));
    gst_object_unref(AudioPad);
    gst_bin_add(GST_BIN(Pipeline), AudioBin);
}

int32_t Playable::RemainTime()
{
    gint64 Length, Position;
    gst_element_get_state(Pipeline, NULL, NULL, GST_CLOCK_TIME_NONE);
    gst_element_query_duration(Pipeline, GST_FORMAT_TIME, &Length);
    gst_element_query_position(Pipeline, GST_FORMAT_TIME, &Position);
    return (Length - Position) / GST_MSECOND;
}

int32_t Playable::DurationTime()
{
    gint64 Length;
    gst_element_get_state(Pipeline, NULL, NULL, GST_CLOCK_TIME_NONE);
    gst_element_query_duration(Pipeline, GST_FORMAT_TIME, &Length);
    return Length / GST_MSECOND;
}

void Playable::SetLoop(bool Loop)
{
    this->Loop = Loop;
}

void Playable::Stop()
{
    gst_element_set_state(Pipeline, GST_STATE_NULL);
}

void Playable::Play()
{
    gst_element_set_state(Pipeline, GST_STATE_PLAYING);
    if (gst_element_get_state(Pipeline, NULL, NULL, GST_SECOND) == GST_STATE_CHANGE_SUCCESS)
        gst_element_seek_simple(Pipeline, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH, Begin);
}

void Playable::SetVolume(int32_t Time, int32_t Volume)
{
    g_object_set(G_OBJECT(VolumeFilter), "volume", Volume / 1000.0, NULL);
}

void Playable::SetFrequency(int32_t Time, int32_t Frequency)
{
}

void Playable::SetPan(int32_t Time, int32_t Pan)
{
}

void Playable::SetLoopPoint(int32_t Begin, int32_t End)
{
    this->Begin = GST_TIME_AS_MSECONDS(Begin);
    this->End = GST_TIME_AS_MSECONDS(End);
}

void Playable::OnEOS()
{
    if (Loop)
        thread([this](){Play();}).detach();
}

void Playable::Request(int32_t State)
{
    if (State == Nsb::PLAY)
        Play();
}

bool Playable::Action()
{
    return RemainTime() <= 0;
}
