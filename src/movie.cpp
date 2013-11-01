#include "movie.hpp"

Movie::Movie(int32_t x, int32_t y, bool Loop, const std::string& File, int32_t Priority, bool Blocking) :
Priority(Priority),
Blocking(Blocking)
{
    openFromFile(File);
    setPosition(x, y);
    setLoop(Loop);
    play();
}

bool Movie::ShouldRemove() const
{
    return getStatus() != sfe::Movie::Playing;
}

bool Movie::IsBlocking() const
{
    return Blocking;
}

int32_t Movie::GetPriority() const
{
    return Priority;
}
