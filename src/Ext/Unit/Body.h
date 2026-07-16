#pragma once

#include <Ext/Foot/Body.h>
#include <UnitClass.h>

// Concrete leaf extension for UnitClass. Empty for now: all techno-level data lives
// in TechnoExt; this leaf only exists so a unit's extension has its own
// concrete type (TechnoExt itself is never instantiated).
class UnitExt final : public FootExt
{
public:
	using base_type = UnitClass;
	using ExtData = UnitExt;

	static constexpr DWORD Canary = 0xE1E2E3E4;

	explicit UnitExt(UnitClass* const OwnerObject) : FootExt(OwnerObject)
	{ }

	UnitClass* OwnerObject() const
	{
		return static_cast<UnitClass*>(this->GetAttachedObject());
	}

	virtual ~UnitExt() override
	{
		ExtMap.Unregister(this);
	}

	class ExtContainer final : public Container<UnitExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static UnitExt* Fetch(const UnitClass* pThis)
	{
		return ExtMap.Find(pThis);
	}

	static UnitExt* TryFetch(const UnitClass* pThis)
	{
		return ExtMap.TryFind(pThis);
	}
};
