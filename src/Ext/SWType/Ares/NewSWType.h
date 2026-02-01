#pragma once
#include "../Body.h"

struct TargetingData
{
	struct LaunchSite
	{
		BuildingClass* Building;
		CellStruct Center;
		double MinRange;
		double MaxRange;
	};

	struct RangedItem
	{
		int RangeSqr;
		CellStruct Center;
	};

	SuperWeaponTypeClass** TypeExt_Ares;
	HouseClass* Owner;
	bool NeedsLaunchSite;
	bool NeedsDesignator;
	std::vector<LaunchSite> LaunchSites;
	std::vector<RangedItem> Designators;
	std::vector<RangedItem> Inhibitors;
};

class AresNewSWType
{
public:
	virtual ~AresNewSWType() = default;
	virtual bool CanFireAt(const TargetingData& data, const CellStruct cell, bool manual) const { return false; }
	virtual bool AbortFire(SuperClass* pSuper, bool isPlayer) const { return false; }
	virtual bool Activate(SuperClass* pSuper, const CellStruct cell, bool isPlayer) { return false; }
	virtual void Deactivate(SuperClass* pSuper, const CellStruct cell, bool isPlayer) { }
	virtual void Initialize(SuperWeaponTypeClass** pExt_Ares) const { }
	virtual void LoadFromINI(SuperWeaponTypeClass** pExt_Ares, CCINIClass* pINI) const { }
	virtual WarheadTypeClass* GetWarhead(const SuperWeaponTypeClass** pExt_Ares) const { return nullptr; }
	virtual AnimTypeClass* GetAnim(const SuperWeaponTypeClass** pExt_Ares) const { return nullptr; }
	virtual int GetSound(const SuperWeaponTypeClass** pExt_Ares) const { return -1; }
	virtual int GetDamage(const SuperWeaponTypeClass** pExt_Ares) const { return 0; }
	virtual std::pair<float, int> GetRange(const SuperWeaponTypeClass** pExt_Ares) const { return {}; }
	virtual const char* GetTypeString() const { return nullptr; }
	virtual bool HandlesType(SuperWeaponType type) const { return false; }
	virtual SuperWeaponFlags Flags() const { return SuperWeaponFlags::None; }
	virtual bool IsLaunchSite(SuperWeaponTypeClass** pExt_Ares, BuildingClass* pBuilding) const { return false; }
	virtual std::pair<double, double> GetLaunchSiteRange(SuperWeaponTypeClass** pExt_Ares, BuildingClass* pBuilding = nullptr) const { return {}; }
	virtual bool IsDesignator(SuperWeaponTypeClass** pExt_Ares, HouseClass* pOwner, TechnoClass* pTechno) const { return false; }
	virtual bool IsInhibitor(SuperWeaponTypeClass** pExt_Ares, HouseClass* pOwner, TechnoClass* pTechno) const { return false; }

	TargetingData& GetTargetingData(TargetingData& data, SuperWeaponTypeClass** pExt_Ares, HouseClass* pOwner) const;
};
