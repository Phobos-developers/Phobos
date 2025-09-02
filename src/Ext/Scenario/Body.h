#pragma once

#include <ScenarioClass.h>
#include <MessageListClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <Ext/Techno/Body.h>

#include <map>

struct ExtendedVariable
{
	char Name[0x100];
	int Value;
};

class ScenarioExt
{
public:
	using base_type = ScenarioClass;

	static constexpr DWORD Canary = 0xABCD1595;

	class ExtData final : public Extension<ScenarioClass>
	{
	public:

		bool ShowBriefing;
		int BriefingTheme;

		std::map<int, CellStruct> Waypoints;
		std::map<int, ExtendedVariable> Variables[2]; // 0 for local, 1 for global

		std::vector<TechnoExt::ExtData*> AutoDeathObjects;
		std::vector<TechnoExt::ExtData*> TransportReloaders; // Objects that can reload ammo in limbo

		bool SWSidebar_Enable;
		std::vector<int> SWSidebar_Indices;

		std::unique_ptr<MessageListClass> NewMessageList;

		PhobosFixedString<64u> DefaultLS640BkgdName;
		PhobosFixedString<64u> DefaultLS800BkgdName;
		PhobosFixedString<64u> DefaultLS800BkgdPal;

		BulletClass* MasterDetonationBullet; // Used to do warhead/weapon detonations on spot without having to create new BulletClass instance every time.

		int DropshipLoadout_Theme;
		long DropshipLoadout_Money;
		NullableIdx<VoxClass> DropshipLoadout_StartEVA;
		std::vector<TechnoTypeClass*> DropshipLoadout_Carriers;
		bool DropshipLoadout_AddUnusedMoneyToPlayer;
		ConvertClass* DropshipLoadout_Palette;
		SHPStruct* DropshipLoadout_Background;
		SHPStruct* DropshipLoadout_UpArrow;
		SHPStruct* DropshipLoadout_DownArrow;
		SHPStruct* DropshipLoadout_Loadout;
		SHPStruct* DropshipLoadout_PilotLit;
		std::vector<SHPStruct*> DropshipLoadout_DGreenList;
		PhobosPCXFile DropshipLoadout_BackgroundPCX;
		PhobosPCXFile DropshipLoadout_UpArrowPCX;
		PhobosPCXFile DropshipLoadout_DownArrowPCX;
		std::vector<PhobosPCXFile> DropshipLoadout_LoadoutPCX;
		Point2D DropshipLoadout_LoadoutLocation;
		std::vector<PhobosPCXFile> DropshipLoadout_PilotLitPCX;
		Point2D DropshipLoadout_PilotLitLocation;
		std::vector<std::unique_ptr<std::vector<PhobosPCXFile>>> DropshipLoadout_DGreenListPCX;
		int DropshipLoadout_DGreenAnimationsCount;
		std::vector<Point2D> DropshipLoadout_DGreenLocations;
		Point2D DropshipLoadout_UpArrowLocation;
		Point2D DropshipLoadout_DownArrowLocation;
		int DropshipLoadout_SidebarCameosCount;
		std::vector<Point2D> DropshipLoadout_SidebarCameoLocations;
		int DropshipLoadout_DropshipCameosCount;
		std::vector<std::vector<Point2D>> DropshipLoadout_DropshipCameoLocations;
		//VocClass DropshipLoadout_SellClickSound;
		NullableIdx<VocClass> DropshipLoadout_BuyClickSound;
		NullableIdx<VocClass> DropshipLoadout_SellClickSound;
		NullableIdx<VocClass> DropshipLoadout_ArrowsClickSound;

		ExtData(ScenarioClass* OwnerObject) : Extension<ScenarioClass>(OwnerObject)
			, ShowBriefing { false }
			, BriefingTheme { -1 }
			, Waypoints { }
			, Variables { }
			, AutoDeathObjects {}
			, TransportReloaders {}
			, SWSidebar_Enable { true }
			, SWSidebar_Indices {}
			, NewMessageList {}
			, DefaultLS640BkgdName {}
			, DefaultLS800BkgdName {}
			, DefaultLS800BkgdPal {}
			, MasterDetonationBullet {}
			, DropshipLoadout_Theme { -1 }
			, DropshipLoadout_Money { -1 }
			, DropshipLoadout_StartEVA {}
			, DropshipLoadout_Carriers {}
			, DropshipLoadout_AddUnusedMoneyToPlayer { false }
			, DropshipLoadout_Palette { nullptr }
			, DropshipLoadout_Background { nullptr }
			, DropshipLoadout_UpArrow { nullptr }
			, DropshipLoadout_DownArrow { nullptr }
			, DropshipLoadout_Loadout { nullptr }
			, DropshipLoadout_PilotLit { nullptr }
			, DropshipLoadout_DGreenList {}
			, DropshipLoadout_BackgroundPCX {}
			, DropshipLoadout_UpArrowPCX {}
			, DropshipLoadout_DownArrowPCX {}
			, DropshipLoadout_LoadoutPCX {}
			, DropshipLoadout_LoadoutLocation {}
			, DropshipLoadout_PilotLitPCX {}
			, DropshipLoadout_PilotLitLocation {}
			, DropshipLoadout_DGreenListPCX {}
			, DropshipLoadout_DGreenAnimationsCount { 0 }
			, DropshipLoadout_DGreenLocations {}
			, DropshipLoadout_UpArrowLocation { Point2D::Empty }
			, DropshipLoadout_DownArrowLocation { Point2D::Empty }
			, DropshipLoadout_SidebarCameosCount { 0 }
			, DropshipLoadout_SidebarCameoLocations {}
			, DropshipLoadout_DropshipCameosCount { 0 }
			, DropshipLoadout_DropshipCameoLocations {}
			, DropshipLoadout_BuyClickSound {}
			, DropshipLoadout_SellClickSound {}
			, DropshipLoadout_ArrowsClickSound {}
		{ }
		
		void SetVariableToByID(bool bIsGlobal, int nIndex, char bState);
		void GetVariableStateByID(bool bIsGlobal, int nIndex, char* pOut);
		void ReadVariables(bool bIsGlobal, CCINIClass* pINI);
		static void SaveVariablesToFile(bool isGlobal);

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

		void UpdateAutoDeathObjectsInLimbo();
		void UpdateTransportReloaders();
	private:
		template <typename T>
		void Serialize(T& Stm);
	};

private:
	static std::unique_ptr<ExtData> Data;

public:
	static IStream* g_pStm;

	static bool CellParsed;

	static void Allocate(ScenarioClass* pThis);
	static void Remove(ScenarioClass* pThis);

	static void LoadFromINIFile(ScenarioClass* pThis, CCINIClass* pINI);

	static ExtData* Global()
	{
		return Data.get();
	}

	static void Clear()
	{
		Allocate(ScenarioClass::Instance);
	}

	static void PointerGotInvalid(void* ptr, bool removed)
	{
		Global()->InvalidatePointer(ptr, removed);
	}

};
