#pragma once

#include <Ext/TechnoType/Body.h>
#include <InfantryTypeClass.h>

// Concrete leaf extension for InfantryTypeClass (empty).
class InfantryTypeExt final : public TechnoTypeExt
{
public:
	using base_type = InfantryTypeClass;

	static constexpr DWORD Canary = 0xF5F6F7F8;

	explicit InfantryTypeExt(InfantryTypeClass* const OwnerObject) : TechnoTypeExt(OwnerObject)
	{ }

	InfantryTypeClass* OwnerObject() const
	{
		return static_cast<InfantryTypeClass*>(this->GetAttachedObject());
	}

	class ExtContainer final : public Container<InfantryTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static InfantryTypeExt* Fetch(const InfantryTypeClass* pThis)
	{
		return AbstractExt::Fetch<InfantryTypeExt>(pThis);
	}

	static InfantryTypeExt* TryFetch(const InfantryTypeClass* pThis)
	{
		return AbstractExt::TryFetch<InfantryTypeExt>(pThis);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);
};
