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
#include "game.hpp"
#include "drawable.hpp"
#include "text.hpp"
#include "movie.hpp"

#include <SFML/Window/Event.hpp>

Game::Game(NsbInterpreter* pInterpreter, const char* WindowTitle) :
sf::RenderWindow(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), WindowTitle, sf::Style::Close),
IsRunning(true),
IgnoreText(false),
pText(nullptr),
pMovie(nullptr),
pInterpreter(pInterpreter)
{
    setFramerateLimit(60);
    sf::Vector2i WindowPos(sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height);
    WindowPos -= sf::Vector2i(WINDOW_WIDTH, WINDOW_HEIGHT);
    WindowPos /= 2;
    setPosition(WindowPos);
}

Game::~Game()
{
}

void Game::Run()
{
    sf::Event Event;

    while (IsRunning)
    {
        while (pollEvent(Event))
            HandleEvent(Event);

        clear();
        auto d = Drawables.begin();
        while (d != Drawables.end())
        {
            (*d)->Update();
            (*d)->Draw(this);
            ++d;
        }
        if (pText)
            draw(*pText->ToText());
        if (pMovie)
        {
            pushGLStates();
            pMovie->Update();
            popGLStates();
        }
        else
            display();

        GLMutex.lock();
        while (!GLCallbacks.empty())
        {
            GLCallbacks.front()();
            GLCallbacks.pop();
        }
        GLMutex.unlock();

        if (!pText || IgnoreText)
            pInterpreter->Start();
    }

    pInterpreter->Stop();
}

void Game::HandleEvent(sf::Event Event)
{
    switch (Event.type)
    {
        case sf::Event::KeyPressed:
            // TODO: Move to interpreter
            if (Event.key.code == sf::Keyboard::F8)
                pInterpreter->DumpState();
            else if (Event.key.code == sf::Keyboard::LControl)
                IgnoreText = !IgnoreText;
            pInterpreter->KeyPressed(Event.key.code);
            break;
        case sf::Event::MouseButtonPressed:
            if (pText && Event.mouseButton.button == sf::Mouse::Left)
                if (!pText->NextLine())
                    pInterpreter->Start();
            pInterpreter->MouseClicked(Event.mouseButton);
            break;
        case sf::Event::MouseMoved:
                pInterpreter->MouseMoved(sf::Mouse::getPosition(*this));
            break;
        case sf::Event::Closed:
            IsRunning = false;
            break;
        default:
            break;
    }
}

void Game::GLCallback(const std::function<void()>& Func)
{
    GLMutex.lock();
    GLCallbacks.push(Func);
    GLMutex.unlock();
    pInterpreter->Pause();
}

void Game::SetText(Text* pText)
{
    this->pText = pText;
}

void Game::ClearText()
{
    delete pText; // Doesn't seem quite right...
    pText = nullptr;
}

void Game::AddDrawable(DrawableBase* pDrawable)
{
    auto Spot = Drawables.begin();
    while (Spot != Drawables.end())
    {
        if ((*Spot)->GetPriority() >= pDrawable->GetPriority())
            break;
        ++Spot;
    }
    Drawables.insert(Spot, pDrawable);
}

void Game::AddDrawable(Movie* pMovie)
{
    this->pMovie = pMovie;
}

void Game::RemoveDrawable(DrawableBase* pDrawable)
{
    Drawables.remove(pDrawable);
}
