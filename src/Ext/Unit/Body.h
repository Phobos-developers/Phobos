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

	static constexpr DWORD Canary = 0xE1E2E3E4;

	int SubterraneanHarvStatus; // 0 = none, 1 = created, 2 = out from factory
	AbstractClass* SubterraneanHarvRallyPoint;
	bool ReceiveDamage;
	CDTimerClass DeployFireTimer;
	bool KeepTargetOnMove;
	CDTimerClass SimpleDeployerAnimationTimer;

	explicit UnitExt(UnitClass* const OwnerObject) : FootExt(OwnerObject)
		, SubterraneanHarvStatus { 0 }
		, SubterraneanHarvRallyPoint { nullptr }
		, ReceiveDamage { false }
		, DeployFireTimer {}
		, KeepTargetOnMove { false }
		, SimpleDeployerAnimationTimer {}
	{ }

	void UpdateSubterraneanHarvester();
	void UpdateKeepTargetOnMove();

	UnitClass* OwnerObject() const
	{
		return static_cast<UnitClass*>(this->GetAttachedObject());
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
		return AbstractExt::Fetch<UnitExt>(pThis);
	}

	static UnitExt* TryFetch(const UnitClass* pThis)
	{
		return AbstractExt::TryFetch<UnitExt>(pThis);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);
};
