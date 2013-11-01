#include "game.hpp"
#include "resourcemgr.hpp"
#include "movie.hpp"

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
        auto d = Movies.begin();
        while (d != Movies.end())
        {
            if ((*d)->ShouldRemove())
                d = Movies.erase(d);
            else
            {
                if ((*d)->IsBlocking())
                    RunInterpreter = false;
                draw(**d);
            }
            ++d;
        }
        display();
    }
}

void Game::AddDrawable(Movie* pMovie)
{
    auto Spot = Movies.begin();
    while (Spot != Movies.end())
    {
        if ((*Spot)->GetPriority() >= pMovie->GetPriority())
            break;
        ++Spot;
    }
    Movies.insert(Spot, pMovie);
}

void Game::RemoveDrawable(Movie* pMovie)
{
    Movies.remove(pMovie);
}

void Game::RegisterCallback(sf::Keyboard::Key Key, const std::string& Script)
{
    Callbacks.push_back({Key, Script});
}
