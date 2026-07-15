#pragma once

#include <Ext/TechnoType/Body.h>
#include <AircraftTypeClass.h>

// Concrete leaf extension for AircraftTypeClass (empty).
class AircraftTypeClassExtension : public TechnoTypeClassExtension
{
public:
	explicit AircraftTypeClassExtension(AircraftTypeClass* const OwnerObject) : TechnoTypeClassExtension(OwnerObject)
	{ }

	AircraftTypeClass* OwnerObject() const
	{
		return static_cast<AircraftTypeClass*>(this->GetAttachedObject());
	}
};
