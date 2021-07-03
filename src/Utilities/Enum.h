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

#include <Phobos.h>

enum class AttachedAnimFlag {
	None = 0x0,
	Hides = 0x1,
	Temporal = 0x2,
	Paused = 0x4,

	PausedTemporal = Paused | Temporal
};

MAKE_ENUM_FLAGS(AttachedAnimFlag);

enum class AirAttackStatus
{
	ValidateAZ = 0,
	PickAttackLocation = 1,
	TakeOff = 2,
	FlyToPosition = 3,
	FireAtTarget = 4,
	FireAtTarget2 = 5,
	FireAtTarget2_Strafe = 6,
	FireAtTarget3_Strafe = 7,
	FireAtTarget4_Strafe = 8,
	FireAtTarget5_Strafe = 9,
	ReturnToBase = 10
};

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

enum class AffectedTarget : unsigned char {
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

MAKE_ENUM_FLAGS(AffectedTarget);

enum class AffectedHouse : unsigned char {
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

enum class PhobosAction {
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
