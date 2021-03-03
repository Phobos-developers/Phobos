#pragma once
#include "../BuildingType/Body.h"
#include <Helpers/Enumerators.h>
#include <BuildingClass.h>

#include "../_Container.hpp"
#include "../../Phobos.h"

#include "../../Utilities/Debug.h"

class BuildingExt
{
public:
	using base_type = BuildingClass;

	class ExtData final : public Extension<BuildingClass>
	{
	public:

		//bool SpySat;
		//bool BigGap;
		//int TransactMoney;

		ExtData(BuildingClass* OwnerObject) : Extension<BuildingClass>(OwnerObject)//,
			//SpySat(false),
			//BigGap(false),
			//TransactMoney(0)
		{ }


		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override {}

		virtual void LoadFromStream(IStream* Stm);

		virtual void SaveToStream(IStream* Stm) const;
	};

	class ExtContainer final : public Container<BuildingExt> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};