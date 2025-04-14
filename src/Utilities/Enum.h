#pragma region Ares Copyrights
/*
 *Copyright (c) 2008+, All Ares Contributors
 *All rights reserved.
 *
 *Redistribution and use in source and binary forms, with or without
 *modification, are permitted provided that the following conditions are met:
 *1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *3. All advertising materials mentioning features or use of this software
 *   must display the following acknowledgement:
 *   This product includes software developed by the Ares Contributors.
 *4. Neither the name of Ares nor the
 *   names of its contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 *THIS SOFTWARE IS PROVIDED BY ITS CONTRIBUTORS ''AS IS'' AND ANY
 *EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *DISCLAIMED. IN NO EVENT SHALL THE ARES CONTRIBUTORS BE LIABLE FOR ANY
 *DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#pragma endregion

#pragma once

#include <GeneralDefinitions.h>

enum class AttachedAnimFlag
{
	None = 0x0,
	Hides = 0x1,
	Temporal = 0x2,
	Paused = 0x4,

	PausedTemporal = Paused | Temporal
};

MAKE_ENUM_FLAGS(AttachedAnimFlag);

enum class SuperWeaponAITargetingMode
{
	None = 0,
	Nuke = 1,
	LightningStorm = 2,
	PsychicDominator = 3,
	ParaDrop = 4,
	GeneticMutator = 5,
	ForceShield = 6,
	NoTarget = 7,
	Offensive = 8,
	Stealth = 9,
	Self = 10,
	Base = 11,
	MultiMissile = 12,
	HunterSeeker = 13,
	EnemyBase = 14
};

enum class LandTypeFlags : unsigned short
{
	None = 0,
	Clear = 1 << (char)LandType::Clear,
	Road = 1 << (char)LandType::Road,
	Water = 1 << (char)LandType::Water,
	Rock = 1 << (char)LandType::Rock,
	Wall = 1 << (char)LandType::Wall,
	Tiberium = 1 << (char)LandType::Tiberium,
	Beach = 1 << (char)LandType::Beach,
	Rough = 1 << (char)LandType::Rough,
	Ice = 1 << (char)LandType::Ice,
	Railroad = 1 << (char)LandType::Railroad,
	Tunnel = 1 << (char)LandType::Tunnel,
	Weeds = 1 << (char)LandType::Weeds,

	All = 0xFFFF,
	DefaultDisallowed = Water | Rock | Ice | Beach
};

MAKE_ENUM_FLAGS(LandTypeFlags);

constexpr bool IsLandTypeInFlags(LandTypeFlags flags, LandType type)
{
	return (bool)((LandTypeFlags)(1 << (char)type) & flags);
}

enum class AffectedTarget : unsigned char
{
	None = 0x0,
	Land = 0x1,
	Water = 0x2,
	NoContent = 0x4,
	Infantry = 0x8,
	Unit = 0x10,
	Building = 0x20,
	Aircraft = 0x40,

	All = 0xFF,
	AllCells = Land | Water,
	AllTechnos = Infantry | Unit | Building | Aircraft,
	AllContents = NoContent | AllTechnos
};

MAKE_ENUM_FLAGS(AffectedTarget);

enum class AffectedHouse : unsigned char
{
	None = 0x0,
	Owner = 0x1,
	Allies = 0x2,
	Enemies = 0x4,

	Team = Owner | Allies,
	NotAllies = Owner | Enemies,
	NotOwner = Allies | Enemies,
	All = Owner | Allies | Enemies
};

MAKE_ENUM_FLAGS(AffectedHouse);

enum class OwnerHouseKind : int
{
	Default,
	Invoker,
	Killer,
	Victim,
	Civilian,
	Special,
	Neutral,
	Random
};

enum class SuperWeaponFlags : unsigned short
{
	None = 0x0,
	NoAnim = 0x1,
	NoSound = 0x2,
	NoEvent = 0x4,
	NoEVA = 0x8,
	NoMoney = 0x10,
	NoCleanup = 0x20,
	NoMessage = 0x40,
	PreClick = 0x80,
	PostClick = 0x100
};

MAKE_ENUM_FLAGS(SuperWeaponFlags);

enum class AreaFireTarget
{
	Base = 0,
	Self = 1,
	Random = 2
};

enum class SlaveChangeOwnerType
{
	Killer = 0, // default
	Master = 1,
	Suicide = 2,
	Neutral = 4,
};

enum class AutoDeathBehavior
{
	Kill = 0,     // default death option
	Vanish = 1,
	Sell = 2,     // buildings only
};

enum class SelfHealGainType
{
	NoHeal = 0,
	Infantry = 1,
	Units = 2
};

enum class InterceptedStatus
{
	None = 0,
	Targeted = 1,
	Intercepted = 2
};

enum class PhobosAction
{
	None = 0,
	Hijack = 1,
	Drive = 2
};

enum class TextAlign : int
{
	None = 0xFFF,
	Left = 0x000,
	Center = 0x100,
	Right = 0x200,
};

MAKE_ENUM_FLAGS(TextAlign);

enum class IronCurtainEffect : BYTE
{
	Kill = 0,
	Invulnerable = 1,
	Ignore = 2
};

enum class TargetZoneScanType
{
	Same = 0,
	Any = 1,
	InRange = 2
};

enum class DamageDisplayType
{
	Regular = 0,
	Shield = 1,
	Intercept = 2
};

enum class ChronoSparkleDisplayPosition : unsigned char
{
	None = 0x0,
	Building = 0x1,
	Occupants = 0x2,
	OccupantSlots = 0x4,

	All = 0xFF,
};

MAKE_ENUM_FLAGS(ChronoSparkleDisplayPosition);

enum class HorizontalPosition : BYTE
{
	Left = 0,
	Center = 1,
	Right = 2
};

enum class VerticalPosition : BYTE
{
	Top = 0,
	Center = 1,
	Bottom = 2
};

//hexagon
enum class BuildingSelectBracketPosition :BYTE
{
	Top = 0,
	LeftTop = 1,
	LeftBottom = 2,
	Bottom = 3,
	RightBottom = 4,
	RightTop = 5
};

enum class DisplayInfoType : BYTE
{
	Health = 0,
	Shield = 1,
	Ammo = 2,
	MindControl = 3,
	Spawns = 4,
	Passengers = 5,
	Tiberium = 6,
	Experience = 7,
	Occupants = 8,
	GattlingStage = 9
};

class MouseCursorHotSpotX
{
public:
	typedef MouseHotSpotX Value;

	static bool Parse(char* key, Value* value)
	{
		if (key && value)
		{
			if (!_strcmpi(key, "left"))
			{
				*value = MouseHotSpotX::Left;
			}
			else if (!_strcmpi(key, "right"))
			{
				*value = MouseHotSpotX::Right;
			}
			else if (!_strcmpi(key, "center"))
			{
				*value = MouseHotSpotX::Center;
			}
			else
			{
				return false;
			}
			return true;
		}
		return false;
	}
};

class MouseCursorHotSpotY
{
public:
	typedef MouseHotSpotY Value;

	static bool Parse(char* key, Value* value)
	{
		if (key && value)
		{
			if (!_strcmpi(key, "top"))
			{
				*value = MouseHotSpotY::Top;
			}
			else if (!_strcmpi(key, "bottom"))
			{
				*value = MouseHotSpotY::Bottom;
			}
			else if (!_strcmpi(key, "middle"))
			{
				*value = MouseHotSpotY::Middle;
			}
			else
			{
				return false;
			}
			return true;
		}
		return false;
	}
};
