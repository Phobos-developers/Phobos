#pragma once

#include <Ext/TechnoType/Body.h>
#include <UnitTypeClass.h>

// Concrete leaf extension for UnitTypeClass (empty; techno-type data lives in TechnoTypeExt).
class UnitTypeExt : public TechnoTypeExt
{
public:
	explicit UnitTypeExt(UnitTypeClass* const OwnerObject) : TechnoTypeExt(OwnerObject)
	{ }

	UnitTypeClass* OwnerObject() const
	{
		return static_cast<UnitTypeClass*>(this->GetAttachedObject());
	}
};
