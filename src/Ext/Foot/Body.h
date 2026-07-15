#pragma once

#include <Ext/Techno/Body.h>
#include <FootClass.h>

// Empty intermediate base mirroring FootClass in the extension hierarchy.
// UnitClassExtension / InfantryClassExtension / AircraftClassExtension derive from this.
class FootClassExtension : public TechnoClassExtension
{
public:
	explicit FootClassExtension(FootClass* const OwnerObject) : TechnoClassExtension(OwnerObject)
	{ }

	FootClass* OwnerObject() const
	{
		return static_cast<FootClass*>(this->GetAttachedObject());
	}
};
