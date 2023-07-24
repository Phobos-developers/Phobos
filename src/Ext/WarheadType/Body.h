#pragma once
#include <WarheadTypeClass.h>
#include <SuperWeaponTypeClass.h>
#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <New/Type/ShieldTypeClass.h>
#include <Ext/Bullet/Body.h>

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
		Valueable<bool> TransactMoney_Display;
		Valueable<AffectedHouse> TransactMoney_Display_Houses;
		Valueable<bool> TransactMoney_Display_AtFirer;
		Valueable<Point2D> TransactMoney_Display_Offset;
		ValueableVector<AnimTypeClass*> SplashList;
		Valueable<bool> SplashList_PickRandom;
		Valueable<bool> RemoveDisguise;
		Valueable<bool> RemoveMindControl;
		Valueable<bool> AnimList_PickRandom;
		Valueable<bool> DecloakDamagedTargets;
		Valueable<bool> ShakeIsLocal;

		Valueable<double> Crit_Chance;
		Valueable<bool> Crit_ApplyChancePerTarget;
		Valueable<int> Crit_ExtraDamage;
		Nullable<WarheadTypeClass*> Crit_Warhead;
		Valueable<AffectedTarget> Crit_Affects;
		ValueableVector<AnimTypeClass*> Crit_AnimList;
		Nullable<bool> Crit_AnimList_PickRandom;
		Valueable<bool> Crit_AnimOnAffectedTargets;
		Valueable<double> Crit_AffectBelowPercent;
		Valueable<bool> Crit_SuppressWhenIntercepted;

		Nullable<AnimTypeClass*> MindControl_Anim;

		Valueable<bool> Shield_Penetrate;
		Valueable<bool> Shield_Break;
		Nullable<AnimTypeClass*> Shield_BreakAnim;
		Nullable<AnimTypeClass*> Shield_HitAnim;
		Nullable<WeaponTypeClass*> Shield_BreakWeapon;

		Nullable<double> Shield_AbsorbPercent;
		Nullable<double> Shield_PassPercent;

		Valueable<int> Shield_Respawn_Duration;
		Valueable<double> Shield_Respawn_Amount;
		Valueable<int> Shield_Respawn_Rate;
		Valueable<bool> Shield_Respawn_ResetTimer;
		Valueable<int> Shield_SelfHealing_Duration;
		Nullable<double> Shield_SelfHealing_Amount;
		Valueable<int> Shield_SelfHealing_Rate;
		Valueable<bool> Shield_SelfHealing_ResetTimer;

		ValueableVector<ShieldTypeClass*> Shield_AttachTypes;
		ValueableVector<ShieldTypeClass*> Shield_RemoveTypes;
		Valueable<bool> Shield_ReplaceOnly;
		Valueable<bool> Shield_ReplaceNonRespawning;
		Valueable<bool> Shield_InheritStateOnReplace;
		Valueable<int> Shield_MinimumReplaceDelay;
		ValueableVector<ShieldTypeClass*> Shield_AffectTypes;

		Valueable<int> NotHuman_DeathSequence;
		ValueableVector<SuperWeaponTypeClass*> LaunchSW;
		Valueable<bool> LaunchSW_RealLaunch;
		Valueable<bool> LaunchSW_IgnoreInhibitors;
		Valueable<bool> LaunchSW_IgnoreDesignators;
		Valueable<bool> AllowDamageOnSelf;

		Valueable<bool> DetonateOnAllMapObjects;
		Valueable<bool> DetonateOnAllMapObjects_RequireVerses;
		Valueable<AffectedTarget> DetonateOnAllMapObjects_AffectTargets;
		Valueable<AffectedHouse> DetonateOnAllMapObjects_AffectHouses;
		ValueableVector<TechnoTypeClass*> DetonateOnAllMapObjects_AffectTypes;
		ValueableVector<TechnoTypeClass*> DetonateOnAllMapObjects_IgnoreTypes;

		// Ares tags
		// http://ares-developers.github.io/Ares-docs/new/warheads/general.html
		Valueable<bool> AffectsEnemies;
		Nullable<bool> AffectsOwner;

		double RandomBuffer;
		bool HasCrit;
		bool WasDetonatedOnAllMapObjects;

	private:
		Valueable<double> Shield_Respawn_Rate_InMinutes;
		Valueable<double> Shield_SelfHealing_Rate_InMinutes;

	public:
		ExtData(WarheadTypeClass* OwnerObject) : Extension<WarheadTypeClass>(OwnerObject)
			, SpySat { false }
			, BigGap { false }
			, TransactMoney { 0 }
			, TransactMoney_Display { false }
			, TransactMoney_Display_Houses { AffectedHouse::All }
			, TransactMoney_Display_AtFirer { false }
			, TransactMoney_Display_Offset { { 0, 0 } }
			, SplashList {}
			, SplashList_PickRandom { false }
			, RemoveDisguise { false }
			, RemoveMindControl { false }
			, AnimList_PickRandom { false }
			, DecloakDamagedTargets { true }
			, ShakeIsLocal { false }

			, Crit_Chance { 0.0 }
			, Crit_ApplyChancePerTarget { false }
			, Crit_ExtraDamage { 0 }
			, Crit_Warhead {}
			, Crit_Affects { AffectedTarget::All }
			, Crit_AnimList {}
			, Crit_AnimList_PickRandom {}
			, Crit_AnimOnAffectedTargets { false }
			, Crit_AffectBelowPercent { 1.0 }
			, Crit_SuppressWhenIntercepted { false }

			, MindControl_Anim {}

			, Shield_Penetrate { false }
			, Shield_Break { false }
			, Shield_BreakAnim {}
			, Shield_HitAnim {}
			, Shield_BreakWeapon {}
			, Shield_AbsorbPercent {}
			, Shield_PassPercent {}

			, Shield_Respawn_Duration { 0 }
			, Shield_Respawn_Amount { 0.0 }
			, Shield_Respawn_Rate { -1 }
			, Shield_Respawn_Rate_InMinutes { -1.0 }
			, Shield_Respawn_ResetTimer { false }
			, Shield_SelfHealing_Duration { 0 }
			, Shield_SelfHealing_Amount { }
			, Shield_SelfHealing_Rate { -1 }
			, Shield_SelfHealing_Rate_InMinutes { -1.0 }
			, Shield_SelfHealing_ResetTimer { false }
			, Shield_AttachTypes {}
			, Shield_RemoveTypes {}
			, Shield_ReplaceOnly { false }
			, Shield_ReplaceNonRespawning { false }
			, Shield_InheritStateOnReplace { false }
			, Shield_MinimumReplaceDelay { 0 }
			, Shield_AffectTypes {}

			, NotHuman_DeathSequence { -1 }
			, LaunchSW {}
			, LaunchSW_RealLaunch { true }
			, LaunchSW_IgnoreInhibitors { false }
			, LaunchSW_IgnoreDesignators { true }
			, AllowDamageOnSelf { false }

			, DetonateOnAllMapObjects { false }
			, DetonateOnAllMapObjects_RequireVerses { false }
			, DetonateOnAllMapObjects_AffectTargets { AffectedTarget::None }
			, DetonateOnAllMapObjects_AffectHouses { AffectedHouse::None }
			, DetonateOnAllMapObjects_AffectTypes {}
			, DetonateOnAllMapObjects_IgnoreTypes {}

			, AffectsEnemies { true }
			, AffectsOwner {}

			, RandomBuffer { 0.0 }
			, HasCrit { false }
			, WasDetonatedOnAllMapObjects { false }
		{ }

	private:
		void DetonateOnOneUnit(HouseClass* pHouse, TechnoClass* pTarget, TechnoClass* pOwner = nullptr, bool bulletWasIntercepted = false);

		void ApplyRemoveDisguiseToInf(HouseClass* pHouse, TechnoClass* pTarget);
		void ApplyRemoveMindControl(HouseClass* pHouse, TechnoClass* pTarget);
		void ApplyCrit(HouseClass* pHouse, TechnoClass* pTarget, TechnoClass* Owner);
		void ApplyShieldModifiers(TechnoClass* pTarget);

	public:
		void Detonate(TechnoClass* pOwner, HouseClass* pHouse, BulletExt::ExtData* pBullet, CoordStruct coords);
		bool CanTargetHouse(HouseClass* pHouse, TechnoClass* pTechno);
		void InterceptBullets(TechnoClass* pOwner, WeaponTypeClass* pWeapon, CoordStruct coords);
		bool EligibleForFullMapDetonation(TechnoClass* pTechno, HouseClass* pOwner);

		virtual ~ExtData() = default;
		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<WarheadTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
	};

	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static void DetonateAt(WarheadTypeClass* pThis, ObjectClass* pTarget, TechnoClass* pOwner, int damage, HouseClass* pFiringHouse = nullptr);
	static void DetonateAt(WarheadTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, int damage, HouseClass* pFiringHouse = nullptr);
};
