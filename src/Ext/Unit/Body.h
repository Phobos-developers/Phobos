#pragma once

#include <Ext/Foot/Body.h>
#include <Ext/UnitType/Body.h>
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
	bool IsBurrowed;
	bool UndergroundTracked;

	std::vector<RecoilData> ExtraTurretRecoil;
	std::vector<RecoilData> ExtraBarrelRecoil;

	explicit UnitExt(UnitClass* const OwnerObject) : FootExt(OwnerObject)
		, SubterraneanHarvStatus { 0 }
		, SubterraneanHarvRallyPoint { nullptr }
		, ReceiveDamage { false }
		, DeployFireTimer {}
		, KeepTargetOnMove { false }
		, SimpleDeployerAnimationTimer {}
		, IsBurrowed { false }
		, UndergroundTracked { false }
		, ExtraTurretRecoil {}
		, ExtraBarrelRecoil {}
	{ }

	virtual ~UnitExt() override;

	virtual bool IsBurrowedState() const override { return this->IsBurrowed; }

	void UpdateSubterraneanHarvester();
	void UpdateKeepTargetOnMove();
	void DepletedAmmoActions();
	void InitializeRecoilData();
	void UpdateRecoilData();
	void RecordRecoilData();

	static UnitClass* Deployer;

	static bool CannotMove(UnitClass* pThis);
	static bool HasAmmoToDeploy(UnitClass* pThis);
	static void HandleOnDeployAmmoChange(UnitClass* pThis, int maxAmmoOverride = -1);
	static bool SimpleDeployerAllowedToDeploy(UnitClass* pThis, bool defaultValue, bool alwaysCheckLandTypes);
	static bool CanDeployIntoBuilding(UnitClass* pThis, bool noDeploysIntoDefaultValue = false);
	static UnitTypeClass* GetUnitTypeExtra(UnitClass* pUnit, UnitTypeExt* pData);

	UnitClass* OwnerObject() const
	{
		return static_cast<UnitClass*>(this->GetAttachedObject());
	}

	// a unit's type extension is always the UnitTypeExt leaf
	UnitTypeExt* GetTypeExtData() const
	{
		return static_cast<UnitTypeExt*>(this->TypeExtData);
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
