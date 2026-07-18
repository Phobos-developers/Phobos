#pragma once

#include <Ext/Techno/Body.h>
#include <FootClass.h>

// Empty intermediate base mirroring FootClass in the extension hierarchy.
// UnitExt / InfantryExt / AircraftExt derive from this.
class FootExt : public TechnoExt
{
public:
	explicit FootExt(FootClass* const OwnerObject) : TechnoExt(OwnerObject)
	{ }

	FootClass* OwnerObject() const
	{
		return static_cast<FootClass*>(this->GetAttachedObject());
	}

	static FootExt* Fetch(const FootClass* pThis)
	{
		return AbstractExt::Fetch<FootExt>(pThis);
	}

	static FootExt* TryFetch(const FootClass* pThis)
	{
		return AbstractExt::TryFetch<FootExt>(pThis);
	}
};
