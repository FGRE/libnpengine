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

    while (IsRunning)
    {
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

        clear();
        for (auto d : Drawables)
            draw(*d.pDrawable);
        display();
    }
}

void Game::AddDrawable(Drawable Obj)
{
    auto Spot = Drawables.begin();
    while (Spot != Drawables.end())
    {
        if (Spot->Priority >= Obj.Priority)
            break;
        ++Spot;
    }
    Drawables.insert(Spot, Obj);
}

void Game::RemoveDrawable(Drawable Obj)
{
    Drawables.remove(Obj);
}

void Game::RegisterCallback(sf::Keyboard::Key Key, const std::string& Script)
{
    Callbacks.push_back({Key, Script});
}
