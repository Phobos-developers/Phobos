#pragma once

#include <Ext/Foot/Body.h>
#include <Ext/InfantryType/Body.h>
#include <InfantryClass.h>

// Concrete leaf extension for InfantryClass.
class InfantryExt final : public FootExt
{
public:
	using base_type = InfantryClass;

	static constexpr DWORD Canary = 0xF1F2F3F4;

	bool SkipTargetChangeResetSequence;
	bool HasDeployConverted;
	bool HasUndeployConverted;

	explicit InfantryExt(InfantryClass* const OwnerObject) : FootExt(OwnerObject)
		, SkipTargetChangeResetSequence { false }
		, HasDeployConverted { false }
		, HasUndeployConverted { false }
	{ }

	InfantryClass* OwnerObject() const
	{
		return static_cast<InfantryClass*>(this->GetAttachedObject());
	}

	// an infantry's type extension is always the InfantryTypeExt leaf
	InfantryTypeExt* GetTypeExtData() const
	{
		return static_cast<InfantryTypeExt*>(this->TypeExtData);
	}

	static CoordStruct GetSimpleFLH(InfantryClass* pThis, int weaponIndex, bool& FLHFound);

	class ExtContainer final : public Container<InfantryExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static InfantryExt* Fetch(const InfantryClass* pThis)
	{
		return AbstractExt::Fetch<InfantryExt>(pThis);
	}

	static InfantryExt* TryFetch(const InfantryClass* pThis)
	{
		return AbstractExt::TryFetch<InfantryExt>(pThis);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);
};
