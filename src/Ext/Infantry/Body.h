#pragma once

#include <Ext/Foot/Body.h>
#include <InfantryClass.h>

// Concrete leaf extension for InfantryClass (empty; techno data lives in TechnoExt).
class InfantryExt final : public FootExt
{
public:
	using base_type = InfantryClass;
	using ExtData = InfantryExt;

	static constexpr DWORD Canary = 0xF1F2F3F4;

	explicit InfantryExt(InfantryClass* const OwnerObject) : FootExt(OwnerObject)
	{ }

	InfantryClass* OwnerObject() const
	{
		return static_cast<InfantryClass*>(this->GetAttachedObject());
	}

	virtual ~InfantryExt() override
	{
		ExtMap.Unregister(this);
	}

	class ExtContainer final : public Container<InfantryExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static InfantryExt* Fetch(const InfantryClass* pThis)
	{
		return ExtMap.Find(pThis);
	}

	static InfantryExt* TryFetch(const InfantryClass* pThis)
	{
		return ExtMap.TryFind(pThis);
	}
};
