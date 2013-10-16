#include "movie.hpp"

Movie::Movie(int32_t x, int32_t y, bool Loop, const std::string& File)
{
    openFromFile(File);
    setPosition(x, y);
    setLoop(Loop);
    play();
}
