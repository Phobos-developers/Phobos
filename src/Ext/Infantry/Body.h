#pragma once

#include <Ext/Foot/Body.h>
#include <InfantryClass.h>

// Concrete leaf extension for InfantryClass (empty; techno data lives in TechnoClassExtension).
class InfantryClassExtension : public FootClassExtension
{
public:
	explicit InfantryClassExtension(InfantryClass* const OwnerObject) : FootClassExtension(OwnerObject)
	{ }

	InfantryClass* OwnerObject() const
	{
		return static_cast<InfantryClass*>(this->GetAttachedObject());
	}
};
