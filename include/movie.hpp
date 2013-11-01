#ifndef NP_MOVIE_HPP
#define NP_MOVIE_HPP

#include <sfeMovie/Movie.hpp>

class Movie : public sfe::Movie
{
public:
    Movie(int32_t x, int32_t y, bool Loop, const std::string& File, int32_t Priority, bool Blocking);

    bool ShouldRemove() const;
    bool IsBlocking() const;
    int32_t GetPriority() const;

private:
    int32_t Priority;
    bool Blocking;
};

#endif
