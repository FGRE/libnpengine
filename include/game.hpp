#ifndef GAME_HPP
#define GAME_HPP

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <list>
#include "movie.hpp"
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
    void AddDrawable(Movie* pMovie);
    void RemoveDrawable(Movie* pMovie);
    void RegisterCallback(sf::Keyboard::Key Key, const std::string& Script);

    std::vector<Callback> Callbacks;
    std::list<Movie*> Movies;
    bool IsRunning;
    NsbInterpreter* pInterpreter;
};

#endif
