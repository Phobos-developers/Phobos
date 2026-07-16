#pragma once

#include <Ext/Foot/Body.h>
#include <UnitClass.h>

// Concrete leaf extension for UnitClass. Empty for now: all techno-level data lives
// in TechnoExt; this leaf only exists so a unit's extension has its own
// concrete type (TechnoExt itself is never instantiated).
class UnitExt : public FootExt
{
public:
	explicit UnitExt(UnitClass* const OwnerObject) : FootExt(OwnerObject)
	{ }

	UnitClass* OwnerObject() const
	{
		return static_cast<UnitClass*>(this->GetAttachedObject());
	}
};
