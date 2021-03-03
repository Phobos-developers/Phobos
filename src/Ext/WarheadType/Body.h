#pragma once

#include <CCINIClass.h>
#include <WarheadTypeClass.h>

#include "../_Container.hpp"
#include "../../Phobos.h"

#include "../../Utilities/Debug.h"

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
		char CritAffects[0x100];
		char CritAnimsBuffer[0x100];
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
			CritAffects(""),
			CritAnimsBuffer(""),
			CritAnims()
		{ }

		void ApplyCrit(const CoordStruct& coords, TechnoClass* const Owner);

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
	static bool CanAffectTarget(TechnoClass* pTarget, HouseClass* pSourceHouse, WarheadTypeClass* pWarhead);
};
