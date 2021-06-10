#pragma once

#include <ScenarioClass.h>

#include <Helpers/Macro.h>
#include <Ext/_Container.hpp>
#include <Utilities/TemplateDef.h>

#include <map>

class ScenarioExt
{
public:
	using base_type = ScenarioClass;

	class ExtData final : public Extension<ScenarioClass>
	{
	public:
		std::map<int, CellStruct> Waypoints;

		ExtData(ScenarioClass* OwnerObject) : Extension<ScenarioClass>(OwnerObject),
			Waypoints {}
		{

		}

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