#pragma once

#include <Ext/TechnoType/Body.h>
#include <InfantryTypeClass.h>

// Concrete leaf extension for InfantryTypeClass (empty).
class InfantryTypeExt final : public TechnoTypeExt
{
public:
	using base_type = InfantryTypeClass;
	using ExtData = InfantryTypeExt;

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
		return ExtMap.Find(pThis);
	}

	static InfantryTypeExt* TryFetch(const InfantryTypeClass* pThis)
	{
		return ExtMap.TryFind(pThis);
	}
};
