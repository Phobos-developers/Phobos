#pragma once

#include <HouseTypeClass.h>

#include <Ext/AbstractType/Body.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <New/Type/EVATypeClass.h>

class HouseTypeExt final : public AbstractTypeExt
{
public:
	using base_type = HouseTypeClass;

	// deprecated: the pre-rework nested data class is now the extension class itself
	using ExtData [[deprecated("use the extension class itself instead")]] = HouseTypeExt;

	static constexpr DWORD Canary = 0xAFFEAFFE;

public:
	// typed owner accessor
	HouseTypeClass* OwnerObject() const
	{
		return static_cast<HouseTypeClass*>(this->GetAttachedObject());
	}

	EVAType EVATag;
	Nullable<int> DropshipLoadout_StartingDropships;
	ValueableVector<TechnoTypeClass*> DropshipLoadout_AllowableUnits;
	ValueableVector<int> DropshipLoadout_AllowableUnitMaximums;
	std::map<int, std::vector<TechnoTypeClass*>> DropshipLoadout_AllowableUnitsLists;
	std::map<int, std::vector<int>> DropshipLoadout_AllowableUnitMaximumsLists;
	Nullable<int> DropshipLoadout_Theme;
	Nullable<int> DropshipLoadout_Money;
	NullableIdx<VoxClass> DropshipLoadout_StartEVA;
	ValueableVector<TechnoTypeClass*> DropshipLoadout_Carriers;
	ValueableVector<int> DropshipLoadout_Carriers_SizeLimit;
	Nullable<bool> DropshipLoadout_AddUnusedMoneyToPlayer;
	Nullable<bool> DropshipLoadout_RememberPurchasedCargo;
	ConvertClass* DropshipLoadout_Palette;

	Nullable<PhobosPCXFile> DropshipLoadout_BackgroundPCX;
	std::string DropshipLoadout_BackgroundPCXPattern; // raw format string, e.g. "DROP%04d.PCX"
	Nullable<PhobosPCXFile> DropshipLoadout_UpArrowPCX;
	Nullable<PhobosPCXFile> DropshipLoadout_DownArrowPCX;
	ValueableVector<PhobosPCXFile> DropshipLoadout_LoadoutPCX;
	ValueableVector<PhobosPCXFile> DropshipLoadout_PilotLitPCX;
	ValueableVector<std::unique_ptr<std::vector<PhobosPCXFile>>> DropshipLoadout_DGreenListPCX;
	Nullable<int> DropshipLoadout_DGreenAnimationsCount;
	ValueableVector<Point2D> DropshipLoadout_DGreenLocations;
	Nullable<Point2D> DropshipLoadout_UpArrowLocation;
	Nullable<Point2D> DropshipLoadout_DownArrowLocation;
	Nullable<Point2D> DropshipLoadout_LoadoutLocation;
	Nullable<Point2D> DropshipLoadout_PilotLitLocation;
	Nullable<int> DropshipLoadout_SidebarCameosCount;
	ValueableVector<Point2D> DropshipLoadout_SidebarCameoLocations;
	Nullable<int> DropshipLoadout_DropshipCameosCount;
	ValueableVector<std::vector<Point2D>> DropshipLoadout_DropshipCameoLocations;
	std::vector<std::vector<TechnoTypeClass*>> DropshipLoadout_FixedUnits;
	std::vector<std::vector<TechnoTypeClass*>> DropshipLoadout_InitialUnits;
	NullableIdx<VocClass> DropshipLoadout_BuyClickSound;
	NullableIdx<VocClass> DropshipLoadout_SellClickSound;
	NullableIdx<VocClass> DropshipLoadout_ArrowsClickSound;
	NullableIdx<VocClass> DropshipLoadout_StartingDragDropSound;
	NullableIdx<VocClass> DropshipLoadout_EndingDragDropSound;

	HouseTypeExt(HouseTypeClass* OwnerObject) : AbstractTypeExt(OwnerObject)
		, EVATag { -2 }
		, DropshipLoadout_StartingDropships {}
		, DropshipLoadout_AllowableUnits {}
		, DropshipLoadout_AllowableUnitMaximums {}
		, DropshipLoadout_AllowableUnitsLists {}
		, DropshipLoadout_AllowableUnitMaximumsLists {}
		, DropshipLoadout_Theme {}
		, DropshipLoadout_Money {}
		, DropshipLoadout_StartEVA {}
		, DropshipLoadout_Carriers {}
		, DropshipLoadout_Carriers_SizeLimit {}
		, DropshipLoadout_AddUnusedMoneyToPlayer {}
		, DropshipLoadout_RememberPurchasedCargo {}
		, DropshipLoadout_Palette { nullptr }
		, DropshipLoadout_BackgroundPCX {}
		, DropshipLoadout_UpArrowPCX {}
		, DropshipLoadout_DownArrowPCX {}
		, DropshipLoadout_LoadoutPCX {}
		, DropshipLoadout_LoadoutLocation {}
		, DropshipLoadout_PilotLitPCX {}
		, DropshipLoadout_PilotLitLocation {}
		, DropshipLoadout_DGreenListPCX {}
		, DropshipLoadout_DGreenAnimationsCount {}
		, DropshipLoadout_DGreenLocations {}
		, DropshipLoadout_UpArrowLocation {}
		, DropshipLoadout_DownArrowLocation {}
		, DropshipLoadout_SidebarCameosCount {}
		, DropshipLoadout_SidebarCameoLocations {}
		, DropshipLoadout_DropshipCameosCount {}
		, DropshipLoadout_DropshipCameoLocations {}
		, DropshipLoadout_FixedUnits {}
		, DropshipLoadout_InitialUnits {}
		, DropshipLoadout_BuyClickSound {}
		, DropshipLoadout_SellClickSound {}
		, DropshipLoadout_ArrowsClickSound {}
		, DropshipLoadout_StartingDragDropSound {}
		, DropshipLoadout_EndingDragDropSound {}
	{ }

	virtual ~HouseTypeExt() = default;

	virtual void LoadFromINIFile(CCINIClass* pINI) override;
	virtual void Initialize() override;
	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);

public:
	class ExtContainer final : public Container<HouseTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static ExtContainer ExtMap;

	static HouseTypeExt* Fetch(const HouseTypeClass* pThis)
	{
		return AbstractExt::Fetch<HouseTypeExt>(pThis);
	}

	static HouseTypeExt* TryFetch(const HouseTypeClass* pThis)
	{
		return AbstractExt::TryFetch<HouseTypeExt>(pThis);
	}
};

