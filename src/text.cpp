/* 
 * libnpengine: Nitroplus script interpreter
 * Copyright (C) 2013 Mislav Blažević <krofnica996@gmail.com>
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

#include <cassert>
#include <sstream>
#include <string>
#include <memory>
#include <iostream> // HACK
#include <SFML/Audio/Music.hpp>

Text::Text(const std::string& XML) : LineIter(0)
{
    std::istringstream ss(XML);
    std::string TextLine;
    size_t VoiceIter = 0;

    while (std::getline(ss, TextLine))
    {
        if (TextLine.empty() || TextLine.compare(0, 4, "<PRE") == 0 || TextLine == "</PRE>")
            continue;

        Voices.resize(Lines.size() + 1);

        // TODO: Line may start with <RUBY text or <FONT incolor
        if (TextLine.front() == '<')
        {
            std::string Attr;
            size_t i = TextLine.find('"'),  j, k = 0;
            while (i != std::string::npos)
            {
                j = TextLine.find('"', i + 1);
                Attr = TextLine.substr(i + 1, j - i - 1);
                std::cout << Attr << std::endl;
                i = TextLine.find('"', j + 1);
                switch (k)
                {
                    case 0: // ？？？
                    case 1: // VID_MAY
                        break;
                    case 2: // voice/MAY_0001
                    {
                        // TODO: Move to resourcemgr and reuse in interpreter
                        sf::Music* pMusic = new sf::Music;
                        uint32_t Size;
                        char* pMusicData = sResourceMgr->Read(Attr + ".ogg", &Size);
                        assert(pMusicData);
                        pMusic->openFromMemory(pMusicData, Size);
                        Voices[VoiceIter] = pMusic;
                        break;
                    }
                    case 3: // on
                        break;
                }
                ++k;
            }
        }
        else
        {
            Lines.push_back(TextLine);
            std::cout << TextLine;
            std::cin.get();
        }
        ++VoiceIter;
    }

    setString(Lines[0]);
    if (sf::Music* pMusic = Voices[0])
        pMusic->play();
}

Text::~Text()
{
    std::for_each(Voices.begin(), Voices.end(), std::default_delete<sf::Music>());
}

bool Text::NextLine()
{
    if (++LineIter >= Lines.size())
        return false;
    setString(Lines[LineIter]);
    if (sf::Music* pMusic = Voices[LineIter])
        pMusic->play();
    return true;
}
