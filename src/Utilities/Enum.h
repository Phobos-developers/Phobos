#pragma once

#include "./../Phobos.h"

enum class SuperWeaponAITargetingMode {
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

enum class SuperWeaponTarget : unsigned char {
	None = 0x0,
	Land = 0x1,
	Water = 0x2,
	NoContent = 0x4,
	Infantry = 0x8,
	Unit = 0x10,
	Building = 0x20,

	All = 0xFF,
	AllCells = Land | Water,
	AllTechnos = Infantry | Unit | Building,
	AllContents = NoContent | AllTechnos
};

MAKE_ENUM_FLAGS(SuperWeaponTarget);

enum class SuperWeaponAffectedHouse : unsigned char {
	None = 0x0,
	Owner = 0x1,
	Allies = 0x2,
	Enemies = 0x4,

	Team = Owner | Allies,
	NotAllies = Owner | Enemies,
	NotOwner = Allies | Enemies,
	All = Owner | Allies | Enemies
};

MAKE_ENUM_FLAGS(SuperWeaponAffectedHouse);

enum class OwnerHouseKind : int {
	Default,
	Invoker,
	Killer,
	Victim,
	Civilian,
	Special,
	Neutral,
	Random
};

enum class SuperWeaponFlags : unsigned short {
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

enum class AresAction {
	None = 0,
	Hijack = 1,
	Drive = 2
};

class MouseCursorHotSpotX {
public:
	typedef MouseHotSpotX Value;

	static bool Parse(char* key, Value* value) {
		if (key && value) {
			if (!_strcmpi(key, "left")) {
				*value = MouseHotSpotX::Left;
			}
			else if (!_strcmpi(key, "right")) {
				*value = MouseHotSpotX::Right;
			}
			else if (!_strcmpi(key, "center")) {
				*value = MouseHotSpotX::Center;
			}
			else {
				return false;
			}
			return true;
		}
		return false;
	}
};

class MouseCursorHotSpotY {
public:
	typedef MouseHotSpotY Value;

	static bool Parse(char* key, Value* value) {
		if (key && value) {
			if (!_strcmpi(key, "top")) {
				*value = MouseHotSpotY::Top;
			}
			else if (!_strcmpi(key, "bottom")) {
				*value = MouseHotSpotY::Bottom;
			}
			else if (!_strcmpi(key, "middle")) {
				*value = MouseHotSpotY::Middle;
			}
			else {
				return false;
			}
			return true;
		}
		return false;
	}
};
