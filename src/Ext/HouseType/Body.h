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
		ValueableVector<std::unique_ptr<std::vector<PhobosPCXFile>>> DropshipLoadout_PilotLitPCX;
		ValueableVector<std::unique_ptr<std::vector<PhobosPCXFile>>> DropshipLoadout_DGreenListPCX;

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
			, DropshipLoadout_PilotLitPCX {}
			, DropshipLoadout_DGreenListPCX {}
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
