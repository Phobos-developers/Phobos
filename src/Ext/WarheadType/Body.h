#pragma once
#include <WarheadTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class WarheadTypeExt
{
public:
	using base_type = WarheadTypeClass;

	class ExtData final : public Extension<WarheadTypeClass>
	{
	public:

		Valueable<bool> SpySat;
		Valueable<bool> BigGap;
		Valueable<int> TransactMoney;
		ValueableVector<AnimTypeClass*> SplashList;
		Valueable<bool> SplashList_PickRandom;
		Valueable<bool> RemoveDisguise;
		Valueable<bool> RemoveMindControl;
		Valueable<bool> AnimList_PickRandom;

		Valueable<int> Crit_ExtraDamage;
		Valueable<double> Crit_Chance;
		Valueable<AffectedTarget> Crit_Affects;
		ValueableVector<AnimTypeClass*> Crit_AnimList;

		Nullable<AnimTypeClass*> MindControl_Anim;

		// Ares tags
		// http://ares-developers.github.io/Ares-docs/new/warheads/general.html
		Valueable<bool> AffectsEnemies;
		Nullable<bool> AffectsOwner;
		
		Valueable<bool> PenetratesShield;
		Valueable<bool> BreaksShield;

		double RandomBuffer;
		
		Valueable<int> InfDeathSequence;

		ExtData(WarheadTypeClass* OwnerObject) : Extension<WarheadTypeClass>(OwnerObject),
			SpySat(false),
			BigGap(false),
			TransactMoney(0),
			SplashList(),
			SplashList_PickRandom(false),
			RemoveDisguise(false),
			RemoveMindControl(false),
			AnimList_PickRandom(false),

			Crit_Chance(0.0),
			Crit_ExtraDamage(0),
			Crit_Affects(AffectedTarget::All),
			Crit_AnimList(),
			RandomBuffer(0.0),

			MindControl_Anim(),

			AffectsEnemies(true),
			AffectsOwner(),

			PenetratesShield(false),
			BreaksShield(false),
			InfDeathSequence(-1)
		{ }
	private:
		void DetonateOnOneUnit(HouseClass* pHouse, TechnoClass* pTarget, TechnoClass* pOwner = nullptr);

		void ApplyRemoveDisguiseToInf(HouseClass* pHouse, TechnoClass* pTarget);
		void ApplyRemoveMindControl(HouseClass* pHouse, TechnoClass* pTarget);
		void ApplyCrit(HouseClass* pHouse, TechnoClass* pTarget, TechnoClass* Owner);
	public:
		void Detonate(TechnoClass* pOwner, HouseClass* pHouse, BulletClass* pBullet, CoordStruct coords);
		bool CanTargetHouse(HouseClass* pHouse, TechnoClass* pTechno);

		bool IsCellEligible(CellClass* const pCell, AffectedTarget allowed);
		bool IsTechnoEligible(TechnoClass* const pCell, AffectedTarget allowed);

		virtual ~ExtData() = default;
		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override {}
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<WarheadTypeExt> {
	public:
		ExtContainer();
		~ExtContainer();

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
	};

	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};
