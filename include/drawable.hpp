#ifndef DRAWABLE_HPP
#define DRAWABLE_HPP

struct Drawable
{
    bool operator==(const Drawable& Other)
    {
        return Other.pDrawable == pDrawable;
    }

    sf::Drawable* pDrawable;
    int32_t Priority;
};

#endif
