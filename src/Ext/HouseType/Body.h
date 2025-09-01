#pragma once
#include <HouseTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class HouseTypeExt
{
public:
	using base_type = HouseTypeClass;
	static constexpr DWORD Canary = 0x11112222;

	class ExtData final : public Extension<HouseTypeClass>
	{
	public:
		Nullable<int> DropshipLoadout_StartingDropships;
		ValueableVector<TechnoTypeClass*> DropshipLoadout_AllowableUnits;
		ValueableVector<int> DropshipLoadout_AllowableUnitMaximums;
		Nullable<int> DropshipLoadout_Theme;
		Nullable<int> DropshipLoadout_Money;
		NullableIdx<VoxClass> DropshipLoadout_StartEVA;
		ValueableVector<TechnoTypeClass*> DropshipLoadout_Carriers;
		Nullable<bool> DropshipLoadout_AddUnusedMoneyToPlayer;

		Nullable<PhobosPCXFile> DropshipLoadout_BackgroundPCX;
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
		NullableIdx<VocClass> DropshipLoadout_BuyClickSound;
		NullableIdx<VocClass> DropshipLoadout_SellClickSound;
		NullableIdx<VocClass> DropshipLoadout_ArrowsClickSound;

		ExtData(HouseTypeClass* OwnerObject) : Extension<HouseTypeClass>(OwnerObject)
			, DropshipLoadout_StartingDropships {}
			, DropshipLoadout_AllowableUnits {}
			, DropshipLoadout_AllowableUnitMaximums {}
			, DropshipLoadout_Theme {}
			, DropshipLoadout_Money {}
			, DropshipLoadout_StartEVA {}
			, DropshipLoadout_Carriers {}
			, DropshipLoadout_AddUnusedMoneyToPlayer {}
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
			, DropshipLoadout_BuyClickSound {}
			, DropshipLoadout_SellClickSound {}
			, DropshipLoadout_ArrowsClickSound {}
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void Initialize() override;
		virtual void CompleteInitialization();

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override
		{
		}

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<HouseTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		virtual bool Load(HouseTypeClass* pThis, IStream* pStm) override;
	};

	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};
