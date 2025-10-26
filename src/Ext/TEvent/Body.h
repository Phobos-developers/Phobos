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

#include <optional>

#include <TEventClass.h>

class HouseClass;

enum PhobosTriggerEvent
{
	LocalVariableGreaterThan = 500,
	LocalVariableLessThan = 501,
	LocalVariableEqualsTo = 502,
	LocalVariableGreaterThanOrEqualsTo = 503,
	LocalVariableLessThanOrEqualsTo = 504,
	LocalVariableAndIsTrue = 505,
	GlobalVariableGreaterThan = 506,
	GlobalVariableLessThan = 507,
	GlobalVariableEqualsTo = 508,
	GlobalVariableGreaterThanOrEqualsTo = 509,
	GlobalVariableLessThanOrEqualsTo = 510,
	GlobalVariableAndIsTrue = 511,
	LocalVariableGreaterThanLocalVariable = 512,
	LocalVariableLessThanLocalVariable = 513,
	LocalVariableEqualsToLocalVariable = 514,
	LocalVariableGreaterThanOrEqualsToLocalVariable = 515,
	LocalVariableLessThanOrEqualsToLocalVariable = 516,
	LocalVariableAndIsTrueLocalVariable = 517,
	GlobalVariableGreaterThanLocalVariable = 518,
	GlobalVariableLessThanLocalVariable = 519,
	GlobalVariableEqualsToLocalVariable = 520,
	GlobalVariableGreaterThanOrEqualsToLocalVariable = 521,
	GlobalVariableLessThanOrEqualsToLocalVariable = 522,
	GlobalVariableAndIsTrueLocalVariable = 523,
	LocalVariableGreaterThanGlobalVariable = 524,
	LocalVariableLessThanGlobalVariable = 525,
	LocalVariableEqualsToGlobalVariable = 526,
	LocalVariableGreaterThanOrEqualsToGlobalVariable = 527,
	LocalVariableLessThanOrEqualsToGlobalVariable = 528,
	LocalVariableAndIsTrueGlobalVariable = 529,
	GlobalVariableGreaterThanGlobalVariable = 530,
	GlobalVariableLessThanGlobalVariable = 531,
	GlobalVariableEqualsToGlobalVariable = 532,
	GlobalVariableGreaterThanOrEqualsToGlobalVariable = 533,
	GlobalVariableLessThanOrEqualsToGlobalVariable = 534,
	GlobalVariableAndIsTrueGlobalVariable = 535,

	ShieldBroken = 600,
	HouseOwnsTechnoType = 601,
	HouseDoesntOwnTechnoType = 602,
	CellHasTechnoType = 604,
	CellHasAnyTechnoTypeFromList = 605,
	AttachedIsUnderAttachedEffect = 606,

	_DummyMaximum,
};

class TEventExt
{
public:
	using base_type = TEventClass;

	static constexpr DWORD Canary = 0x91919191;

	class ExtData final : public Extension<TEventClass>
	{
	public:
		ExtData(TEventClass* const OwnerObject) : Extension<TEventClass>(OwnerObject)
		{ }

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	static int GetFlags(int iEvent);

	static std::optional<bool> Execute(TEventClass* pThis, int iEvent, HouseClass* pHouse,
		ObjectClass* pObject, CDTimerClass* pTimer, bool* isPersitant, TechnoClass* pSource);

	template<bool IsGlobal, typename _Pr>
	static bool VariableCheck(TEventClass* pThis);
	template<bool IsSrcGlobal, bool IsGlobal, typename _Pr>
	static bool VariableCheckBinary(TEventClass* pThis);

	static bool HouseOwnsTechnoTypeTEvent(TEventClass* pThis);
	static bool HouseDoesntOwnTechnoTypeTEvent(TEventClass* pThis);

	static bool CellHasAnyTechnoTypeFromListTEvent(TEventClass* pThis, ObjectClass* pObject, HouseClass* pHouse);
	static bool CellHasTechnoTypeTEvent(TEventClass* pThis, ObjectClass* pObject, HouseClass* pHouse);

	static bool AttachedIsUnderAttachedEffectTEvent(TEventClass* pThis, ObjectClass* pObject);


	class ExtContainer final : public Container<TEventExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};
