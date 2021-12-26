#pragma once
#include <BuildingClass.h>
#include <HouseClass.h>
#include <FactoryClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/Building/Body.h>

class BuildingExt
{
public:
	using base_type = BuildingClass;

	class ExtData final : public Extension<BuildingClass>
	{
	public:
		Valueable<bool> DeployedTechno;
		Valueable<int> LimboID;
		Valueable<BuildingClass*> CurrentAirFactory;

		ExtData(BuildingClass* OwnerObject) : Extension<BuildingClass>(OwnerObject)
			, DeployedTechno { false }
			, LimboID { -1 }
			, CurrentAirFactory(nullptr)

		{ }

		virtual ~ExtData() = default;

		// virtual void LoadFromINIFile(CCINIClass* pINI) override;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<BuildingExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static void UpdatePrimaryFactoryAI(BuildingClass* pThis);
	static int CountOccupiedDocks(BuildingClass* pBuilding);
	static bool HasFreeDocks(BuildingClass* pBuilding);
};