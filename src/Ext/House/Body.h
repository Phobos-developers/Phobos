#pragma once
#include <HouseClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include "../../Utilities/TemplateDef.h"


class HouseExt 
{
public:/*
	using base_type = HouseClass;
	class ExtData final : public Extension<HouseClass>
	{
	public:
		

		ExtData(HouseClass* OwnerObject) : Extension<HouseClass>(OwnerObject)
		{ }

		virtual ~ExtData() = default;

		//virtual void Initialize() override;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override {}

		virtual void LoadFromStream(IStream * Stm);
		virtual void SaveToStream(IStream * Stm);
	};

	class ExtContainer final : public Container<HouseExt> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;*/
	static int ActiveHarvesterCount(HouseClass* pThis);
	static int TotalHarvesterCount(HouseClass* pThis);
};