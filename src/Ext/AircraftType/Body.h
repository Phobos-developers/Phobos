#pragma once

#include <Ext/TechnoType/Body.h>
#include <AircraftTypeClass.h>

// Concrete leaf extension for AircraftTypeClass (empty).
class AircraftTypeExt final : public TechnoTypeExt
{
public:
	using base_type = AircraftTypeClass;

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
		return AbstractExt::Fetch<AircraftTypeExt>(pThis);
	}

	static AircraftTypeExt* TryFetch(const AircraftTypeClass* pThis)
	{
		return AbstractExt::TryFetch<AircraftTypeExt>(pThis);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);
};
