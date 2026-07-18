#pragma once

#include <Ext/TechnoType/Body.h>
#include <UnitTypeClass.h>

// Concrete leaf extension for UnitTypeClass (empty; techno-type data lives in TechnoTypeExt).
class UnitTypeExt final : public TechnoTypeExt
{
public:
	using base_type = UnitTypeClass;

	static constexpr DWORD Canary = 0xE5E6E7E8;

	explicit UnitTypeExt(UnitTypeClass* const OwnerObject) : TechnoTypeExt(OwnerObject)
	{ }

	UnitTypeClass* OwnerObject() const
	{
		return static_cast<UnitTypeClass*>(this->GetAttachedObject());
	}

	class ExtContainer final : public Container<UnitTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static UnitTypeExt* Fetch(const UnitTypeClass* pThis)
	{
		return AbstractExt::Fetch<UnitTypeExt>(pThis);
	}

	static UnitTypeExt* TryFetch(const UnitTypeClass* pThis)
	{
		return AbstractExt::TryFetch<UnitTypeExt>(pThis);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);
};
