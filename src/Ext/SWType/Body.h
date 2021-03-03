#pragma once

#include <CCINIClass.h>
#include <SuperWeaponTypeClass.h>

#include "../_Container.hpp"
#include "../../Phobos.h"
#include "../../Utilities/GeneralUtils.h"

#include "../../Utilities/TemplateDef.h"

class SWTypeExt
{
public:
	using base_type = SuperWeaponTypeClass;

	class ExtData final : public Extension<SuperWeaponTypeClass>
	{
	public:

		Valueable<int> Money_Amount;
		Valueable<CSFText> UIDescription;

		ExtData(SuperWeaponTypeClass* OwnerObject) : Extension<SuperWeaponTypeClass>(OwnerObject),
			Money_Amount(0),
			UIDescription()
		{ }

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override {}

		virtual void LoadFromStream(IStream* Stm);

		virtual void SaveToStream(IStream* Stm) const;
	};

	class ExtContainer final : public Container<SWTypeExt> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};