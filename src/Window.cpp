/* 
 * libnpengine: Nitroplus script interpreter
 * Copyright (C) 2014-2016 Mislav Blažević <krofnica996@gmail.com>
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
#include <GL/glew.h>
#include <iostream>
#include "NSBInterpreter.hpp"
#include "Window.hpp"
#include "Texture.hpp"

uint32_t SDL_NSB_MOVECURSOR;
Window* Object::pWindow = nullptr;

Window::Window(const char* WindowTitle, const int Width, const int Height) : WIDTH(Width), HEIGHT(Height), pInterpreter(nullptr), IsRunning(true), EventLoop(false)
{
    Object::pWindow = this;
    SDL_Init(SDL_INIT_VIDEO);
    SDLWindow = SDL_CreateWindow(WindowTitle, 0, 0, WIDTH, HEIGHT, SDL_WINDOW_OPENGL);
    GLContext = SDL_GL_CreateContext(SDLWindow);
    SDL_NSB_MOVECURSOR = SDL_RegisterEvents(1);

    GLenum err = glewInit();
    if (err != GLEW_OK)
        cout << glewGetErrorString(err) << endl;

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glViewport(0, 0, WIDTH, HEIGHT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WIDTH, HEIGHT, 0, -1, 1);
}

Window::~Window()
{
    SDL_GL_DeleteContext(GLContext);
    SDL_DestroyWindow(SDLWindow);
    SDL_Quit();
    delete pInterpreter;
}

void Window::PushMoveCursorEvent(int X, int Y)
{
    SDL_Event Event;
    SDL_zero(Event);
    Event.type = SDL_NSB_MOVECURSOR;
    Event.user.data1 = reinterpret_cast<void*>(X);
    Event.user.data2 = reinterpret_cast<void*>(Y);
    SDL_PushEvent(&Event);
}

void Window::Run()
{
    LastDrawTime = SDL_GetTicks();
    SDL_Event Event;
    while (IsRunning)
    {
        while (SDL_PollEvent(&Event))
            HandleEvent(Event);

        Draw();
        pInterpreter->Run(100);
        SDL_Delay(16);
    }
}

void Window::Exit()
{
    IsRunning = false;
}

void Window::Select(bool Enable)
{
    EventLoop = Enable;
}

void Window::HandleEvent(SDL_Event& Event)
{
    switch (Event.type)
    {
    case SDL_QUIT:
        IsRunning = false;
        break;
    default:
        if (Event.type == SDL_NSB_MOVECURSOR)
            MoveCursor((int64_t)Event.user.data1, (int64_t)Event.user.data2);
        else if (EventLoop)
            pInterpreter->PushEvent(Event);
        break;
    }
    pInterpreter->HandleEvent(Event);
}

void Window::Draw()
{
    uint32_t Diff = SDL_GetTicks() - LastDrawTime;
    glClear(GL_COLOR_BUFFER_BIT);
    for (auto i = Textures.begin(); i != Textures.end(); ++i)
        (*i)->Draw(Diff);
    SDL_GL_SwapWindow(SDLWindow);
    LastDrawTime = SDL_GetTicks();
}

void Window::AddTexture(Texture* pTexture)
{
    auto Spot = Textures.begin();
    while (Spot != Textures.end())
    {
        if ((*Spot)->GetPriority() >= pTexture->GetPriority())
            break;
        ++Spot;
    }
    Textures.insert(Spot, pTexture);
}

void Window::RemoveTexture(Texture* pTexture)
{
    Textures.remove(pTexture);
}

void Window::MoveCursor(int X, int Y)
{
    SDL_WarpMouseInWindow(SDLWindow, X, Y);
}

X11::SDL_SysWMinfo Window::GetWindowInfo()
{
    X11::SDL_SysWMinfo Info;
    SDL_VERSION(&Info.version);
    if (!X11::SDL_GetWindowWMInfo(SDLWindow, &Info) || Info.subsystem != X11::SDL_SYSWM_X11)
        cout << "Failed to detect X11!" << endl;
    return Info;
}
