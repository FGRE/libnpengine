#ifndef GAME_HPP
#define GAME_HPP

#include <SFML/Graphics/RenderWindow.hpp>
#include <list>
#include "drawable.hpp"
#include "nsbinterpreter.hpp"

class Game : public sf::RenderWindow
{
    friend class NsbInterpreter;
public:
    Game(const std::vector<std::string>& AchieveFileNames, const std::string& InitScript);
    ~Game();

    void Run();

private:
    void AddDrawable(Drawable Obj);
    void RemoveDrawable(Drawable Obj);

    std::list<Drawable> Drawables;
    bool IsRunning;
    NsbInterpreter* pInterpreter;
};

#endif
