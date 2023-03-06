#pragma once

#include <ScenarioClass.h>
#include <HouseClass.h>
#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

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

	class ExtData final : public Extension<ScenarioClass>
	{
	public:
		std::map<int, CellStruct> Waypoints;
		std::map<int, ExtendedVariable> Variables[2]; // 0 for local, 1 for global

		LightingStruct DefaultNormalLighting;
		int DefaultAmbientOriginal;
		int DefaultAmbientCurrent;
		int DefaultAmbientTarget;
		TintStruct CurrentTint_Tiles;
		TintStruct CurrentTint_Schemes;
		TintStruct CurrentTint_Hashes;
		bool AdjustLightingFix;

		ExtData(ScenarioClass* OwnerObject) : Extension<ScenarioClass>(OwnerObject)
			, Waypoints { }
			, Variables { }
			, DefaultNormalLighting { {1000,1000,1000},0,0 }
			, DefaultAmbientOriginal { 0 }
			, DefaultAmbientCurrent { 0 }
			, DefaultAmbientTarget { 0 }
			, CurrentTint_Tiles { -1,-1,-1 }
			, CurrentTint_Schemes { -1,-1,-1 }
			, CurrentTint_Hashes { -1,-1,-1 }
			, AdjustLightingFix { false }
		{ }

		void SetVariableToByID(bool bIsGlobal, int nIndex, char bState);
		void GetVariableStateByID(bool bIsGlobal, int nIndex, char* pOut);
		void ReadVariables(bool bIsGlobal, CCINIClass* pINI);

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
	private:
		template <typename T>
		void Serialize(T& Stm);
	};

private:
	static std::unique_ptr<ExtData> Data;

public:
	static IStream* g_pStm;

	static bool CellParsed;

	static void RecreateLightSources();

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

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};
