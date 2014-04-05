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
#include "drawable.hpp"
#include "game.hpp"
#include "text.hpp"
#include "movie.hpp"
#include "nsbcontext.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <fstream>

sf::Texture* LoadTextureFromFile(const string& File, const sf::IntRect& Area = sf::IntRect())
{
    uint32_t Size;
    char* pPixels = sResourceMgr->Read(File, &Size);
    if (!pPixels)
    {
        std::cout << "Failed to load " << File << " pixels" << std::endl;
        return nullptr;
    }

    sf::Texture* pTexture = new sf::Texture;
    if (!pTexture->loadFromMemory(pPixels, Size, Area))
    {
        std::cout << "Failed to load pixels from " << File << " in memory" << std::endl;
        delete pTexture;
        pTexture = nullptr;
    }
    delete pPixels;
    return pTexture;
}

sf::Texture* LoadTextureFromColor(string Color, int32_t Width, int32_t Height)
{
    uint32_t IntColor;
    std::transform(Color.begin(), Color.end(), Color.begin(), ::tolower);

    if (Color[0] == '#')
    {
        Color = string(Color.c_str() + 1);
        std::stringstream ss(Color);
        ss >> std::hex >> IntColor;
    }
    else
    {
        if (Color == "black")
            IntColor = 0;
        else if (Color == "white")
            IntColor = 0xFFFFFF;
        else if (Color == "blue")
            IntColor = 0xFF;
        else
        {
            std::cout << "Unknown color: " << Color << std::endl;
            return nullptr;
        }
    }

    sf::Image ColorImage;
    ColorImage.create(Width, Height, sf::Color(IntColor / 0x10000, (IntColor / 0x100) % 0x100, IntColor & 0xFF));

    sf::Texture* pTexture = new sf::Texture;
    if (!pTexture->loadFromImage(ColorImage))
    {
        std::cout << "Failed to create texture from color " << Color << std::endl;
        delete pTexture;
        return nullptr;
    }
    return pTexture;
}

// TODO: Color unused
void NsbInterpreter::GLCreateRenderTexture(int32_t Width, int32_t Height, const string& Color)
{
    if (sf::RenderTexture* pTexture = CacheHolder<sf::RenderTexture>::Read(HandleName))
        delete pTexture;

    sf::RenderTexture* pTexture = new sf::RenderTexture;
    pTexture->create(Width, Height);
    CacheHolder<sf::RenderTexture>::Write(HandleName, pTexture);
}

void NsbInterpreter::GLDrawToTexture(sf::RenderTexture* pTexture, int32_t x, int32_t y, const string& File)
{
    if (sf::Texture* pTempTexture = LoadTextureFromFile(File))
    {
        sf::Sprite TempSprite(*pTempTexture);
        TempSprite.setPosition(x, y);
        pTexture->draw(TempSprite);
        pTexture->display();
        delete pTempTexture;
    }
}

// Just a fancy combo of GLCreateRenderTexture and GLDrawToTexture
void NsbInterpreter::GLLoadImage(const string& File)
{
    if (sf::RenderTexture* pTexture = CacheHolder<sf::RenderTexture>::Read(HandleName))
        delete pTexture;

    sf::Texture* pTempTexture = LoadTextureFromFile(File);
    if (!pTempTexture)
    {
        CacheHolder<sf::RenderTexture>::Write(HandleName, nullptr);
        return;
    }

    sf::Sprite TempSprite(*pTempTexture);
    TempSprite.setPosition(0, 0);
    sf::RenderTexture* pTexture = new sf::RenderTexture;
    pTexture->create(pTempTexture->getSize().x, pTempTexture->getSize().y);
    pTexture->draw(TempSprite);
    pTexture->display();
    delete pTempTexture;
    CacheHolder<sf::RenderTexture>::Write(HandleName, pTexture);
}

void NsbInterpreter::GLApplyBlur(Drawable* pDrawable, const string& Heaviness)
{
    pDrawable->SetBlur(Heaviness);
}

void NsbInterpreter::GLDrawTransition(Drawable* pDrawable, int32_t Time, int32_t Start, int32_t End, int32_t Range, const string& Tempo, string File, bool Wait)
{
    if (sf::Texture* pTexture = LoadTextureFromFile(File))
        pDrawable->SetMask(pTexture, Start, End, Time);
    if (Wait)
        pContext->Sleep(Time);
}

void NsbInterpreter::GLCreateColor(int32_t Priority, int32_t x, int32_t y, int32_t Width, int32_t Height, const string& Color)
{
    // Workaround
    if (HandleName == "クリア黒")
        return;

    if (DrawableBase* pDrawable = CacheHolder<DrawableBase>::Read(HandleName))
    {
        pGame->RemoveDrawable(pDrawable);
        delete pDrawable;
    }

    sf::Texture* pTexture = LoadTextureFromColor(Color, Width, Height);
    if (!pTexture)
        return;

    sf::Sprite* pSprite = new sf::Sprite(*pTexture);
    pSprite->setPosition(x, y);
    CacheHolder<DrawableBase>::Write(HandleName, new Drawable(pSprite, Priority, DRAWABLE_TEXTURE));
}

void NsbInterpreter::GLCreateMovie(int32_t Priority, int32_t x, int32_t y, bool Loop, bool Alpha, const string& File, bool Audio)
{
    if (std::ifstream("NOMOVIE"))
        return;

    if (Playable* pMovie = CacheHolder<Playable>::Read(HandleName))
    {
        pGame->AddDrawable((Movie*)nullptr);
        delete pMovie;
    }

    if (NsbAssert(!Alpha, "GLLoadMovie: Alpha not yet implemented"))
        return;

    string BoxHandle(HandleName, 0, HandleName.find_first_of("/"));
    sf::IntRect* pRect = CacheHolder<sf::IntRect>::Read(BoxHandle);
    Movie* pMovie = new Movie(File, pGame->getSystemHandle(), Priority, Alpha, Audio, pRect);
    pMovie->SetLoop(Loop);

    // Probably unused
    //pMovie->SetPosition(x, y);

    CacheHolder<Playable>::Write(HandleName, pMovie);
    pGame->AddDrawable(pMovie);
}

void NsbInterpreter::GLCreateTexture(int32_t Priority, PosFunc XFunc, PosFunc YFunc, const string& File)
{
    GLLoadTextureClip(Priority, XFunc, YFunc, 0, 0, 0, 0, File);
}

void NsbInterpreter::GLLoadTextureClip(int32_t Priority, PosFunc XFunc, PosFunc YFunc, int32_t tx, int32_t ty, int32_t width, int32_t height, const string& File)
{
    if (DrawableBase* pDrawable = CacheHolder<DrawableBase>::Read(HandleName))
    {
        pGame->RemoveDrawable(pDrawable);
        delete pDrawable;
    }

    sf::Texture* pTexture;

    // Load from texture instead of file
    if (sf::RenderTexture* pRenderTexture = CacheHolder<sf::RenderTexture>::Read(File))
        pTexture = new sf::Texture(pRenderTexture->getTexture()); // TODO: Dont copy
    else
        pTexture = LoadTextureFromFile(File, sf::IntRect(tx, ty, width, height));

    if (!pTexture)
    {
        CacheHolder<DrawableBase>::Write(HandleName, nullptr);
        return;
    }

    sf::Sprite* pSprite = new sf::Sprite(*pTexture);
    pSprite->setPosition(XFunc(pTexture->getSize().x), YFunc(pTexture->getSize().y));
    Drawable* pDrawable = new Drawable(pSprite, Priority, DRAWABLE_TEXTURE);
    CacheHolder<DrawableBase>::Write(HandleName, pDrawable);
    pGame->AddDrawable(pDrawable);
}

void NsbInterpreter::GLParseText(const string& Box, const string& XML)
{
    string NewHandle = Box + "/" + HandleName;
    SetVariable("$SYSTEM_present_text", new Variable(NewHandle));
    if (DrawableBase* pText = CacheHolder<DrawableBase>::Read(NewHandle))
        delete pText;
    Text* pText = new Text(XML);
    CacheHolder<DrawableBase>::Write(NewHandle, pText);
}

void NsbInterpreter::GLDelete(DrawableBase* pDrawable)
{
    if (pDrawable) // Not really needed?
        pGame->RemoveDrawable(pDrawable);
    delete pDrawable;
}

void NsbInterpreter::NSBRequest(const string& State)
{
    if (Drawable* pDrawable = (Drawable*)CacheHolder<DrawableBase>::Read(HandleName))
    {
        if (State == "Smoothing")
        {
            if (NsbAssert(pDrawable->Type == DRAWABLE_TEXTURE, "Smoothing non-texture drawable"))
                return;

            pGame->GLCallback([pDrawable]()
            {
                sf::Texture* pTexture = const_cast<sf::Texture*>(pDrawable->ToSprite()->getTexture());
                pTexture->setSmooth(true);
            });
        }
    }
    else if (Playable* pPlayable = CacheHolder<Playable>::Read(HandleName))
    {
        if (State == "Play")
            pPlayable->Play();
    }
    else if (NsbContext* pThread = CacheHolder<NsbContext>::Read(HandleName))
    {
        if (State == "Start")
            pThread->Start();
    }
}

void NsbInterpreter::NSBCreateWindow(int32_t unk0, int32_t x, int32_t y, int32_t Width, int32_t Height, bool unk1)
{
    sf::IntRect* pRect = new sf::IntRect(x, y, Width, Height);
    CacheHolder<sf::IntRect>::Write(HandleName, pRect);
}

// TODO: Rename; works for any playable
void NsbInterpreter::NSBGetMovieTime()
{
    int32_t Duration = 0;
    if (Playable* pPlayable = CacheHolder<Playable>::Read(HandleName))
        Duration = pPlayable->GetTimeLeft();
    else
        std::cout << "Failed to get movie time because there is no Playable " << HandleName << std::endl;
    Push(Duration);
}

void NsbInterpreter::NSBFade(DrawableBase* pDrawable, int32_t Time, int32_t Opacity, const string& Tempo, bool Wait)
{
    if (pDrawable->Type == DRAWABLE_TEXT)
    {
        if (NsbAssert(Opacity == 0, "There is no support for text fade out effect"))
            return;

        pGame->GLCallback(std::bind(&Game::ClearText, pGame));
        CacheHolder<DrawableBase>::Write(HandleName, nullptr); // hack: see Game::ClearText
    }
    else
        ((Drawable*)pDrawable)->SetOpacity(Opacity, Time, FADE_TEX);

    if (Wait)
        pContext->Sleep(Time);
}

void NsbInterpreter::NSBCreateSound(const string& Type, const string& File)
{
    if (Playable* pMusic = CacheHolder<Playable>::Read(HandleName))
        delete pMusic;

    NpaIterator AudioFile = sResourceMgr->GetFile(File);
    if (NsbAssert(AudioFile.GetFileSize() > 0, "Attempting to create Playable from empty file"))
    {
        CacheHolder<Playable>::Write(HandleName, nullptr);
        return;
    }

    CacheHolder<Playable>::Write(HandleName, new Playable(AudioFile));
}

void NsbInterpreter::NSBWaitText(Text* pText, const string& unk)
{
    if (Playable* pMusic = pText->Voices[0].pMusic)
    {
        pMusic->Play();
        pText->pCurrentMusic = pMusic;
    }
    pGame->SetText(pText);
    Pause();
}

void NsbInterpreter::NSBSetLoop(Playable* pMusic, bool Loop)
{
    pMusic->SetLoop(Loop);
}

void NsbInterpreter::GLMove(DrawableBase* pDrawable, int32_t Time, int32_t x, int32_t y, const string& Tempo, bool Wait)
{
    if (pDrawable->Type == DRAWABLE_TEXT)
    {
        if (!NsbAssert(Time == 0, "There is currently no support for moving text"))
            pDrawable->ToText()->setPosition(x, y);
    }
    else
        ((Drawable*)pDrawable)->AddLerpEffect(LERP_ANIM, x, y, Time);

    if (Wait)
        pContext->Sleep(Time);
}

void NsbInterpreter::NSBSetLoopPoint(Playable* pMusic, int32_t Begin, int32_t End)
{
    pMusic->SetRange(Begin, End);
}

// TODO: Actually audio fade out? Destroy after fade if Volume=0?
// NumSeconds is actually milliseconds
void NsbInterpreter::NSBSetVolume(Playable* pMusic, int32_t NumSeconds, int32_t Volume, const string& Tempo)
{
    pMusic->SetVolume(Volume / 1000.0d);
}

void NsbInterpreter::GLZoom(Drawable* pDrawable, int32_t Time, float x, float y, const string& Tempo, bool Wait)
{
    if (NsbAssert(x != 0 && y != 0, "Script attempted to Zoom with x or y scale value 0"))
        return;

    pDrawable->AddLerpEffect(LERP_ZOOM, x / 1000.0f, y / 1000.0f, Time);
    if (Wait)
        pContext->Sleep(Time);
}

void NsbInterpreter::NSBDelete()
{
    // Hack: Do not destroy * (aka everything)
    if (HandleName.back() == '*' && HandleName.size() != 1)
    {
        WildcardCall<DrawableBase>(HandleName, [this](DrawableBase* pDrawable)
        {
            pGame->GLCallback(std::bind(&NsbInterpreter::GLDelete, this, pDrawable));
            CacheHolder<DrawableBase>::Write(HandleName, nullptr);
        });
        WildcardCall<Playable>(HandleName, [this](Playable* pMovie)
        {
            delete pMovie;
            pGame->AddDrawable((Movie*)nullptr);
            CacheHolder<Playable>::Write(HandleName, nullptr);
        });
    }
    else if (DrawableBase* pDrawable = CacheHolder<DrawableBase>::Read(HandleName))
    {
        pGame->GLCallback(std::bind(&NsbInterpreter::GLDelete, this, pDrawable));
        CacheHolder<DrawableBase>::Write(HandleName, nullptr);
    }
    else if (Playable* pMovie = CacheHolder<Playable>::Read(HandleName))
    {
        delete pMovie;
        pGame->AddDrawable((Movie*)nullptr);
        CacheHolder<Playable>::Write(HandleName, nullptr);
    }
}

// ShellExecute
void NsbInterpreter::NSBSystem(string Command, string Parameters, string Directory)
{
    static const string OpenStr = "OPEN:";
    if (Command.substr(0, OpenStr.size()) != OpenStr)
        return;

    Command = Command.substr(OpenStr.size());

    if (fork() == 0)
        execlp("/usr/bin/xdg-open", "/usr/bin/xdg-open", Command.c_str(), NULL);
}

// Reads data from tree at specified depth
void NsbInterpreter::NSBArrayRead(int32_t Depth)
{
    size_t StackIndex = Stack.size() - Depth;
    int32_t _Depth = Depth;
    ArrayVariable* pVariable;
    auto iter = Arrays.find(HandleName);
    if (NsbAssert(iter != Arrays.end(), "Reading from non-existing array"))
    {
        pVariable = new ArrayVariable;
        goto cleanup;
    }

    pVariable = iter->second;
    while (Depth --> 0) // Depth goes to zero; 'cause recursion is too mainstream
    {
        ArrayMembers& Members = pVariable->Members;
        // Lookup by identifier
        if (string* Index = boost::get<string>(&Stack[StackIndex]->Value))
        {
            // Parameters contain identifiers of child nodes
            // for each level specifying which path to take
            for (auto i = Members.begin(); i != Members.end(); ++i)
            {
                // Node is found
                if (i->first == *Index)
                {
                    pVariable = i->second;
                    break; // Go to next level (if needed)
                }
                // TODO: Handle case when Identifier is not found
            }
        }
        // Lookup by index
        else if (int32_t* Index = boost::get<int32_t>(&Stack[StackIndex]->Value))
        {
            auto i = Members.begin();
            for (int j = 0; j < *Index; ++j)
                ++i;
            if (i != Members.end())
                pVariable = i->second;
        }
        ++StackIndex;
    }

cleanup:
    // Cleanup
    for (size_t i = 0; i < _Depth; ++i)
        Pop<string>();

    Stack.push(pVariable);
}

void NsbInterpreter::NSBCreateProcess(int32_t unk1, int32_t unk2, int32_t unk3, const string& Function)
{
    if (NsbContext* pThread = CacheHolder<NsbContext>::Read(HandleName))
    {
        Threads.remove(pThread);
        delete pThread;
    }

    NsbContext* pThread = new NsbContext(HandleName);
    pThread->CallSubroutine(pContext->GetScript(), Function);
    CacheHolder<NsbContext>::Write(HandleName, pThread);
    Threads.push_back(pThread);
}

void NsbInterpreter::NSBWriteFile(const string& Filename, const string& Data)
{
    std::ofstream File(Filename, std::ios::binary);
    File.write(Data.c_str(), Data.size());
    File.close();
}

void NsbInterpreter::NSBReadFile(const string& Filename)
{
    std::ifstream File(Filename, std::ios::binary);
    File.seekg(0, std::ios::end);
    string Data;
    Data.resize(File.tellg());
    File.seekg(0, std::ios::beg);
    File.read(&Data[0], Data.size());
    File.close();
    Push(Data);
}
