#pragma once

#include <Ext/Techno/Body.h>
#include <FootClass.h>

// Intermediate base mirroring FootClass in the extension hierarchy; holds the
// data shared by units, infantry and aircraft (but not buildings).
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

	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);
};
