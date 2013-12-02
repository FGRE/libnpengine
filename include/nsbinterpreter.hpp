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
#include <queue>
#include <vector>
#include <boost/thread/thread.hpp>

using std::string;

class NsbFile;
class Game;
class ResourceMgr;
class Line;
class Drawable;
class ArrayVariable;

typedef std::vector<std::pair<string, ArrayVariable>> ArrayMembers;

struct Variable
{
    Variable() : Type("STRING") {}
    Variable(const string& Type, const string& Value) : Type(Type), Value(Value) {}

    string Type;
    string Value;
};

struct ArrayVariable : Variable
{
    ArrayVariable() : Variable() {}
    ArrayVariable(const Variable& Var) : Variable(Var.Type, Var.Value) {}
    ArrayMembers Members;
};

struct FuncReturn
{
    NsbFile* pScript;
    uint32_t SourceLine;
};

class NsbInterpreter
{
    typedef void (NsbInterpreter::*BuiltinFunc)();
public:
    NsbInterpreter(Game* pGame, const string& InitScript);
    ~NsbInterpreter();

    void Stop();
    void Pause();
    void Start();

    void CallScript(const string& FileName); // Deprecated
    void DumpState();
private:
    void LoadScript(const string& FileName);

    void RegisterBuiltins();
    void ThreadMain(string InitScript);
    void ExecuteLine();

    bool Boolify(const string& String);
    template <class T> T GetParam(int32_t Index);
    template <class T> T GetVariable(const string& Identifier);
    template <class T> void WildcardCall(std::string Handle, T Func);

    void CreateColor();
    void SetOpacity();
    void End();
    void LoadTexture();
    void Destroy();
    void Call();
    void Concat();
    void Format();
    void BindIdentifier();
    void LoadMovie();
    void ApplyMask();

    void CreateTexture(const string& HandleName, int32_t Width, int32_t Height, const string& Color);
    void DrawToTexture(const string& HandleName, int32_t x, int32_t y, const string& File);
    void ApplyBlur(Drawable* pDrawable, const string& Heaviness);
    void DisplayText(const string& unk);
    void CreateBox(int32_t unk0, int32_t x, int32_t y, int32_t Width, int32_t Height, bool unk1);
    void SetVariable(const string& Identifier, const Variable& Var);
    bool CallFunction(NsbFile* pDestScript, const char* FuncName); // Obsolete?
    void ArrayRead(const string& HandleName, int32_t Depth);
    void SetTextboxAttributes(const string& Handle, int32_t unk0, const string& Font, int32_t unk1, const string& Color1, const string& Color2, int32_t unk2, const string& unk3);
    void SetFontAttributes(const string& Font, int32_t size, const string& Color1, const string& Color2, int32_t unk0, const string& unk1);
    void SetAudioState(const string& HandleName, int32_t NumSeconds, int32_t Volume, const string& Tempo);
    void SetAudioLoop(const string& HandleName, bool Loop);
    void SetAudioRange(const string& HandleName, int32_t Begin, int32_t End);
    void LoadAudio(const string& HandleName, const string& Type, const string& File);
    void StartAnimation(const string& HandleName, int32_t Time, int32_t x, int32_t y, const string& Tempo, bool Wait);
    void Sleep(int32_t ms);
    void ParseText(const string& HandleName, const string& Box, const string& XML);
    void SetDisplayState(const string& HandleName, const string& State);
    void GetMovieTime(const string& HandleName);

    void NSBSetOpacity(Drawable* pDrawable, int32_t Time, int32_t Opacity, const string& Tempo, bool Wait);
    void GLDestroy(Drawable* pDrawable);
    void GLLoadTexture(const string& HandleName, int32_t Priority, int32_t x, int32_t y, const string& File);
    void GLCreateColor(const string& HandleName, int32_t Priority, int32_t x, int32_t y, int32_t Width, int32_t Height, string Color);
    void GLLoadMovie(int32_t Priority, int32_t x, int32_t y, bool Loop, bool unk0, const string& File, bool unk1);
    void GLApplyMask(Drawable* pDrawable, int32_t Time, int32_t Start, int32_t End, int32_t Range, const string& Tempo, string File, bool Wait);

    void Recover();
    void WriteTrace(std::ostream& Stream);
    void Crash();
    bool NsbAssert(bool expr, const char* fmt);
    void NsbAssert(const char* fmt);
    template<typename T, typename... A> void NsbAssert(const char* fmt, T value, A... args);
    template<typename T, typename... A> bool NsbAssert(bool expr, const char* fmt, T value, A... args);

    Line* pLine;
    Game* pGame;
    NsbFile* pScript;
    volatile bool RunInterpreter;
    volatile bool StopInterpreter;

    string HandleName;
    std::stack<FuncReturn> Returns;
    std::vector<NsbFile*> LoadedScripts;
    std::map<string, Variable> Variables;
    std::map<string, ArrayVariable> Arrays;
    std::vector<Variable> Params;
    std::vector<ArrayVariable*> ArrayParams;
    std::queue<Variable> Placeholders;
    std::vector<BuiltinFunc> Builtins;
    boost::thread ScriptThread;
};

template <> bool NsbInterpreter::GetParam(int32_t Index);

#endif
