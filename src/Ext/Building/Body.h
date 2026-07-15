#pragma once
#include <Ext/Techno/Body.h>
#include <Ext/BuildingType/Body.h>

class BuildingExt
{
public:
	using base_type = BuildingClass;

	static constexpr DWORD Canary = 0x87654321;
	static constexpr bool ShouldConsiderInvalidatePointer = true;

	// BuildingClassExtension is a leaf of the TechnoClass extension hierarchy: one extension
	// per object, stored inline in the shared 0x18 slot and owned by the TechnoClass container.
	class ExtData final : public TechnoExt::ExtData
	{
	public:
		BuildingTypeExt::ExtData* TypeExtData;
		TechnoExt::ExtData* TechnoExtData;
		bool DeployedTechno;
		bool IsCreatedFromMapFile;
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

		ExtData(BuildingClass* OwnerObject) : TechnoExt::ExtData(OwnerObject)
			, TypeExtData { nullptr }
			, TechnoExtData { this }
			, DeployedTechno { false }
			, IsCreatedFromMapFile { false }
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
		{ }

		// typed owner accessor (shadows the TechnoClass one from the base)
		BuildingClass* OwnerObject() const
		{
			return static_cast<BuildingClass*>(this->TechnoExt::ExtData::OwnerObject());
		}

		void DisplayIncomeString();
		void ApplyPoweredKillSpawns();
		bool HasSuperWeapon(int index) const;
		bool HandleInfiltrate(HouseClass* pInfiltratorHouse, int moneybefore);
		void UpdatePrimaryFactoryAI();
		virtual ~ExtData() = default;

		// virtual void LoadFromINIFile(CCINIClass* pINI) override;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override
		{
			TechnoExt::ExtData::InvalidatePointer(ptr, bRemoved);

			if (bRemoved)
				AnnounceInvalidPointer(this->CurrentAirFactory, ptr);
		}

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	// BuildingClassExtension lives in the TechnoClass container (single 0x18 slot), so this
	// is a thin typed accessor facade over that container instead of an owning container.
	class ExtMapFacade
	{
	public:
		ExtData* Find(BuildingClass* key) const
		{
			return static_cast<ExtData*>(TechnoExt::ExtMap.Find(key));
		}

		ExtData* TryFind(BuildingClass* key) const
		{
			return static_cast<ExtData*>(TechnoExt::ExtMap.TryFind(key));
		}
	};

	static ExtMapFacade ExtMap;

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
	static void __fastcall KickOutClone(std::pair<TechnoTypeClass*, HouseClass*>& info, void*, BuildingClass* pFactory);
	static int GetTurretFrame(BuildingClass* pThis);
};
