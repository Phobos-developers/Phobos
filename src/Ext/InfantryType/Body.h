#pragma once

#include <Ext/TechnoType/Body.h>
#include <InfantryTypeClass.h>

// Concrete leaf extension for InfantryTypeClass (empty).
class InfantryTypeClassExtension : public TechnoTypeClassExtension
{
public:
	explicit InfantryTypeClassExtension(InfantryTypeClass* const OwnerObject) : TechnoTypeClassExtension(OwnerObject)
	{ }

	InfantryTypeClass* OwnerObject() const
	{
		return static_cast<InfantryTypeClass*>(this->GetAttachedObject());
	}
};
