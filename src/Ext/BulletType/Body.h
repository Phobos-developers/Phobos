#pragma once

#include <CCINIClass.h>
#include <BulletTypeClass.h>

#include "../_Container.hpp"
#include "../../Phobos.h"
#include "../../Utilities/TemplateDef.h"

class BulletTypeExt
{
public:
	using base_type = BulletTypeClass;

	class ExtData final : public Extension<BulletTypeClass>
	{
	public:
		Valueable<bool> Interceptable;

		ExtData(BulletTypeClass* OwnerObject) : Extension<BulletTypeClass>(OwnerObject),
			Interceptable(false)
		{ }

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override {}

		virtual void LoadFromStream(IStream* Stm);
		virtual void SaveToStream(IStream* Stm);
	};

	class ExtContainer final : public Container<BulletTypeExt> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};
