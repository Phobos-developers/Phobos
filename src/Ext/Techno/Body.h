#pragma once

#include <Helpers/Enumerators.h>
#include <TechnoClass.h>
#include <BulletClass.h>

#include "../_Container.hpp"
#include "../../Phobos.h"
#include "../../Utilities/TemplateDef.h"

class TechnoExt
{
public:
	using base_type = TechnoClass;

	class ExtData final : public Extension<TechnoClass>
	{
	public:
		Valueable<BulletClass*> InterceptedBullet;

		ExtData(TechnoClass* OwnerObject) : Extension<TechnoClass>(OwnerObject),
			InterceptedBullet(nullptr)
		{ }

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override {}

		virtual void LoadFromStream(IStream* Stm);
		virtual void SaveToStream(IStream* Stm);
	};

	class ExtContainer final : public Container<TechnoExt> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};
