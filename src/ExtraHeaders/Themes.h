/*
	Theme control and music playback.
*/

#pragma once

#include <ASMMacros.h>
#include <ArrayClasses.h>
#include <Helpers/String.h>
#include <Helpers/CompileTime.h>

class AudioStream;

class ThemeClass
{
public:
	ThemeClass(const char* pID = nullptr) : ID(pID),
		Sound(),
		UIName(),
		Scenario(0),
		Length(0.0f),
		Normal(true),
		Repeat(false),
		Exists(false),
		Side(-1)
	{ }

	FixedString<0x100> ID;
	FixedString<0x100> Sound;
	FixedWString<0x40> UIName;
	int Scenario;
	float Length;
	bool Normal;
	bool Repeat;
	bool Exists;
	int Side;
};

class ThemePlayer
{
public:
	static constexpr reference<ThemePlayer, 0xA83D10> Instance{};

	const char* GetID(unsigned int index) const
		{ JMP_THIS(0x721270) }

	const char* GetName(unsigned int index) const
		{ JMP_THIS(0x720940) }

	const char* GetFilename(unsigned int index) const
		{ JMP_THIS(0x720E10) }

	const wchar_t* GetUIName(unsigned int index) const
		{ JMP_THIS(0x7209B0) }

	int GetLength(unsigned int index) const
		{ JMP_THIS(0x720E50) }

	bool IsAvailable(int index) const
		{ JMP_THIS(0x721140) }

	bool IsNormal(int index) const
		{ JMP_THIS(0x7211E0) }

	int FindIndex(const char* pID) const
		{ JMP_THIS(0x721210) }

	int GetRandomIndex(unsigned int lastTheme) const
		{ JMP_THIS(0x720A80) }

	void Queue(int index)
		{ JMP_THIS(0x720B20) }

	int Play(int index)
		{ JMP_THIS(0x720BB0) }

	void Stop(bool fade = false)
		{ JMP_THIS(0x720EA0) }

	void Suspend()
		{ JMP_THIS(0x720F70) }

	void Update()
		{ JMP_THIS(0x7209D0) }

	int CurrentTheme; // the playing theme's index
	int LastTheme; // the theme that cannot be selected randomly
	int QueuedTheme; // the next theme to be played
	int unknown_int_C;
	bool ScoreRepeat;
	bool unknown_bool_11;
	bool ScoreShuffle;
	DynamicVectorClass<ThemeClass*> Themes; // the list of all themes
	AudioStream* Stream;
};
