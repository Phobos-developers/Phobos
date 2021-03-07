#pragma once

#include <CCINIClass.h>
#include <TechnoTypeClass.h>

#include "../_Container.hpp"
#include "../../Phobos.h"
#include "../../Utilities/GeneralUtils.h"

#include "../../Utilities/Debug.h"
#include "../../Utilities/TemplateDef.h"

class TechnoTypeExt
{
public:
	using base_type = TechnoTypeClass;

	class ExtData final : public Extension<TechnoTypeClass>
	{
	public:
		
		Valueable<bool> Deployed_RememberTarget;
		Valueable<bool> HealthBar_Hide;
		Valueable<CSFText> UIDescription;
		Valueable<bool> LowSelectionPriority;
		PhobosFixedString<0x20> GroupAs;
		Valueable<double> MindControlRangeLimit;
		Valueable<bool> Interceptor;
		Valueable<double> Interceptor_GuardRange;
		Valueable<double> Interceptor_EliteGuardRange;

		ExtData(TechnoTypeClass* OwnerObject) : Extension<TechnoTypeClass>(OwnerObject),
			Deployed_RememberTarget(false),
			HealthBar_Hide(false),
			UIDescription(),
			LowSelectionPriority(false),
			GroupAs(NONE_STR),
			MindControlRangeLimit(-1.0),
			Interceptor(false),
			Interceptor_GuardRange(0.0),
			Interceptor_EliteGuardRange(0.0)
		{ }

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override {}

		virtual void LoadFromStream(IStream* Stm);

		virtual void SaveToStream(IStream* Stm) const;

		const char* GetSelectionGroupID() const;
	};

	class ExtContainer final : public Container<TechnoTypeExt> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	static const char* GetSelectionGroupID(ObjectTypeClass* pType);
	static bool HasSelectionGroupID(ObjectTypeClass* pType, const char* pID);
	static void TransferMindControl(TechnoClass* From, TechnoClass* To);

	static ExtContainer ExtMap;
};