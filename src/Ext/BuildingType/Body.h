#pragma once

#include <CCINIClass.h>
#include <BuildingTypeClass.h>

#include "../_Container.hpp"
#include "../../Phobos.h"

#include "../../Utilities/Debug.h"

#include "../../Utilities/TemplateDef.h"

class BuildingTypeExt
{
public:
	using base_type = BuildingTypeClass;

	class ExtData final : public Extension<BuildingTypeClass>
	{
	public:
		Valueable<AffectsHouses> PowersUp_Owner;
		ValueableIdxVector<BuildingTypeClass> PowersUp_Buildings;

		ExtData(BuildingTypeClass* OwnerObject) : Extension<BuildingTypeClass>(OwnerObject),
			PowersUp_Owner(AffectsHouses::Owner),
			PowersUp_Buildings()
		{ }

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override {}

		virtual void LoadFromStream(IStream* Stm);

		virtual void SaveToStream(IStream* Stm) const;
	};

	static bool CanUpgrade(BuildingClass* pBuilding, BuildingTypeClass* pUpgradeType, HouseClass* pUpgradeOwner);

	class ExtContainer final : public Container<BuildingTypeExt> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};