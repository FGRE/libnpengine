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
#ifndef NSB_INTERPRETER_HPP
#define NSB_INTERPRETER_HPP

#include <stack>
#include <cstdint>
#include <map>
#include <queue>
#include <vector>
#include <functional>
using std::string;

#define SPECIAL_POS_NUM 7

namespace sf
{
    class RenderTexture;
    class Texture;
}

class NsbFile;
class Game;
class ResourceMgr;
class Line;
class Drawable;
class ArrayVariable;
class Text;
class Playable;

typedef std::vector<std::pair<string, ArrayVariable>> ArrayMembers;

// Represents Nitroscript variable
struct Variable
{
    Variable() : Type("STRING") {}
    Variable(const string& Type, const string& Value) : Type(Type), Value(Value) {}

    string Type;
    string Value;
};

// Note that this structure is actually a tree. See: ArrayMembers typedef
struct ArrayVariable : Variable
{
    ArrayVariable() : Variable() {}
    ArrayVariable(const Variable& Var) : Variable(Var.Type, Var.Value) {}
    ArrayMembers Members;
};

// Element of call stack
// Contains script and line to which function will return upon ending
struct FuncReturn
{
    NsbFile* pScript;
    uint32_t SourceLine;
};

// Concurrently running function created by CreateThread
struct Thread
{
    string FuncSymbol;
};

class NsbInterpreter
{
    typedef void (NsbInterpreter::*BuiltinFunc)();
    friend void NitroscriptMain(NsbInterpreter* pInterpreter);
public:
    NsbInterpreter(Game* pGame);
    ~NsbInterpreter();

    void Stop();
    void Pause();
    void Start();

    void CallScript(const string& FileName);
    void DumpState();

private:
    void Run();
    void Sleep(int32_t ms);
    void LoadScript(const string& FileName);
    void ExecuteScript(const string& InitScript);
    void ExecuteScriptLocal(const string& InitScript);

    // Magic handlers
    void CreateThread();
    void LoadTextureClip();
    void Increment();
    void LogicalGreater();
    void LogicalLess();
    void ArraySize();
    void SetParam();
    void Get();
    void DrawToTexture();
    void CreateColor();
    void SetOpacity();
    void Begin();
    void End();
    void LoadTexture();
    void Destroy();
    void Call();
    void Add();
    void Format();
    void LoadMovie();
    void ApplyMask();
    void ClearParams();
    void CreateTexture();
    void GetMovieTime();
    void ApplyBlur();
    void CreateBox();
    void SetTextboxAttributes();
    void LoadAudio();
    void SetAudioRange();
    void SetFontAttributes();
    void SetAudioState();
    void DisplayText();
    void StartAnimation();
    void SleepMs();
    void SetAudioLoop();
    void ParseText();
    void SetState();
    void RegisterCallback();
    void ArrayRead();
    void Set();
    void Negative();
    void PlaceholderParam();
    void Zoom();
    void If();
    void While();
    void LogicalNot();
    void LogicalEqual();
    void LogicalNotEqual();
    void CallChapter();
    void Center();
    void System();
    void CreateScrollbar();
    void CallScene();

    // Stubs
    void UNK1();
    void UNK2();
    void UNK3();
    void UNK4();
    void UNK5();
    void UNK65();
    void UNK77();

    // Builtin functions
    void NSBDestroy();
    void NSBZoom(Drawable* pDrawable, int32_t Time, float x, float y, const string& Tempo, bool Wait);
    void NSBArrayRead(int32_t Depth);
    void NSBSetState(const string& State);
    void NSBSetAudioLoop(Playable* pMusic, bool Loop);
    void NSBStartAnimation(Drawable* pDrawable, int32_t Time, int32_t x, int32_t y, const string& Tempo, bool Wait);
    void NSBDisplayText(Text* pText, const string& unk);
    void NSBSetAudioState(Playable* pMusic, int32_t NumSeconds, int32_t Volume, const string& Tempo);
    void NSBSetAudioRange(Playable* pMusic, int32_t Begin, int32_t End);
    void NSBSetFontAttributes(const string& Font, int32_t Size, const string& Color1, const string& Color2, int32_t unk0, const string& unk1);
    void NSBLoadAudio(const string& Type, const string& File);
    void NSBSetTextboxAttributes(int32_t unk0, const string& Font, int32_t unk1, const string& Color1, const string& Color2, int32_t unk2, const string& unk3);
    void NSBCreateBox(int32_t unk0, int32_t x, int32_t y, int32_t Width, int32_t Height, bool unk1);
    void NSBGetMovieTime();
    void NSBSetOpacity(Drawable* pDrawable, int32_t Time, int32_t Opacity, const string& Tempo, bool Wait);
    void NSBBindIdentifier();
    void NSBCreateArray();
    void NSBCreateThread(int32_t unk1, int32_t unk2, int32_t unk3, string Function);
    void NSBSystem(string Command, string Parameters, string Directory);

    // GL functions are builtins like NSB, but need to be called from OpenGL thread
    // See: Game::GLCallback
    void GLDestroy(Drawable* pDrawable);
    void GLLoadTexture(int32_t Priority, int32_t x, int32_t y, const string& File);
    void GLCreateColor(int32_t Priority, int32_t x, int32_t y, int32_t Width, int32_t Height, string Color);
    void GLLoadMovie(int32_t Priority, int32_t x, int32_t y, bool Loop, bool Alpha, const string& File, bool Audio);
    void GLApplyMask(Drawable* pDrawable, int32_t Time, int32_t Start, int32_t End, int32_t Range, const string& Tempo, string File, bool Wait);
    void GLCreateTexture(int32_t Width, int32_t Height, const string& Color);
    void GLDrawToTexture(sf::RenderTexture* pTexture, int32_t x, int32_t y, const string& File);
    void GLApplyBlur(Drawable* pDrawable, const string& Heaviness);
    void GLParseText(const string& Box, const string& XML);
    void GLLoadTextureClip(int32_t Priority, int32_t x, int32_t y, int32_t tx, int32_t ty, int32_t width, int32_t height, string File);

    template <class T> T GetParam(int32_t Index);
    template <class T> T GetVariable(const string& Identifier);
    template <class T> void WildcardCall(std::string Handle, std::function<void(T*)> Func);
    bool CallFunction(NsbFile* pDestScript, const char* FuncName);
    bool JumpTo(uint16_t Magic);
    void ReverseJumpTo(uint16_t Magic);
    void SetVariable(const string& Identifier, const Variable& Var);
    sf::Texture* LoadTextureFromFile(const string& File);

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
    volatile int32_t WaitTime;

    string HandleName; // Identifier of current Drawable/Playable used by NSB and GL functions
    bool BranchCondition; // If false, code block after If/While is not executed
    std::stack<FuncReturn> Returns; // Call stack
    std::vector<NsbFile*> LoadedScripts; // Scripts considered in symbol lookup
    std::map<string, Variable> Variables; // All local and global variables (TODO: respect scope)
    std::map<string, ArrayVariable> Arrays; // Same as above, except these are trees
    std::vector<Variable> Params; // Builtin function parameters
    std::vector<ArrayVariable*> ArrayParams;
    std::vector<BuiltinFunc> Builtins; // Jump table for builtin functions
};

template <> bool NsbInterpreter::GetParam(int32_t Index);
template <> bool NsbInterpreter::NsbAssert(bool expr, const char* fmt, std::string value);

#endif
