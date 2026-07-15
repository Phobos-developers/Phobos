#pragma once

#include <Ext/TechnoType/Body.h>
#include <UnitTypeClass.h>

// Concrete leaf extension for UnitTypeClass (empty; techno-type data lives in TechnoTypeClassExtension).
class UnitTypeClassExtension : public TechnoTypeClassExtension
{
public:
	explicit UnitTypeClassExtension(UnitTypeClass* const OwnerObject) : TechnoTypeClassExtension(OwnerObject)
	{ }

	UnitTypeClass* OwnerObject() const
	{
		return static_cast<UnitTypeClass*>(this->GetAttachedObject());
	}
};
