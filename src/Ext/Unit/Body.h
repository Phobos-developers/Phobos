#pragma once

#include <Ext/Foot/Body.h>
#include <UnitClass.h>

// Concrete leaf extension for UnitClass. Empty for now: all techno-level data lives
// in TechnoClassExtension; this leaf only exists so a unit's extension has its own
// concrete type (TechnoClassExtension itself is never instantiated).
class UnitClassExtension : public FootClassExtension
{
public:
	explicit UnitClassExtension(UnitClass* const OwnerObject) : FootClassExtension(OwnerObject)
	{ }

	UnitClass* OwnerObject() const
	{
		return static_cast<UnitClass*>(this->GetAttachedObject());
	}
};
