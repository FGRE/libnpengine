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
#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <list>
using namespace std;

class Texture;
class Window
{
public:
    Window(const char* WindowTitle, const int Width, const int Height);
    virtual ~Window();

    void Run();
    void AddTexture(Texture* pTexture);
    void MoveCursor(int32_t X, int32_t Y);

    const int WIDTH;
    const int HEIGHT;
protected:
    virtual void HandleEvent(SDL_Event Event);
    virtual void RunInterpreter() = 0;

private:
    void Draw();

    bool IsRunning;
    SDL_Window* SDLWindow;
    SDL_GLContext GLContext;
    list<Texture*> Textures;
};

#endif
