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
#ifndef GAME_HPP
#define GAME_HPP

#include "nsbinterpreter.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <boost/thread/mutex.hpp>
#include <list>
#include <queue>

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 576

class Text;
class Drawable;
class Movie;

class Game : public sf::RenderWindow
{
    friend class NsbInterpreter;
public:
    Game(NsbInterpreter* pInterpreter, const char* WindowTitle);
    ~Game();

    void Run();
    void GLCallback(const std::function<void()>& Func); // Request main thread to call Func
    void AddDrawable(DrawableBase* pDrawable);
    void RemoveDrawable(DrawableBase* pDrawable);

protected:
    virtual void HandleEvent(sf::Event Event);
    void ClearText();
    void SetText(Text* pText);
    void AddDrawable(Movie* pMovie);

    boost::mutex GLMutex;
    std::queue<std::function<void()>> GLCallbacks; // TODO: Only one is (probably?) actually possible in practice
    std::list<DrawableBase*> Drawables; // Objects drawn to window sorted by Z coordinate
    Movie* pMovie; // Currently drawn movie
    volatile bool IsRunning; // If false, game should exit
    bool IgnoreText; // If true, doesn't block interpreter untill user clicks
    Text* pText; // Currently drawn text
    NsbInterpreter* pInterpreter; // nsb interpreter
};

#endif
