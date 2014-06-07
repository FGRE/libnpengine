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
#include "movie.hpp"
#include <thread>

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
Loop(false),
AudioBin(nullptr),
Appsrc(nullptr),
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

int32_t Playable::GetTimeLeft()
{
    gint64 Length, Position;
    gst_element_get_state(Pipeline, NULL, NULL, GST_CLOCK_TIME_NONE);
    gst_element_query_duration(Pipeline, GST_FORMAT_TIME, &Length);
    gst_element_query_position(Pipeline, GST_FORMAT_TIME, &Position);
    return (Length - Position) / 1000000;
}

int32_t Playable::GetDuration()
{
    gint64 Length;
    gst_element_get_state(Pipeline, NULL, NULL, GST_CLOCK_TIME_NONE);
    gst_element_query_duration(Pipeline, GST_FORMAT_TIME, &Length);
    return Length / 1000000;
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

void Playable::Update()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void Playable::SetVolume(double Volume)
{
    g_object_set(G_OBJECT(VolumeFilter), "volume", Volume, NULL);
}

void Playable::SetRange(int32_t Begin, int32_t End)
{
    this->Begin = gint64(Begin) * 1000000;
    this->End = gint64(End) * 1000000;
}

void Playable::Request(Game* pGame, const string& State)
{
    if (State == "Play")
        Play();
}

void Playable::Delete(Game* pGame, NsbInterpreter* pInterpreter)
{
    delete this;
}
