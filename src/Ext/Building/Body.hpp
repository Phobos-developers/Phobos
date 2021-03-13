#pragma once
#include <BuildingClass.h>

#include <Helpers/Macro.h>
#include "../_Container.hpp"
#include "../../Utilities/TemplateDef.h"

class BuildingExt
{
public:
	using base_type = BuildingClass;

	class ExtData final : public Extension<BuildingClass>
	{
	public:

		ExtData(BuildingClass* OwnerObject) : Extension<BuildingClass>(OwnerObject)
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