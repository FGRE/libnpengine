#include "game.hpp"
#include "resourcemgr.hpp"

Game::Game(const std::vector<std::string>& AchieveFileNames, const std::string& InitScript) :
IsRunning(true)
{
    pInterpreter = new NsbInterpreter(this, new ResourceMgr(AchieveFileNames), InitScript);
}

Game::~Game()
{
    delete pInterpreter;
}

void Game::Run()
{
    while (IsRunning)
    {
        pInterpreter->Run();
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
