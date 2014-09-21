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
#include "Window.hpp"

Movie::Movie(const string& FileName, Window* pWindow, int32_t Priority, bool Alpha, bool Audio) :
Playable(FileName),
Priority(Priority)
{
    InitVideo(pWindow);
    if (Audio)
        InitAudio();
}

Movie::~Movie()
{
}

void Movie::InitVideo(Window* pWindow)
{
    using X11::_XPrivDisplay;
    X11::SDL_SysWMinfo WindowInfo = pWindow->GetWindowInfo();

    if (WindowInfo.subsystem != X11::SDL_SYSWM_X11)
        return;

    unsigned long Black = BlackPixel(WindowInfo.info.x11.display, DefaultScreen(WindowInfo.info.x11.display));
    XWin = X11::XCreateSimpleWindow(WindowInfo.info.x11.display, WindowInfo.info.x11.window, 0, 0, 1024, 576, 0, Black, Black);

    if (!XWin || !X11::XSelectInput(WindowInfo.info.x11.display, XWin, StructureNotifyMask) || !X11::XMapWindow(WindowInfo.info.x11.display, XWin))
        return;

    while (1)
    {
        X11::XEvent Event;
        X11::XNextEvent(WindowInfo.info.x11.display, &Event);
        if (Event.type == MapNotify && Event.xmap.window == XWin)
            break;
    }

    X11::XSelectInput(WindowInfo.info.x11.display, XWin, NoEventMask);

    VideoBin = gst_bin_new("videobin");
    GstElement* VideoConv = gst_element_factory_make("videoconvert", "vconv");
    GstPad* VideoPad = gst_element_get_static_pad(VideoConv, "sink");
    GstElement* VideoSink = gst_element_factory_make("autovideosink", "sink");

    gst_bin_add(GST_BIN(VideoBin), VideoConv);
    gst_bin_add(GST_BIN(VideoBin), VideoSink);

    gst_element_link_many(VideoConv, VideoSink, nullptr);

    gst_element_add_pad(VideoBin, gst_ghost_pad_new("sink", VideoPad));
    gst_object_unref(VideoPad);
    gst_bin_add(GST_BIN(Pipeline), VideoBin);
}

void Movie::Delete(Window* pWindow)
{
    Playable::Delete(pWindow);
    X11::SDL_SysWMinfo WindowInfo = pWindow->GetWindowInfo();
    XDestroyWindow(WindowInfo.info.x11.display, XWin);
}
