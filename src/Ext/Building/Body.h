#pragma once
#include <Ext/Techno/Body.h>
#include <Ext/BuildingType/Body.h>
#include <BuildingClass.h>

class BuildingExt final : public TechnoExt, public Detach::Listener<BuildingClass>
{
public:
	using base_type = BuildingClass;

	// deprecated: the pre-rework nested data class is now the extension class itself
	using ExtData [[deprecated("use the extension class itself instead")]] = BuildingExt;

	static constexpr DWORD Canary = 0x87654321;

	bool DeployedTechno;
	bool IsCreatedFromMapFile;
	bool HasPowerFromMapFile;
	int LimboID;
	int GrindingWeapon_LastFiredFrame;
	int GrindingWeapon_AccumulatedCredits;
	BuildingClass* CurrentAirFactory;
	int AccumulatedIncome;
	std::optional<int> CurrentLaserWeaponIndex;
	int PoweredUpToLevel; // Distinct from UpgradeLevel, and set to highest PowersUpToLevel out of applied upgrades regardless of how many are currently applied to this building.
	SuperClass* CurrentEMPulseSW;
	bool IsFiringNow;
	int TurretAnimIdleFrame;
	int TurretAnimFiringFrame;
	int TurretAnimRateTick;
	int ConstructionStartFacing;
	bool IsPlayingRoofProductionAnim;
	int MoneyGrade;

	BuildingExt(BuildingClass* OwnerObject) : TechnoExt(OwnerObject)
		, DeployedTechno { false }
		, IsCreatedFromMapFile { false }
		, HasPowerFromMapFile { false }
		, LimboID { -1 }
		, GrindingWeapon_LastFiredFrame { 0 }
		, GrindingWeapon_AccumulatedCredits { 0 }
		, CurrentAirFactory { nullptr }
		, AccumulatedIncome { 0 }
		, CurrentLaserWeaponIndex {}
		, PoweredUpToLevel { 0 }
		, CurrentEMPulseSW {}
		, IsFiringNow { false }
		, TurretAnimIdleFrame { 0 }
		, TurretAnimFiringFrame { -1 }
		, TurretAnimRateTick { 0 }
		, ConstructionStartFacing { -1 }
		, IsPlayingRoofProductionAnim { false }
		, MoneyGrade { -1 }
	{ }

	// typed owner accessor (shadows the TechnoClass one from the base)
	BuildingClass* OwnerObject() const
	{
		return static_cast<BuildingClass*>(this->TechnoExt::OwnerObject());
	}

	// a building's type extension is always the BuildingTypeExt leaf
	BuildingTypeExt* GetTypeExtData() const
	{
		return static_cast<BuildingTypeExt*>(this->TypeExtData);
	}

	void DisplayIncomeString();
	void ApplyPoweredKillSpawns();
	bool HasSuperWeapon(int index) const;
	bool HandleInfiltrate(HouseClass* pInfiltratorHouse, int moneybefore);
	void UpdatePrimaryFactoryAI();

	virtual ~BuildingExt() = default;

	// virtual void LoadFromINIFile(CCINIClass* pINI) override;

	virtual void OnDetach(BuildingClass* pTarget, bool removed) override
	{
		if (removed)
			AnnounceInvalidPointer(this->CurrentAirFactory, pTarget);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);

public:
	class ExtContainer final : public Container<BuildingExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static BuildingExt* Fetch(const BuildingClass* pThis)
	{
		return AbstractExt::Fetch<BuildingExt>(pThis);
	}

	static BuildingExt* TryFetch(const BuildingClass* pThis)
	{
		return AbstractExt::TryFetch<BuildingExt>(pThis);
	}

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static void StoreTiberium(BuildingClass* pThis, float amount, int idxTiberiumType, int idxStorageTiberiumType);

	static int CountOccupiedDocks(BuildingClass* pBuilding);
	static bool HasFreeDocks(BuildingClass* pBuilding);
	static bool CanGrindTechno(BuildingClass* pBuilding, TechnoClass* pTechno);
	static bool DoGrindingExtras(BuildingClass* pBuilding, TechnoClass* pTechno, int refund);
	static bool CanUndeployOnSell(BuildingClass* pThis);
	static void KickOutStuckUnits(BuildingClass* pThis);
	static const std::vector<CellStruct> GetFoundationCells(BuildingClass* pThis, CellStruct baseCoords, bool includeOccupyHeight = false);
	static WeaponStruct* GetLaserWeapon(BuildingClass* pThis);
	static void __stdcall UpdateFactoryQueues(BuildingClass* pThis);
	static void __fastcall KickOutClone(std::pair<TechnoTypeClass*, HouseClass*>& info, void*, BuildingClass* pFactory);
	static int GetTurretFrame(BuildingClass* pThis);
	static bool BuildingOnline(BuildingClass* pThis);
};

