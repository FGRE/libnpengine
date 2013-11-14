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
#ifndef NSB_INTERPRETER_HPP
#define NSB_INTERPRETER_HPP

#include <stack>
#include <cstdint>
#include <map>
#include <vector>
#include <thread>

class NsbFile;
class Game;
class ResourceMgr;

struct Variable
{
    Variable() : Type("STRING") {}
    Variable(const std::string& Type, const std::string& Value) : Type(Type), Value(Value) {}

    std::string Type;
    std::string Value;
};

struct FuncReturn
{
    NsbFile* pScript;
    uint32_t SourceLine;
};

class NsbInterpreter
{
public:
    NsbInterpreter(Game* pGame, ResourceMgr* pResourceMgr, const std::string& InitScript);
    ~NsbInterpreter();

    void Stop();
    void Pause();
    void Start();

    void CallScript(const std::string& FileName); // Deprecated
    void LoadScript(const std::string& FileName);

private:
    void ThreadMain();
    void Run();

    bool Boolify(const std::string& String);
    template <class T> T GetVariable(const std::string& Identifier);
    void SetVariable(const std::string& Identifier, const Variable& Var);
    bool CallFunction(NsbFile* pDestScript, const char* FuncName); // Obsolete?

    void SetAudioState(const std::string& HandleName, int32_t NumSeconds,
                       int32_t Volume, const std::string& Tempo);
    void SetAudioLoop(const std::string& HandleName, bool Loop);
    void Destroy(std::string& HandleName);
    void SetAudioRange(const std::string& HandleName, int32_t begin, int32_t end);
    void LoadAudio(const std::string& HandleName, const std::string& Type, const std::string& File);
    void StartAnimation(const std::string& HandleName, int32_t TimeRequired,
                        int32_t x, int32_t y, const std::string& Tempo, bool Wait);
    void Sleep(int32_t ms);
    void ParseText(const std::string& unk0, const std::string& unk1, const std::string& Text);
    void LoadMovie(const std::string& HandleName, int32_t Priority, int32_t x,
                   int32_t y, bool Loop, bool unk0, const std::string& File, bool unk1);
    void LoadTexture(const std::string& HandleName, int32_t unk0, int32_t unk1,
                     int32_t unk2, const std::string& File);
    void Display(const std::string& HandleName, int32_t unk0, int32_t unk1,
                 const std::string& unk2, bool unk3);
    void SetDisplayState(const std::string& HandleName, const std::string& State);
    void GetMovieTime(const std::string& HandleName);

    void DumpTrace();
    void NsbAssert(bool expr, const char* fmt);
    void NsbAssert(const char* fmt);
    template<typename T, typename... A> void NsbAssert(bool expr, const char* fmt, T value, A... args);

    Game* pGame;
    ResourceMgr* pResourceMgr;
    NsbFile* pScript;
    volatile bool RunInterpreter;
    volatile bool StopInterpreter;

    std::stack<FuncReturn> Returns;
    std::vector<NsbFile*> LoadedScripts;
    std::map<std::string, Variable> Variables;
    std::vector<Variable> Params;
    std::thread ScriptThread;
};

#endif
