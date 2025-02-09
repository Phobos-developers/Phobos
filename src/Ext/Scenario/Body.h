#pragma once

#include <ScenarioClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <Ext/Techno/Body.h>

#include <map>

struct ExtendedVariable
{
	char Name[0x100];
	int Value;
};

class ScenarioExt
{
public:
	using base_type = ScenarioClass;

	static constexpr DWORD Canary = 0xABCD1595;

	class ExtData final : public Extension<ScenarioClass>
	{
	public:

		bool ShowBriefing;
		int BriefingTheme;

		std::map<int, CellStruct> Waypoints;
		std::map<int, ExtendedVariable> Variables[2]; // 0 for local, 1 for global

		std::vector<TechnoExt::ExtData*> AutoDeathObjects;
		std::vector<TechnoExt::ExtData*> TransportReloaders; // Objects that can reload ammo in limbo

		ExtData(ScenarioClass* OwnerObject) : Extension<ScenarioClass>(OwnerObject)
			, ShowBriefing { false }
			, BriefingTheme { -1 }
			, Waypoints { }
			, Variables { }
			, AutoDeathObjects {}
			, TransportReloaders {}
		{ }

		void SetVariableToByID(bool bIsGlobal, int nIndex, char bState);
		void GetVariableStateByID(bool bIsGlobal, int nIndex, char* pOut);
		void ReadVariables(bool bIsGlobal, CCINIClass* pINI);
		static void SaveVariablesToFile(bool isGlobal);

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

		void UpdateAutoDeathObjectsInLimbo();
		void UpdateTransportReloaders();
	private:
		template <typename T>
		void Serialize(T& Stm);
	};

private:
	static std::unique_ptr<ExtData> Data;

public:
	static IStream* g_pStm;

	static bool CellParsed;

	static void Allocate(ScenarioClass* pThis);
	static void Remove(ScenarioClass* pThis);

	static void LoadFromINIFile(ScenarioClass* pThis, CCINIClass* pINI);

	static ExtData* Global()
	{
		return Data.get();
	}

	static void Clear()
	{
		Allocate(ScenarioClass::Instance);
	}

	static void PointerGotInvalid(void* ptr, bool removed)
	{
		Global()->InvalidatePointer(ptr, removed);
	}

};
