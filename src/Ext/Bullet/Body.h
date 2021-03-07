#pragma once

#include <Helpers/Enumerators.h>
#include <BulletClass.h>

#include "../_Container.hpp"
#include "../../Phobos.h"
#include "../../Utilities/TemplateDef.h"

class BulletExt
{
public:
	using base_type = BulletClass;

	class ExtData final : public Extension<BulletClass>
	{
	public:
		Valueable<bool> Intercepted;
		Valueable<bool> ShouldIntercept;

		ExtData(BulletClass* OwnerObject) : Extension<BulletClass>(OwnerObject),
			Intercepted(false),
			ShouldIntercept(false)
		{ }

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override {}

		virtual void LoadFromStream(IStream* Stm);
		virtual void SaveToStream(IStream* Stm);
	};

	class ExtContainer final : public Container<BulletExt> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};
