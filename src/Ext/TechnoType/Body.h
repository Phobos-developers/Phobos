#pragma once

#include <CCINIClass.h>
#include <TechnoTypeClass.h>

#include "../_Container.hpp"
#include "../../Phobos.h"
#include "../../Utilities/GeneralUtils.h"

#include "../../Utilities/Debug.h"

class TechnoTypeExt
{
public:
	using base_type = TechnoTypeClass;

	class ExtData final : public Extension<TechnoTypeClass>
	{
	public:
		bool Deployed_RememberTarget;
		bool HealthBar_Hide;
		char UIDescriptionLabel[32];
		const wchar_t* UIDescription;
		bool LowSelectionPriority;
		char GroupAs[32];

		ExtData(TechnoTypeClass* OwnerObject) : Extension<TechnoTypeClass>(OwnerObject),
			Deployed_RememberTarget(false),
			HealthBar_Hide(false),
			UIDescriptionLabel(NONE_STR),
			UIDescription(L""),
			LowSelectionPriority(false),
			GroupAs(NONE_STR)
		{ }

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override {}

		virtual void LoadFromStream(IStream* Stm);

		virtual void SaveToStream(IStream* Stm);

		const char* GetSelectionGroupID() const;
	};

	class ExtContainer final : public Container<TechnoTypeExt> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	static const char* GetSelectionGroupID(ObjectTypeClass* pType);
	static bool HasSelectionGroupID(ObjectTypeClass* pType, const char* pID);

	static ExtContainer ExtMap;
};