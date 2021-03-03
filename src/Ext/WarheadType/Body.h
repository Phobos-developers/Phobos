#pragma once

#include <CCINIClass.h>
#include <WarheadTypeClass.h>
#include <CellClass.h>

#include "../_Container.hpp"
#include "../../Phobos.h"

#include "../../Utilities/Debug.h"

enum class WarheadTarget : unsigned char {
	None = 0x0,
	Land = 0x1,
	Water = 0x2,
	NoContent = 0x4,
	Infantry = 0x8,
	Unit = 0x10,
	Building = 0x20,

	All = 0xFF,
	AllCells = Land | Water,
	AllTechnos = Infantry | Unit | Building,
	AllContents = NoContent | AllTechnos
};
MAKE_ENUM_FLAGS(WarheadTarget);

class WarheadTypeExt
{
public:
	using base_type = WarheadTypeClass;

	class ExtData final : public Extension<WarheadTypeClass>
	{
	public:

		bool SpySat;
		bool BigGap;
		int TransactMoney;
		char SplashList_Buffer[0x400];
		DynamicVectorClass<AnimTypeClass*> SplashList;
		bool SplashList_PickRandom;
		bool AffectsEnemies;
		int CritDamage;
		float CritSpread;
		float CritChance;
		char CritAnimsBuffer[0x100];
		char CritAffectsBuffer[0x100];
		WarheadTarget CritAffects;
		DynamicVectorClass<AnimTypeClass*> CritAnims;

		ExtData(WarheadTypeClass* OwnerObject) : Extension<WarheadTypeClass>(OwnerObject),
			SpySat(false),
			BigGap(false),
			TransactMoney(0),
			SplashList_Buffer(""),
			SplashList(),
			SplashList_PickRandom(false),
			AffectsEnemies(true),
			CritDamage(0),
			CritSpread(0.0),
			CritChance(0.0),
			CritAffects(WarheadTarget::None),
			CritAnimsBuffer(""),
			CritAffectsBuffer(""),
			CritAnims()
		{ }

		void ApplyCrit(const CoordStruct& coords, TechnoClass* const Owner);
		bool IsCellEligible(CellClass* const pCell, WarheadTarget allowed) noexcept;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override {}

		virtual void LoadFromStream(IStream* Stm);

		virtual void SaveToStream(IStream* Stm);
	};

	class ExtContainer final : public Container<WarheadTypeExt> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
	static bool CanAffectTarget(TechnoClass* const pTarget, HouseClass* const pSourceHouse, WarheadTypeClass* const pWarhead);
};
