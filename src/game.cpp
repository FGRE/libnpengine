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
#include "game.hpp"
#include "resourcemgr.hpp"

#include <SFML/Window/Event.hpp>

Game::Game(const std::vector<std::string>& AchieveFileNames, const std::string& InitScript) :
sf::RenderWindow(sf::VideoMode(1024, 576), "steins-gate", sf::Style::Close),
IsRunning(true)
{
    sf::Vector2i WindowPos(sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height);
    WindowPos -= sf::Vector2i(1024, 576);
    WindowPos /= 2;
    setPosition(WindowPos);
    pInterpreter = new NsbInterpreter(this, new ResourceMgr(AchieveFileNames), InitScript);
}

Game::~Game()
{
    delete pInterpreter;
}

void Game::Run()
{
    sf::Event Event;
    bool RunInterpreter = true;

    while (IsRunning)
    {
        if (RunInterpreter)
            pInterpreter->Run();

        while (pollEvent(Event))
        {
            switch (Event.type)
            {
                case sf::Event::KeyPressed:
                    for (uint32_t i = 0; i < Callbacks.size(); ++i)
                        if (Callbacks[i].Key == Event.key.code)
                            pInterpreter->CallScript(Callbacks[i].Script);
                default:
                    break;
            }
        }

        RunInterpreter = true;

        clear();
        auto d = Drawables.begin();
        while (d != Drawables.end())
        {
            if (d->ShouldRemove())
                d = Drawables.erase(d);
            else
            {
                if (d->IsBlocking())
                    RunInterpreter = false;
                draw(*d->Get());
            }
            ++d;
        }
        display();
    }
}

void Game::AddDrawable(Drawable Obj)
{
    auto Spot = Drawables.begin();
    while (Spot != Drawables.end())
    {
        if (Spot->GetPriority() >= Obj.GetPriority())
            break;
        ++Spot;
    }
    Drawables.insert(Spot, Obj);
}

void Game::RemoveDrawable(sf::Drawable* pDrawable)
{
    Drawables.remove(Drawable(pDrawable, 0, false, 0));
}

void Game::RegisterCallback(sf::Keyboard::Key Key, const std::string& Script)
{
    Callbacks.push_back({Key, Script});
}
