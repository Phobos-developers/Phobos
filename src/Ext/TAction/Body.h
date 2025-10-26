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

#include <Utilities/Container.h>
#include <Utilities/Template.h>

#include <Helpers/Template.h>

#include <TActionClass.h>

class HouseClass;

enum class PhobosTriggerAction : unsigned int
{
	SaveGame = 500,
	EditVariable = 501,
	GenerateRandomNumber = 502,
	PrintVariableValue = 503,
	BinaryOperation = 504,
	RunSuperWeaponAtLocation = 505,
	RunSuperWeaponAtWaypoint = 506,
	ToggleMCVRedeploy = 510,

	EditAngerNode = 606,
	ClearAngerNode = 607,
	SetForceEnemy = 608,

	CreateBannerLocal = 800, // any banner w/ local variable
	CreateBannerGlobal = 801, // any banner w/ global variable
	DeleteBanner = 802,
};

class TActionExt
{
public:
	using base_type = TActionClass;

	static constexpr DWORD Canary = 0x91919191;

	class ExtData final : public Extension<TActionClass>
	{
	public:
		ExtData(TActionClass* const OwnerObject) : Extension<TActionClass>(OwnerObject)
		{ }

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	static bool Execute(TActionClass* pThis, HouseClass* pHouse,
			ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location, bool& bHandled);

#pragma push_macro("ACTION_FUNC")
#define ACTION_FUNC(name) \
	static bool name(TActionClass* pThis, HouseClass* pHouse, \
		ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)

	ACTION_FUNC(PlayAudioAtRandomWP);
	ACTION_FUNC(SaveGame);
	ACTION_FUNC(EditVariable);
	ACTION_FUNC(GenerateRandomNumber);
	ACTION_FUNC(PrintVariableValue);
	ACTION_FUNC(BinaryOperation);
	ACTION_FUNC(RunSuperWeaponAtLocation);
	ACTION_FUNC(RunSuperWeaponAtWaypoint);
	ACTION_FUNC(ToggleMCVRedeploy);

	ACTION_FUNC(EditAngerNode);
	ACTION_FUNC(ClearAngerNode);
	ACTION_FUNC(SetForceEnemy);

	ACTION_FUNC(CreateBannerLocal);
	ACTION_FUNC(CreateBannerGlobal);
	ACTION_FUNC(DeleteBanner);

	static bool RunSuperWeaponAt(TActionClass* pThis, int X, int Y);

#undef ACTION_FUNC
#pragma pop_macro("ACTION_FUNC")

	class ExtContainer final : public Container<TActionExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};
