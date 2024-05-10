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

#include <Windows.h>

#include "Template.h"

#include "INIParser.h"
#include "Enum.h"
#include "Constructs.h"
#include "SavegameDef.h"

#include <InfantryTypeClass.h>
#include <AircraftTypeClass.h>
#include <UnitTypeClass.h>
#include <BuildingTypeClass.h>
#include <WarheadTypeClass.h>
#include <FootClass.h>
#include <VocClass.h>
#include <VoxClass.h>
#include <CRT.h>
#include <LocomotionClass.h>

#include <Locomotion/TestLocomotionClass.h>
#include <Locomotion/AttachmentLocomotionClass.h>

namespace detail
{
	template<typename T>
	concept HasFindOrAllocate = requires(const char* arg){
		{ T::FindOrAllocate(arg) }->std::same_as<T*>;
	};

	template <typename T, bool allocate = false>
	inline bool read(T &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		if (parser.ReadString(pSection, pKey))
		{
			using base_type = std::remove_pointer_t<T>;
			auto const pValue = parser.value();
			T parsed;
			if constexpr (HasFindOrAllocate<base_type> && allocate)
				parsed = base_type::FindOrAllocate(pValue);
			else
				parsed = base_type::Find(pValue);

			if (parsed || INIClass::IsBlank(pValue))
			{
				value = parsed;
				return true;
			}
			else
			{
				Debug::INIParseFailed(pSection, pKey, pValue);
			}
		}

		return false;
	}

	template <>
	inline bool read<bool>(bool &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		bool buffer;

		if (parser.ReadBool(pSection, pKey, &buffer))
		{
			value = buffer;
			return true;
		}
		else if (!parser.empty())
		{
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid boolean value [1, true, yes, 0, false, no]");
		}

		return false;
	}

	template <>
	inline bool read<int>(int &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		int buffer;

		if (parser.ReadInteger(pSection, pKey, &buffer))
		{
			value = buffer;
			return true;
		}
		else if (!parser.empty())
		{
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid number");
		}

		return false;
	}

	template <>
	inline bool read<ArmorType>(ArmorType &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		int buffer = value;

		// Hack cause armor type parser in Ares will return 0 (ArmorType 'none') if armor type is not found instead of -1.
		if (parser.ReadString(pSection, pKey))
		{
			if (!parser.ReadArmor(pSection, pKey, &buffer) || buffer < 0)
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid ArmorType");
				return false;
			}

			value = buffer;
			return true;
		}

		return false;
	}

	template <>
	inline bool read<unsigned short>(unsigned short &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		int buffer;

		if (parser.ReadInteger(pSection, pKey, &buffer))
		{
			value = static_cast<unsigned short>(buffer);
			return true;
		}

		return false;
	}

	template <>
	inline bool read<BYTE>(BYTE &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		int buffer;

		if (parser.ReadInteger(pSection, pKey, &buffer))
		{
			if (buffer <= 255 && buffer >= 0)
			{
				value = static_cast<BYTE>(buffer); // shut up shut up shut up C4244
				return true;
			}
			else
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid number between 0 and 255 inclusive.");
			}
		}
		else if (!parser.empty())
		{
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid number");
		}

		return false;
	}

	template <>
	inline bool read<float>(float &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		double buffer;

		if (parser.ReadDouble(pSection, pKey, &buffer))
		{
			value = static_cast<float>(buffer);
			return true;
		}
		else if (!parser.empty())
		{
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid floating point number");
		}

		return false;
	}

	template <>
	inline bool read<double>(double &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		double buffer;

		if (parser.ReadDouble(pSection, pKey, &buffer))
		{
			value = buffer;
			return true;
		}
		else if (!parser.empty())
		{
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid floating point number");
		}

		return false;
	}

	template <>
	inline bool read<Point2D>(Point2D &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		if (parser.Read2Integers(pSection, pKey, (int *)&value))
			return true;

		return false;
	}

	template <>
	inline bool read<Vector2D<double>>(Vector2D<double> &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		if (parser.Read2Doubles(pSection, pKey, (double*)&value))
			return true;

		return false;
	}

	template <>
	inline bool read<CoordStruct>(CoordStruct &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		if (parser.Read3Integers(pSection, pKey, (int*)&value))
			return true;

		return false;
	}

	template <>
	inline bool read<ColorStruct>(ColorStruct &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		ColorStruct buffer;

		if (parser.Read3Bytes(pSection, pKey, reinterpret_cast<byte*>(&buffer)))
		{
			value = buffer;
			return true;
		}
		else if (!parser.empty())
		{
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid R,G,B color");
		}

		return false;
	}

	template <>
	inline bool read<PartialVector2D<int>>(PartialVector2D<int> &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		value.ValueCount = parser.ReadMultipleIntegers(pSection, pKey, (int*)&value, 2);

		if (value.ValueCount > 0)
			return true;

		return false;
	}

	template <>
	inline bool read<PartialVector2D<double>>(PartialVector2D<double> &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		value.ValueCount = parser.ReadMultipleDoubles(pSection, pKey, (double*)&value, 2);

		if (value.ValueCount > 0)
			return true;

		return false;
	}

	template <>
	inline bool read<PartialVector3D<int>>(PartialVector3D<int> &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		value.ValueCount = parser.ReadMultipleIntegers(pSection, pKey, (int *)&value, 3);

		if (value.ValueCount > 0)
			return true;

		return false;
	}

	template <>
	inline bool read<PartialVector3D<double>>(PartialVector3D<double> &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		value.ValueCount = parser.ReadMultipleDoubles(pSection, pKey, (double*)&value, 3);

		if (value.ValueCount > 0)
			return true;

		return false;
	}

	template <>
	inline bool read<CSFText>(CSFText &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		if (parser.ReadString(pSection, pKey))
		{
			value = parser.value();
			return true;
		}

		return false;
	}

	template <>
	inline bool read<SHPStruct *>(SHPStruct *&value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		if (parser.ReadString(pSection, pKey))
		{

			auto const pValue = parser.value();
			std::string Result = pValue;

			if (!strstr(pValue, ".shp"))
				Result += ".shp";

			if (auto const pImage = FileSystem::LoadSHPFile(Result.c_str()))
			{
				value = pImage;
				return true;
			}
			else
			{
				Debug::Log("Failed to find file %s referenced by [%s]%s=%s\n", Result.c_str(), pSection, pKey, pValue);
			}
		}

		return false;
	}

	template <>
	inline bool read<MouseCursor>(MouseCursor &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		auto ret = false;

		// compact way to define the cursor in one go
		if (parser.ReadString(pSection, pKey))
		{
			auto const buffer = parser.value();
			char* context = nullptr;

			if (auto const pFrame = strtok_s(buffer, Phobos::readDelims, &context))
				Parser<int>::Parse(pFrame, &value.Frame);
			if (auto const pCount = strtok_s(nullptr, Phobos::readDelims, &context))
				Parser<int>::Parse(pCount, &value.Count);
			if (auto const pInterval = strtok_s(nullptr, Phobos::readDelims, &context))
				Parser<int>::Parse(pInterval, &value.Interval);
			if (auto const pFrame = strtok_s(nullptr, Phobos::readDelims, &context))
				Parser<int>::Parse(pFrame, &value.MiniFrame);
			if (auto const pCount = strtok_s(nullptr, Phobos::readDelims, &context))
				Parser<int>::Parse(pCount, &value.MiniCount);
			if (auto const pHotX = strtok_s(nullptr, Phobos::readDelims, &context))
				MouseCursorHotSpotX::Parse(pHotX, &value.HotX);
			if (auto const pHotY = strtok_s(nullptr, Phobos::readDelims, &context))
				MouseCursorHotSpotY::Parse(pHotY, &value.HotY);

			ret = true;
		}

		char pFlagName[32];
		_snprintf_s(pFlagName, 31, "%s.Frame", pKey);
		ret |= read(value.Frame, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 31, "%s.Count", pKey);
		ret |= read(value.Count, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 31, "%s.Interval", pKey);
		ret |= read(value.Interval, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 31, "%s.MiniFrame", pKey);
		ret |= read(value.MiniFrame, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 31, "%s.MiniCount", pKey);
		ret |= read(value.MiniCount, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 31, "%s.HotSpot", pKey);

		if (parser.ReadString(pSection, pFlagName))
		{
			auto const pValue = parser.value();
			char* context = nullptr;
			auto const pHotX = strtok_s(pValue, ",", &context);
			MouseCursorHotSpotX::Parse(pHotX, &value.HotX);

			if (auto const pHotY = strtok_s(nullptr, ",", &context))
				MouseCursorHotSpotY::Parse(pHotY, &value.HotY);

			ret = true;
		}

		return ret;
	}

	template <>
	inline bool read<RocketStruct>(RocketStruct &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		auto ret = false;

		char pFlagName[0x40];
		_snprintf_s(pFlagName, 0x3F, "%s.PauseFrames", pKey);
		ret |= read(value.PauseFrames, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 0x3F, "%s.TiltFrames", pKey);
		ret |= read(value.TiltFrames, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 0x3F, "%s.PitchInitial", pKey);
		ret |= read(value.PitchInitial, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 0x3F, "%s.PitchFinal", pKey);
		ret |= read(value.PitchFinal, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 0x3F, "%s.TurnRate", pKey);
		ret |= read(value.TurnRate, parser, pSection, pFlagName);

		// sic! integer read like a float.
		_snprintf_s(pFlagName, 0x3F, "%s.RaiseRate", pKey);
		float buffer;
		if (read(buffer, parser, pSection, pFlagName))
		{
			value.RaiseRate = Game::F2I(buffer);
			ret = true;
		}

		_snprintf_s(pFlagName, 0x3F, "%s.Acceleration", pKey);
		ret |= read(value.Acceleration, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 0x3F, "%s.Altitude", pKey);
		ret |= read(value.Altitude, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 0x3F, "%s.Damage", pKey);
		ret |= read(value.Damage, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 0x3F, "%s.EliteDamage", pKey);
		ret |= read(value.EliteDamage, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 0x3F, "%s.BodyLength", pKey);
		ret |= read(value.BodyLength, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 0x3F, "%s.LazyCurve", pKey);
		ret |= read(value.LazyCurve, parser, pSection, pFlagName);

		_snprintf_s(pFlagName, 0x3F, "%s.Type", pKey);
		ret |= read(value.Type, parser, pSection, pFlagName);

		return ret;
	}

	template <>
	inline bool read<Leptons>(Leptons &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		double buffer;

		if (parser.ReadDouble(pSection, pKey, &buffer))
		{
			value = Leptons(Game::F2I(buffer * Unsorted::LeptonsPerCell));
			return true;
		}
		else if (!parser.empty())
		{
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid floating point number");
		}

		return false;
	}

	template <>
	inline bool read<OwnerHouseKind>(OwnerHouseKind &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		if (parser.ReadString(pSection, pKey))
		{
			if (_strcmpi(parser.value(), "default") == 0)
			{
				value = OwnerHouseKind::Default;
			}
			else if (_strcmpi(parser.value(), "invoker") == 0)
			{
				value = OwnerHouseKind::Invoker;
			}
			else if (_strcmpi(parser.value(), "killer") == 0)
			{
				value = OwnerHouseKind::Killer;
			}
			else if (_strcmpi(parser.value(), "victim") == 0)
			{
				value = OwnerHouseKind::Victim;
			}
			else if (_strcmpi(parser.value(), "civilian") == 0)
			{
				value = OwnerHouseKind::Civilian;
			}
			else if (_strcmpi(parser.value(), "special") == 0)
			{
				value = OwnerHouseKind::Special;
			}
			else if (_strcmpi(parser.value(), "neutral") == 0)
			{
				value = OwnerHouseKind::Neutral;
			}
			else if (_strcmpi(parser.value(), "random") == 0)
			{
				value = OwnerHouseKind::Random;
			}
			else
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a owner house kind");
				return false;
			}

			return true;
		}

		return false;
	}

	template <>
	inline bool read<Mission>(Mission &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto const mission = MissionControlClass::FindIndex(parser.value());

			if (mission != Mission::None)
			{
				value = mission;
				return true;
			}
			else if (!parser.empty())
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Invalid Mission name");
			}
		}

		return false;
	}

	template <>
	inline bool read<DirType>(DirType &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		int buffer;

		if (parser.ReadInteger(pSection, pKey, &buffer))
		{
			unsigned int absValue = abs(buffer);
			bool isNegative = buffer < 0;

			if ((int)DirType::North <= absValue && absValue <= (int)DirType::Max)
			{
				value = static_cast<DirType>(!isNegative ? absValue : (int)DirType::Max - absValue);
				return true;
			}
			else
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid DirType (0-255 abs. value).");
			}
		}

		return false;
	}

	template <>
	inline bool read<FacingType>(FacingType &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		int buffer;

		if (parser.ReadInteger(pSection, pKey, &buffer))
		{
			if (buffer < (int)FacingType::Count && buffer >= (int)FacingType::None)
			{
				value = static_cast<FacingType>(buffer);
				return true;
			}
			else
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid FacingType (0-7 or -1).");
			}
		}

		return false;
	}

	template <>
	inline bool read<SuperWeaponAITargetingMode>(SuperWeaponAITargetingMode &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		if (parser.ReadString(pSection, pKey))
		{
			static const auto Modes = {
				"none", "nuke", "lightningstorm", "psychicdominator", "paradrop",
				"geneticmutator", "forceshield", "notarget", "offensive", "stealth",
				"self", "base", "multimissile", "hunterseeker", "enemybase" };

			auto it = Modes.begin();

			for (auto i = 0u; i < Modes.size(); ++i)
			{
				if (_strcmpi(parser.value(), *it++) == 0)
				{
					value = static_cast<SuperWeaponAITargetingMode>(i);
					return true;
				}
			}

			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a targeting mode");
		}

		return false;
	}

	template <>
	inline bool read<AffectedTarget>(AffectedTarget &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto parsed = AffectedTarget::None;
			auto str = parser.value();
			char* context = nullptr;

			for (auto cur = strtok_s(str, Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				if (!_strcmpi(cur, "land"))
				{
					parsed |= AffectedTarget::Land;
				}
				else if (!_strcmpi(cur, "water"))
				{
					parsed |= AffectedTarget::Water;
				}
				else if (!_strcmpi(cur, "empty"))
				{
					parsed |= AffectedTarget::NoContent;
				}
				else if (!_strcmpi(cur, "infantry"))
				{
					parsed |= AffectedTarget::Infantry;
				}
				else if (!_strcmpi(cur, "units"))
				{
					parsed |= AffectedTarget::Unit;
				}
				else if (!_strcmpi(cur, "buildings"))
				{
					parsed |= AffectedTarget::Building;
				}
				else if (!_strcmpi(cur, "aircraft"))
				{
					parsed |= AffectedTarget::Aircraft;
				}
				else if (!_strcmpi(cur, "all"))
				{
					parsed |= AffectedTarget::All;
				}
				else if (_strcmpi(cur, "none"))
				{
					Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a super weapon target");
					return false;
				}
			}

			value = parsed;
			return true;
		}

		return false;
	}

	template <>
	inline bool read<AffectedHouse>(AffectedHouse &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto parsed = AffectedHouse::None;

			auto str = parser.value();
			char* context = nullptr;
			for (auto cur = strtok_s(str, Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				if (!_strcmpi(cur, "owner") || !_strcmpi(cur, "self"))
				{
					parsed |= AffectedHouse::Owner;
				}
				else if (!_strcmpi(cur, "allies") || !_strcmpi(cur, "ally"))
				{
					parsed |= AffectedHouse::Allies;
				}
				else if (!_strcmpi(cur, "enemies") || !_strcmpi(cur, "enemy"))
				{
					parsed |= AffectedHouse::Enemies;
				}
				else if (!_strcmpi(cur, "team"))
				{
					parsed |= AffectedHouse::Team;
				}
				else if (!_strcmpi(cur, "others"))
				{
					parsed |= AffectedHouse::NotOwner;
				}
				else if (!_strcmpi(cur, "all"))
				{
					parsed |= AffectedHouse::All;
				}
				else if (_strcmpi(cur, "none"))
				{
					Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a super weapon affected house");
					return false;
				}
			}

			value = parsed;
			return true;
		}

		return false;
	}

	template <>
	inline bool read<AttachedAnimFlag>(AttachedAnimFlag &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto parsed = AttachedAnimFlag::None;

			auto str = parser.value();

			if (_strcmpi(str, "hides") == 0)
			{
				parsed = AttachedAnimFlag::Hides;
			}
			else if (_strcmpi(str, "temporal") == 0)
			{
				parsed = AttachedAnimFlag::Temporal;
			}
			else if (_strcmpi(str, "paused") == 0)
			{
				parsed = AttachedAnimFlag::Paused;
			}
			else if (_strcmpi(str, "pausedtemporal") == 0)
			{
				parsed = AttachedAnimFlag::PausedTemporal;
			}
			else if (_strcmpi(str, "none"))
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a AttachedAnimFlag");
				return false;
			}

			value = parsed;
			return true;
		}

		return false;
	}

	template <>
	inline bool read<AreaFireTarget>(AreaFireTarget &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		if (parser.ReadString(pSection, pKey))
		{
			if (_strcmpi(parser.value(), "base") == 0)
			{
				value = AreaFireTarget::Base;
			}
			else if (_strcmpi(parser.value(), "self") == 0)
			{
				value = AreaFireTarget::Self;
			}
			else if (_strcmpi(parser.value(), "random") == 0)
			{
				value = AreaFireTarget::Random;
			}
			else
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected an area fire target");
				return false;
			}

			return true;
		}

		return false;
	}

	template <>
	inline bool read<SelfHealGainType>(SelfHealGainType &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		if (parser.ReadString(pSection, pKey))
		{
			if (_strcmpi(parser.value(), "none") == 0)
			{
				value = SelfHealGainType::None;
			}
			else if (_strcmpi(parser.value(), "infantry") == 0)
			{
				value = SelfHealGainType::Infantry;
			}
			else if (_strcmpi(parser.value(), "units") == 0)
			{
				value = SelfHealGainType::Units;
			}
			else
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a self heal gain type");
				return false;
			}

			return true;
		}

		return false;
	}

	template <>
	inline bool read<SlaveChangeOwnerType>(SlaveChangeOwnerType &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		if (parser.ReadString(pSection, pKey))
		{
			if (_strcmpi(parser.value(), "suicide") == 0)
			{
				value = SlaveChangeOwnerType::Suicide;
			}
			else if (_strcmpi(parser.value(), "master") == 0)
			{
				value = SlaveChangeOwnerType::Master;
			}
			else if (_strcmpi(parser.value(), "neutral") == 0)
			{
				value = SlaveChangeOwnerType::Neutral;
			}
			else
			{
				if (_strcmpi(parser.value(), "killer") != 0)
					Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a slave ownership option, default killer");
				value = SlaveChangeOwnerType::Killer;
			}

			return true;
		}

		return false;
	}

	template <>
	inline bool read<AutoDeathBehavior>(AutoDeathBehavior &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		if (parser.ReadString(pSection, pKey))
		{
			if (_strcmpi(parser.value(), "sell") == 0)
			{
				value = AutoDeathBehavior::Sell;
			}
			else if (_strcmpi(parser.value(), "vanish") == 0)
			{
				value = AutoDeathBehavior::Vanish;
			}
			else
			{
				if (_strcmpi(parser.value(), "kill") != 0)
					Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a self-destruction behavior, default to kill if set");
				value = AutoDeathBehavior::Kill;
			}

			return true;
		}

		return false;
	}

	template <>
	inline bool read<TextAlign>(TextAlign &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto parsed = TextAlign::None;
			auto str = parser.value();

			if (_strcmpi(str, "left") == 0)
			{
				parsed = TextAlign::Left;
			}
			else if (_strcmpi(str, "center") == 0)
			{
				parsed = TextAlign::Center;
			}
			else if (_strcmpi(str, "centre") == 0)
			{
				parsed = TextAlign::Center;
			}
			else if (_strcmpi(str, "right") == 0)
			{
				parsed = TextAlign::Right;
			}
			else if (_strcmpi(str, "none") == 0)
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Text Alignment can be either Left, Center/Centre or Right");
				return false;
			}

			if (parsed != TextAlign::None)
				value = parsed;

			return true;
		}

		return false;
	}

	template <>
	inline bool read<TranslucencyLevel>(TranslucencyLevel &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		return value.Read(parser, pSection, pKey);
	}


	template <>
	inline bool read<IronCurtainEffect>(IronCurtainEffect &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto parsed = IronCurtainEffect::Kill;
			auto str = parser.value();

			if (_strcmpi(str, "invulnerable") == 0)
			{
				parsed = IronCurtainEffect::Invulnerable;
			}
			else if (_strcmpi(str, "ignore") == 0)
			{
				parsed = IronCurtainEffect::Ignore;
			}
			else if (_strcmpi(str, "kill") != 0)
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), "IronCurtainEffect can be either kill, invulnerable or ignore");
				return false;
			}

			value = parsed;
			return true;
		}

		return false;
	}

	template <>
	inline bool read<TargetZoneScanType>(TargetZoneScanType &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		if (parser.ReadString(pSection, pKey))
		{
			if (_strcmpi(parser.value(), "same") == 0)
			{
				value = TargetZoneScanType::Same;
			}
			else if (_strcmpi(parser.value(), "any") == 0)
			{
				value = TargetZoneScanType::Any;
			}
			else if (_strcmpi(parser.value(), "inrange") == 0)
			{
				value = TargetZoneScanType::InRange;
			}
			else
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a target zone scan type");
				return false;
			}

			return true;
		}

		return false;
	}

	template <>
	inline bool read<ChronoSparkleDisplayPosition>(ChronoSparkleDisplayPosition &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto parsed = ChronoSparkleDisplayPosition::None;

			auto str = parser.value();
			char* context = nullptr;
			for (auto cur = strtok_s(str, Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				if (!_strcmpi(cur, "building"))
				{
					parsed |= ChronoSparkleDisplayPosition::Building;
				}
				else if (!_strcmpi(cur, "occupants"))
				{
					parsed |= ChronoSparkleDisplayPosition::Occupants;
				}
				else if (!_strcmpi(cur, "occupantslots"))
				{
					parsed |= ChronoSparkleDisplayPosition::OccupantSlots;
				}
				else if (!_strcmpi(cur, "all") )
				{
					parsed |= ChronoSparkleDisplayPosition::All;
				}
				else if (_strcmpi(cur, "none"))
				{
					Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a chrono sparkle position type");
					return false;
				}
			}

			value = parsed;
			return true;
		}

		return false;
	}

	template <>
	inline bool read<AttachmentTimerConversionMode>(AttachmentTimerConversionMode& value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		if (parser.ReadString(pSection, pKey))
		{
			bool success;
			value = ParseEnum<AttachmentTimerConversionMode>(parser.value(), success);
			if(!success)
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a time conversion mode, use default value.");
			return success;
		}
		return false;
	}

	template <>
	inline bool read<AttachmentInstanceConversionMode>(AttachmentInstanceConversionMode& value, INI_EX& parser, const char* pSection, const char* pKey)
	{
		if (parser.ReadString(pSection, pKey))
		{
			bool success;
			value = ParseEnum<AttachmentInstanceConversionMode>(parser.value(), success);
			if (!success)
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a instance conversion mode, use default value.");
			return success;
		}
		return false;
	}

	template <>
	inline bool read<Layer>(Layer& value, INI_EX& parser, const char* pSection, const char* pKey)
	{
		if (parser.ReadString(pSection, pKey))
		{
			bool success;
			value = ParseEnum<Layer>(parser.value(), success);
			if (!success)
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a layer value, use default value.");
			return success;
		}
		return false;
	}

	template <>
	inline bool read<CLSID>(CLSID &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		if (!parser.ReadString(pSection, pKey))
			return false;

		// Semantic locomotor aliases
		if (parser.value()[0] != '{')
		{
#define PARSE_IF_IS_LOCO(name)\
if(_strcmpi(parser.value(), #name) == 0){ value = LocomotionClass::CLSIDs::name; return true; }

			PARSE_IF_IS_LOCO(Drive);
			PARSE_IF_IS_LOCO(Jumpjet);
			PARSE_IF_IS_LOCO(Hover);
			PARSE_IF_IS_LOCO(Rocket);
			PARSE_IF_IS_LOCO(Tunnel);
			PARSE_IF_IS_LOCO(Walk);
			PARSE_IF_IS_LOCO(Fly);
			PARSE_IF_IS_LOCO(Teleport);
			PARSE_IF_IS_LOCO(Mech);
			PARSE_IF_IS_LOCO(Ship);
			PARSE_IF_IS_LOCO(Droppod);

#undef PARSE_IF_IS_LOCO

#define PARSE_IF_IS_PHOBOS_LOCO(name)\
if(_strcmpi(parser.value(), #name) == 0){ value = __uuidof(name ## LocomotionClass); return true; }

		// Add your locomotor parsing here
#ifdef CUSTOM_LOCO_EXAMPLE_ENABLED // Add semantic parsing for loco
			PARSE_IF_IS_PHOBOS_LOCO(Test);
#endif
			PARSE_IF_IS_PHOBOS_LOCO(Attachment);

#undef PARSE_IF_IS_PHOBOS_LOCO

			return false;
		}

		CHAR bytestr[128];
		WCHAR wcharstr[128];

		strncpy(bytestr, parser.value(), 128);
		bytestr[127] = NULL;
		CRT::strtrim(bytestr);
		if (!strlen(bytestr))
			return false;

		MultiByteToWideChar(0, 1, bytestr, -1, wcharstr, 128);
		if (CLSIDFromString(wcharstr, &value) < 0)
			return false;

		return true;
	}

	template <>
	inline bool read<HorizontalPosition>(HorizontalPosition &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto str = parser.value();
			if (_strcmpi(str, "left") == 0)
			{
				value = HorizontalPosition::Left;
			}
			else if (_strcmpi(str, "center") == 0 || _strcmpi(str, "centre") == 0)
			{
				value = HorizontalPosition::Center;
			}
			else if (_strcmpi(str, "right") == 0)
			{
				value = HorizontalPosition::Right;
			}
			else
			{
				Debug::INIParseFailed(pSection, pKey, str, "Horizontal Position can be either Left, Center/Centre or Right");
				return false;
			}
			return true;
		}
		return false;
	}

	template <>
	inline bool read<VerticalPosition>(VerticalPosition &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto str = parser.value();
			if (_strcmpi(str, "top") == 0)
			{
				value = VerticalPosition::Top;
			}
			else if (_strcmpi(str, "center") == 0 || _strcmpi(str, "centre") == 0)
			{
				value = VerticalPosition::Center;
			}
			else if (_strcmpi(str, "bottom") == 0)
			{
				value = VerticalPosition::Bottom;
			}
			else
			{
				Debug::INIParseFailed(pSection, pKey, str, "Vertical Position can be either Top, Center/Centre or Bottom");
				return false;
			}
			return true;
		}
		return false;
	}

	template <>
	inline bool read<BuildingSelectBracketPosition>(BuildingSelectBracketPosition &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto str = parser.value();
			if (_strcmpi(str, "top") == 0)
			{
				value = BuildingSelectBracketPosition::Top;
			}
			else if (_strcmpi(str, "lefttop") == 0)
			{
				value = BuildingSelectBracketPosition::LeftTop;
			}
			else if (_strcmpi(str, "leftbottom") == 0)
			{
				value = BuildingSelectBracketPosition::LeftBottom;
			}
			else if (_strcmpi(str, "bottom") == 0)
			{
				value = BuildingSelectBracketPosition::Bottom;
			}
			else if (_strcmpi(str, "rightbottom") == 0)
			{
				value = BuildingSelectBracketPosition::RightBottom;
			}
			else if (_strcmpi(str, "righttop") == 0)
			{
				value = BuildingSelectBracketPosition::RightTop;
			}
			else
			{
				Debug::INIParseFailed(pSection, pKey, str, "BuildingPosition is invalid");
				return false;
			}
			return true;
		}

		return false;
	}

	template <>
	inline bool read<DisplayInfoType>(DisplayInfoType &value, INI_EX &parser, const char *pSection, const char *pKey)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto str = parser.value();
			if (_strcmpi(str, "health") == 0)
			{
				value = DisplayInfoType::Health;
			}
			else if (_strcmpi(str, "shield") == 0)
			{
				value = DisplayInfoType::Shield;
			}
			else if (_strcmpi(str, "ammo") == 0)
			{
				value = DisplayInfoType::Ammo;
			}
			else if (_strcmpi(str, "mindcontrol") == 0)
			{
				value = DisplayInfoType::MindControl;
			}
			else if (_strcmpi(str, "spawns") == 0)
			{
				value = DisplayInfoType::Spawns;
			}
			else if (_strcmpi(str, "passengers") == 0)
			{
				value = DisplayInfoType::Passengers;
			}
			else if (_strcmpi(str, "tiberium") == 0)
			{
				value = DisplayInfoType::Tiberium;
			}
			else if (_strcmpi(str, "experience") == 0)
			{
				value = DisplayInfoType::Experience;
			}
			else if (_strcmpi(str, "occupants") == 0)
			{
				value = DisplayInfoType::Occupants;
			}
			else if (_strcmpi(str, "gattlingstage") == 0)
			{
				value = DisplayInfoType::GattlingStage;
			}
			else
			{
				Debug::INIParseFailed(pSection, pKey, str, "Display info type is invalid");
				return false;
			}
			return true;
		}
		return false;
	}

	template <>
	inline bool read<AttachmentYSortPosition>(AttachmentYSortPosition& value, INI_EX& parser, const char* pSection, const char* pKey)
	{
		if (parser.ReadString(pSection, pKey))
		{
			if (_strcmpi(parser.value(), "default") == 0)
			{
				value = AttachmentYSortPosition::Default;
			}
			else if (_strcmpi(parser.value(), "underparent") == 0)
			{
				value = AttachmentYSortPosition::UnderParent;
			}
			else if (_strcmpi(parser.value(), "overparent") == 0)
			{
				value = AttachmentYSortPosition::OverParent;
			}
			else
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected an attachment YSort position");
				return false;
			}
			return true;
		}
		return false;
	}

	template <typename T>
	void parse_values(std::vector<T>& vector, INI_EX& parser, const char* pSection, const char* pKey)
	{
		char* context = nullptr;

		for (auto pCur = strtok_s(parser.value(), Phobos::readDelims, &context); pCur; pCur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			T buffer = T();

			if (Parser<T>::Parse(pCur, &buffer))
				vector.push_back(buffer);
			else if (!INIClass::IsBlank(pCur))
				Debug::INIParseFailed(pSection, pKey, pCur);
		}
	}

	template <typename Lookuper, typename T>
	void parse_indexes(std::vector<T>& vector, INI_EX& parser, const char* pSection, const char* pKey)
	{
		char* context = nullptr;

		for (auto pCur = strtok_s(parser.value(), Phobos::readDelims, &context); pCur; pCur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			int idx = Lookuper::FindIndex(pCur);

			if (idx != -1)
				vector.push_back(idx);
			else if (!INIClass::IsBlank(pCur))
				Debug::INIParseFailed(pSection, pKey, pCur);
		}
	}
}


// Valueable

template <typename T>
template <bool Allocate>
void __declspec(noinline) Valueable<T>::Read(INI_EX &parser, const char *pSection, const char *pKey)
{
	detail::read<T,Allocate>(this->Value, parser, pSection, pKey);
}

template <typename T>
bool Valueable<T>::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Savegame::ReadPhobosStream(Stm, this->Value, RegisterForChange);
}

template <typename T>
bool Valueable<T>::Save(PhobosStreamWriter& Stm) const
{
	return Savegame::WritePhobosStream(Stm, this->Value);
}


// ValueableIdx

template <typename Lookuper>
void __declspec(noinline) ValueableIdx<Lookuper>::Read(INI_EX& parser, const char* pSection, const char* pKey)
{
	if (parser.ReadString(pSection, pKey))
	{
		const char* val = parser.value();
		int idx = Lookuper::FindIndex(val);

		if (idx != -1 || INIClass::IsBlank(val))
			this->Value = idx;
		else
			Debug::INIParseFailed(pSection, pKey, val);
	}
}


// Nullable

template <typename T>
template <bool Allocate>
void __declspec(noinline) Nullable<T>::Read(INI_EX& parser, const char* pSection, const char* pKey)
{
	if (parser.ReadString(pSection, pKey))
	{
		const char* val = parser.value();

		if (!_strcmpi(val, "<default>") || INIClass::IsBlank(val))
			this->Reset();
		else if (detail::read<T, Allocate>(this->Value, parser, pSection, pKey))
			this->HasValue = true;
	}
}

template <typename T>
bool Nullable<T>::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->Reset();
	auto ret = Savegame::ReadPhobosStream(Stm, this->HasValue);

	if (ret && this->HasValue)
		ret = Savegame::ReadPhobosStream(Stm, this->Value, RegisterForChange);

	return ret;
}

template <typename T>
bool Nullable<T>::Save(PhobosStreamWriter& Stm) const
{
	auto ret = Savegame::WritePhobosStream(Stm, this->HasValue);

	if (this->HasValue)
		ret = Savegame::WritePhobosStream(Stm, this->Value);

	return ret;
}


// NullableIdx

template <typename Lookuper>
void __declspec(noinline) NullableIdx<Lookuper>::Read(INI_EX& parser, const char* pSection, const char* pKey)
{
	if (parser.ReadString(pSection, pKey))
	{
		const char* val = parser.value();
		int idx = Lookuper::FindIndex(val);

		if (idx != -1 || INIClass::IsBlank(val))
		{
			this->Value = idx;
			this->HasValue = true;
		}
		else
		{
			Debug::INIParseFailed(pSection, pKey, val);
		}
	}
}


// Promotable

template <typename T>
void __declspec(noinline) Promotable<T>::Read(INI_EX& parser, const char* const pSection, const char* const pBaseFlag, const char* const pSingleFlag)
{

	// read the common flag, with the trailing dot being stripped
	char flagName[0x40];
	auto const pSingleFormat = pSingleFlag ? pSingleFlag : pBaseFlag;
	auto res = _snprintf_s(flagName, _TRUNCATE, pSingleFormat, "");
	if (res > 0 && flagName[res - 1] == '.')
	{
		flagName[res - 1] = '\0';
	}

	T placeholder;
	if (detail::read(placeholder, parser, pSection, flagName))
		this->SetAll(placeholder);

	// read specific flags
	_snprintf_s(flagName, _TRUNCATE, pBaseFlag, "Rookie");
	detail::read(this->Rookie, parser, pSection, flagName);

	_snprintf_s(flagName, _TRUNCATE, pBaseFlag, "Veteran");
	detail::read(this->Veteran, parser, pSection, flagName);

	_snprintf_s(flagName, _TRUNCATE, pBaseFlag, "Elite");
	detail::read(this->Elite, parser, pSection, flagName);
};

template <typename T>
bool Promotable<T>::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Savegame::ReadPhobosStream(Stm, this->Rookie, RegisterForChange)
		&& Savegame::ReadPhobosStream(Stm, this->Veteran, RegisterForChange)
		&& Savegame::ReadPhobosStream(Stm, this->Elite, RegisterForChange);
}

template <typename T>
bool Promotable<T>::Save(PhobosStreamWriter& Stm) const
{
	return Savegame::WritePhobosStream(Stm, this->Rookie)
		&& Savegame::WritePhobosStream(Stm, this->Veteran)
		&& Savegame::WritePhobosStream(Stm, this->Elite);
}


// ValueableVector

template <typename T>
void __declspec(noinline) ValueableVector<T>::Read(INI_EX& parser, const char* pSection, const char* pKey)
{
	if (parser.ReadString(pSection, pKey))
	{
		this->clear();
		detail::parse_values<T>(*this, parser, pSection, pKey);
	}
}

template <typename T>
bool ValueableVector<T>::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	size_t size = 0;
	if (Savegame::ReadPhobosStream(Stm, size, RegisterForChange))
	{
		this->clear();
		this->reserve(size);

		for (size_t i = 0; i < size; ++i)
		{
			value_type buffer = value_type();
			Savegame::ReadPhobosStream(Stm, buffer, false);
			this->push_back(std::move(buffer));

			if (RegisterForChange)
				Swizzle swizzle(this->back());
		}

		return true;
	}

	return false;
}

template <>
inline bool ValueableVector<bool>::Load(PhobosStreamReader& stm, bool registerForChange)
{
	size_t size = 0;
	if (Savegame::ReadPhobosStream(stm, size, registerForChange))
	{
		this->clear();

		for (size_t i = 0; i < size; ++i)
		{
			bool value;

			if (!Savegame::ReadPhobosStream(stm, value, false))
				return false;

			this->emplace_back(value);
		}

		return true;
	}

	return false;
}

template <typename T>
bool ValueableVector<T>::Save(PhobosStreamWriter& Stm) const
{
	auto size = this->size();
	if (Savegame::WritePhobosStream(Stm, size))
	{
		for (auto const& item : *this)
		{
			if (!Savegame::WritePhobosStream(Stm, item))
				return false;
		}

		return true;
	}

	return false;
}

template <>
inline bool ValueableVector<bool>::Save(PhobosStreamWriter& stm) const
{
	auto size = this->size();
	if (Savegame::WritePhobosStream(stm, size))
	{
		for (bool item : *this)
		{
			if (!Savegame::WritePhobosStream(stm, item))
				return false;
		}

		return true;
	}

	return false;
}

// NullableVector

template <typename T>
void __declspec(noinline) NullableVector<T>::Read(INI_EX& parser, const char* pSection, const char* pKey)
{
	if (parser.ReadString(pSection, pKey))
	{
		this->clear();
		auto const non_default = _strcmpi(parser.value(), "<default>");
		this->hasValue = non_default;

		if (non_default)
			detail::parse_values<T>(*this, parser, pSection, pKey);
	}
}

template <typename T>
bool NullableVector<T>::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->clear();

	if (Savegame::ReadPhobosStream(Stm, this->hasValue, RegisterForChange))
		return !this->hasValue || ValueableVector<T>::Load(Stm, RegisterForChange);

	return false;
}

template <typename T>
bool NullableVector<T>::Save(PhobosStreamWriter& Stm) const
{
	if (Savegame::WritePhobosStream(Stm, this->hasValue))
		return !this->hasValue || ValueableVector<T>::Save(Stm);

	return false;
}


// ValueableIdxVector

template <typename Lookuper>
void __declspec(noinline) ValueableIdxVector<Lookuper>::Read(INI_EX& parser, const char* pSection, const char* pKey)
{
	if (parser.ReadString(pSection, pKey))
	{
		this->clear();
		detail::parse_indexes<Lookuper>(*this, parser, pSection, pKey);
	}
}


// NullableIdxVector

template <typename Lookuper>
void __declspec(noinline) NullableIdxVector<Lookuper>::Read(INI_EX& parser, const char* pSection, const char* pKey)
{
	if (parser.ReadString(pSection, pKey))
	{
		this->clear();
		auto const non_default = _strcmpi(parser.value(), "<default>") != 0;
		this->hasValue = non_default;

		if (non_default)
		{
			detail::parse_indexes<Lookuper>(*this, parser, pSection, pKey);
		}
	}
}

// Damageable

template <typename T>
void __declspec(noinline) Damageable<T>::Read(INI_EX& parser, const char* const pSection, const char* const pBaseFlag, const char* const pSingleFlag)
{
	// read the common flag, with the trailing dot being stripped
	char flagName[0x40];
	auto const pSingleFormat = pSingleFlag ? pSingleFlag : pBaseFlag;
	auto res = _snprintf_s(flagName, _TRUNCATE, pSingleFormat, "");

	if (res > 0 && flagName[res - 1] == '.')
		flagName[res - 1] = '\0';

	this->BaseValue.Read(parser, pSection, flagName);

	_snprintf_s(flagName, _TRUNCATE, pBaseFlag, "ConditionYellow");
	this->ConditionYellow.Read(parser, pSection, flagName);

	_snprintf_s(flagName, _TRUNCATE, pBaseFlag, "ConditionRed");
	this->ConditionRed.Read(parser, pSection, flagName);
};

template <typename T>
bool Damageable<T>::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Savegame::ReadPhobosStream(Stm, this->BaseValue, RegisterForChange)
		&& Savegame::ReadPhobosStream(Stm, this->ConditionYellow, RegisterForChange)
		&& Savegame::ReadPhobosStream(Stm, this->ConditionRed, RegisterForChange);
}

template <typename T>
bool Damageable<T>::Save(PhobosStreamWriter& Stm) const
{
	return Savegame::WritePhobosStream(Stm, this->BaseValue)
		&& Savegame::WritePhobosStream(Stm, this->ConditionYellow)
		&& Savegame::WritePhobosStream(Stm, this->ConditionRed);
}
