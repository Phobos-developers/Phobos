#pragma once

#include <Ext/TechnoType/Body.h>
#include <InfantryTypeClass.h>

// Concrete leaf extension for InfantryTypeClass (empty).
class InfantryTypeExt : public TechnoTypeExt
{
public:
	explicit InfantryTypeExt(InfantryTypeClass* const OwnerObject) : TechnoTypeExt(OwnerObject)
	{ }

	InfantryTypeClass* OwnerObject() const
	{
		return static_cast<InfantryTypeClass*>(this->GetAttachedObject());
	}
};
