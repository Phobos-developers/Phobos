#pragma once

#include <CCINIClass.h>
#include <SideClass.h>

#include "../../Phobos.h"
#include "../_Container.hpp"

class SideExt
{
public:
	using base_type = SideClass;

	class ExtData final : public Extension<SideClass>
	{
	public:
		int ArrayIndex;
		bool Sidebar_GDIPositions;

		ExtData(SideClass* OwnerObject) : Extension<SideClass>(OwnerObject),
			ArrayIndex(-1)//,
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void Initialize() override;
		virtual void InvalidatePointer(void *ptr, bool bRemoved) override {
		}

		virtual void LoadFromStream(IStream* Stm);
		virtual void SaveToStream(IStream* Stm);
	};

	class ExtContainer final : public Container<SideExt> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};
