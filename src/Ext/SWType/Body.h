#pragma once

#include <CCINIClass.h>
#include <SuperWeaponTypeClass.h>

#include "../_Container.hpp"
#include "../../Phobos.h"
#include "../../Utilities/GeneralUtils.h"

class SWTypeExt
{
public:
	using base_type = SuperWeaponTypeClass;

	class ExtData final : public Extension<SuperWeaponTypeClass>
	{
	public:

		int Money_Amount;
		char UIDescriptionLabel[32];
		const wchar_t* UIDescription;

		ExtData(SuperWeaponTypeClass* OwnerObject) : Extension<SuperWeaponTypeClass>(OwnerObject),
			Money_Amount(0),
			UIDescriptionLabel(NONE_STR),
			UIDescription(L"")
		{ }

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override {}

		virtual void LoadFromStream(IStream* Stm);

		virtual void SaveToStream(IStream* Stm);
	};

	class ExtContainer final : public Container<SWTypeExt> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};