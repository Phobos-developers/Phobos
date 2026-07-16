#pragma once

#include <Ext/TechnoType/Body.h>
#include <AircraftTypeClass.h>

// Concrete leaf extension for AircraftTypeClass (empty).
class AircraftTypeExt final : public TechnoTypeExt
{
public:
	using base_type = AircraftTypeClass;
	using ExtData = AircraftTypeExt;

	static constexpr DWORD Canary = 0xA5A6A7A8;

	explicit AircraftTypeExt(AircraftTypeClass* const OwnerObject) : TechnoTypeExt(OwnerObject)
	{ }

	AircraftTypeClass* OwnerObject() const
	{
		return static_cast<AircraftTypeClass*>(this->GetAttachedObject());
	}

	class ExtContainer final : public Container<AircraftTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static AircraftTypeExt* Fetch(const AircraftTypeClass* pThis)
	{
		return ExtMap.Find(pThis);
	}

	static AircraftTypeExt* TryFetch(const AircraftTypeClass* pThis)
	{
		return ExtMap.TryFind(pThis);
	}
};
