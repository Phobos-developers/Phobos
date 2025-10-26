// Phobos - Ares-compatible C&C Red Alert 2: Yuri's Revenge engine extension
// Copyright (C) 2020 Phobos developers
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.


// Phobos - Ares-compatible C&C Red Alert 2: Yuri's Revenge engine extension
// Copyright (C) 2020 Phobos developers
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.


// Phobos - Ares-compatible C&C Red Alert 2: Yuri's Revenge engine extension
// Copyright (C) 2020 Phobos developers
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.


// Phobos - Ares-compatible C&C Red Alert 2: Yuri's Revenge engine extension
// Copyright (C) 2020 Phobos developers
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.


#pragma once
#include <TeamClass.h>

#include <Helpers/Enumerators.h>
#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <Phobos.h>

class TeamExt
{
public:
	using base_type = TeamClass;

	static constexpr DWORD Canary = 0x414B4B41;
	static constexpr size_t ExtPointerOffset = 0x18;
	static constexpr bool ShouldConsiderInvalidatePointer = true;

	class ExtData final : public Extension<TeamClass>
	{
	public:
		int WaitNoTargetAttempts;
		double NextSuccessWeightAward;
		int IdxSelectedObjectFromAIList;
		double CloseEnough;
		int Countdown_RegroupAtLeader;
		int MoveMissionEndMode;
		int WaitNoTargetCounter;
		CDTimerClass WaitNoTargetTimer;
		CDTimerClass ForceJump_Countdown;
		int ForceJump_InitialCountdown;
		bool ForceJump_RepeatMode;
		FootClass* TeamLeader;
		std::vector<ScriptClass*> PreviousScriptList;

		ExtData(TeamClass* OwnerObject) : Extension<TeamClass>(OwnerObject)
			, WaitNoTargetAttempts { 0 }
			, NextSuccessWeightAward { 0 }
			, IdxSelectedObjectFromAIList { -1 }
			, CloseEnough { -1 }
			, Countdown_RegroupAtLeader { -1 }
			, MoveMissionEndMode { 0 }
			, WaitNoTargetCounter { 0 }
			, WaitNoTargetTimer { }
			, ForceJump_Countdown { }
			, ForceJump_InitialCountdown { -1 }
			, ForceJump_RepeatMode { false }
			, TeamLeader { nullptr }
			, PreviousScriptList { }
		{ }

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<TeamExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		virtual bool InvalidateExtDataIgnorable(void* const ptr) const override
		{
			auto const abs = static_cast<AbstractClass*>(ptr)->WhatAmI();

			switch (abs)
			{
			case AbstractType::Infantry:
			case AbstractType::Unit:
			case AbstractType::Aircraft:
				return false;
			}

			return true;
		}
	};

	static ExtContainer ExtMap;

};
