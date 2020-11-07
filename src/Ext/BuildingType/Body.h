#pragma once

#include <CCINIClass.h>
#include <BuildingTypeClass.h>

#include "../_Container.hpp"
#include "../../Phobos.h"

#include "../../Utilities/Debug.h"

#include "../../Utilities/CanTargetFlags.h"

class BuildingTypeExt
{
public:
	using base_type = BuildingTypeClass;

	class ExtData final : public Extension<BuildingTypeClass>
	{
	public:

		CanTargetFlags canTargetFlags;
		//bool SpySat;
		//bool BigGap;
		//int TransactMoney;

		ExtData(BuildingTypeClass* OwnerObject) : Extension<BuildingTypeClass>(OwnerObject),
			canTargetFlags(CanTargetFlags::Own)
			//SpySat(false),
			//BigGap(false),
			//TransactMoney(0)
		{ }

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override {}

		virtual void LoadFromStream(IStream* Stm);

		virtual void SaveToStream(IStream* Stm);
	};

	class ExtContainer final : public Container<BuildingTypeExt> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};