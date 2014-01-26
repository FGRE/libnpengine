/* 
 * libnpengine: Nitroplus script interpreter
 * Copyright (C) 2013-2014 Mislav Blažević <krofnica996@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * */
#include "text.hpp"
#include "resourcemgr.hpp"
#include "playable.hpp"

#include <cassert>
#include <sstream>
#include <string>
#include <memory>
#include <fstream>
#include <limits>
#include <SFML/Graphics/RenderWindow.hpp>

sf::Font Text::Font;
bool Text::Fuwanovel;

static const char* lf = "\n";

Text::Text(const std::string& XML) :
::DrawableBase(new sf::Text, std::numeric_limits<int32_t>::max(), DRAWABLE_TEXT),
LineIter(0),
pCurrentMusic(nullptr)
{
    std::istringstream ss(XML);
    std::string TextLine;
    Playable* pMusic = nullptr;

    while (std::getline(ss, TextLine))
    {
        if (TextLine.empty() || TextLine.compare(0, 4, "<PRE") == 0 || TextLine == "</PRE>")
            continue;

        // TODO: Line may start with <RUBY text or <FONT incolor or <center>stuff</center>
        if (TextLine.compare(0, 6, "<voice") == 0)
        {
            std::string Attr;
            size_t i = TextLine.find('"'),  j, k = 0;
            while (i != std::string::npos)
            {
                j = TextLine.find('"', i + 1);
                Attr = TextLine.substr(i + 1, j - i - 1);
                i = TextLine.find('"', j + 1);
                switch (k)
                {
                    case 0: // ？？？
                        static_cast<sf::Text*>(pDrawable)->setString(sf::String::fromUtf8(Attr.begin(), Attr.end()));
                        break;
                    case 1: // VID_MAY
                        break;
                    case 2: // voice/MAY_0001
                        pMusic = new Playable(sResourceMgr->GetFile(Attr + ".ogg"));
                        break;
                    case 3: // on
                        break;
                }
                ++k;
            }
        }
        else
        {
            Voices.push_back({pMusic, sf::String::fromUtf8(TextLine.begin(), TextLine.end())});
            sf::String* Str = &Voices[Voices.size() - 1].String;
            if (Fuwanovel)
            {
                size_t i = 0, j, k = 1;
                do
                {
                do
                {
                    j = i;
                    i = Str->find(" ", i + 1);
                } while (i < 52 * k && i < Str->getSize());
                if (j+1 < Str->getSize() && i != sf::String::InvalidPos)
                    Str->insert(j + 1, sf::String::fromUtf8(lf, lf + 1));
                ++k;
                } while (i != sf::String::InvalidPos);
            }
            else
            {
                size_t i = 28;
                while (i < Str->getSize())
                {
                    Str->insert(i, sf::String::fromUtf8(lf, lf + 1));
                    i += 29;
                }
            }
            pMusic = nullptr;
        }
    }

    ToText()->setFont(Font);
    ToText()->setString(Voices[0].String);
}

Text::~Text()
{
    StopMusic();
    std::for_each(Voices.begin(), Voices.end(), [](const Voice& V) { delete V.pMusic; });
}

void Text::StopMusic()
{
    if (pCurrentMusic)
        pCurrentMusic->Stop();
}

bool Text::NextLine()
{
    if (++LineIter >= Voices.size())
        return false;

    ToText()->setString(Voices[LineIter].String);
    if (Playable* pMusic = Voices[LineIter].pMusic)
    {
        StopMusic();
        pMusic->Play();
        pCurrentMusic = pMusic;
    }
    return true;
}

void Text::Initialize(const std::string& FontFile)
{
    if (std::ifstream("FUWANOVEL"))
        Fuwanovel = true;
    else
        Fuwanovel = false;
    assert(Font.loadFromFile(FontFile));
}
