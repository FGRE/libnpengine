#ifndef NP_MOVIE_HPP
#define NP_MOVIE_HPP

#include <sfeMovie/Movie.hpp>
#include "drawable.hpp"

class Movie : public sfe::Movie
{
public:
    Movie(int32_t x, int32_t y, bool Loop, const std::string& File);

private:
};

#endif
