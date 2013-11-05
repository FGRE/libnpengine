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
#ifndef GAME_HPP
#define GAME_HPP

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <list>
#include "drawable.hpp"
#include "nsbinterpreter.hpp"

struct Callback
{
    sf::Keyboard::Key Key;
    std::string Script;
};

class Game : public sf::RenderWindow
{
    friend class NsbInterpreter;
public:
    Game(const std::vector<std::string>& AchieveFileNames, const std::string& InitScript);
    ~Game();

    void Run();

private:
    void AddDrawable(Drawable* pDrawable);
    void RemoveDrawable(Drawable* pDrawable);
    void RegisterCallback(sf::Keyboard::Key Key, const std::string& Script);

    std::vector<Callback> Callbacks;
    std::list<Drawable*> Drawables;
    bool IsRunning;
    NsbInterpreter* pInterpreter;
};

#endif
