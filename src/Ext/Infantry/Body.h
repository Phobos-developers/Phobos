#pragma once

#include <Ext/Foot/Body.h>
#include <InfantryClass.h>

// Concrete leaf extension for InfantryClass (empty; techno data lives in TechnoExt).
class InfantryExt : public FootExt
{
public:
	explicit InfantryExt(InfantryClass* const OwnerObject) : FootExt(OwnerObject)
	{ }

	InfantryClass* OwnerObject() const
	{
		return static_cast<InfantryClass*>(this->GetAttachedObject());
	}
};
