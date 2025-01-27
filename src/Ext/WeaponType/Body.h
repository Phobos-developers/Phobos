#pragma once
#include <BulletClass.h>
#include <WeaponTypeClass.h>
#include <DiskLaserClass.h>
#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <New/Type/RadTypeClass.h>
#include <New/Type/AttachEffectTypeClass.h>

class WeaponTypeExt
{
public:
	using base_type = WeaponTypeClass;

	static constexpr DWORD Canary = 0x22222222;
	static constexpr size_t ExtPointerOffset = 0x18;

	class ExtData final : public Extension<WeaponTypeClass>
	{
	public:

		Valueable<double> DiskLaser_Radius;
		Valueable<Leptons> ProjectileRange;
		Valueable<RadTypeClass*> RadType;
		Valueable<bool> Bolt_Disable1;
		Valueable<bool> Bolt_Disable2;
		Valueable<bool> Bolt_Disable3;
		Valueable<int> Bolt_Arcs;
		Nullable<bool> Strafing;
		Nullable<int> Strafing_Shots;
		Valueable<bool> Strafing_SimulateBurst;
		Valueable<bool> Strafing_UseAmmoPerShot;
		Valueable<AffectedTarget> CanTarget;
		Valueable<AffectedHouse> CanTargetHouses;
		ValueableVector<int> Burst_Delays;
		Valueable<bool> Burst_FireWithinSequence;
		Valueable<AreaFireTarget> AreaFire_Target;
		Valueable<WeaponTypeClass*> FeedbackWeapon;
		Valueable<bool> Laser_IsSingleColor;
		Nullable<PartialVector2D<int>> ROF_RandomDelay;
		ValueableVector<int> ChargeTurret_Delays;
		Valueable<bool> OmniFire_TurnToTarget;
		Valueable<bool> FireOnce_ResetSequence;
		ValueableVector<WarheadTypeClass*> ExtraWarheads;
		ValueableVector<int> ExtraWarheads_DamageOverrides;
		ValueableVector<double> ExtraWarheads_DetonationChances;
		ValueableVector<bool> ExtraWarheads_FullDetonation;
		Nullable<WarheadTypeClass*> AmbientDamage_Warhead;
		Valueable<bool> AmbientDamage_IgnoreTarget;
		ValueableVector<AttachEffectTypeClass*> AttachEffect_RequiredTypes;
		ValueableVector<AttachEffectTypeClass*> AttachEffect_DisallowedTypes;
		std::vector<std::string> AttachEffect_RequiredGroups;
		std::vector<std::string> AttachEffect_DisallowedGroups;
		ValueableVector<int> AttachEffect_RequiredMinCounts;
		ValueableVector<int> AttachEffect_RequiredMaxCounts;
		ValueableVector<int> AttachEffect_DisallowedMinCounts;
		ValueableVector<int> AttachEffect_DisallowedMaxCounts;
		Valueable<bool> AttachEffect_CheckOnFirer;
		Valueable<bool> AttachEffect_IgnoreFromSameSource;
		Valueable<bool> KickOutPassengers;

		ExtData(WeaponTypeClass* OwnerObject) : Extension<WeaponTypeClass>(OwnerObject)
			, DiskLaser_Radius { DiskLaserClass::Radius }
			, ProjectileRange { Leptons(100000) }
			, RadType {}
			, Bolt_Disable1 { false }
			, Bolt_Disable2 { false }
			, Bolt_Disable3 { false }
			, Bolt_Arcs { 8 }
			, Strafing { }
			, Strafing_Shots {}
			, Strafing_SimulateBurst { false }
			, Strafing_UseAmmoPerShot { false }
			, CanTarget { AffectedTarget::All }
			, CanTargetHouses { AffectedHouse::All }
			, Burst_Delays {}
			, Burst_FireWithinSequence { false }
			, AreaFire_Target { AreaFireTarget::Base }
			, FeedbackWeapon {}
			, Laser_IsSingleColor { false }
			, ROF_RandomDelay {}
			, ChargeTurret_Delays {}
			, OmniFire_TurnToTarget { false }
			, FireOnce_ResetSequence { true }
			, ExtraWarheads {}
			, ExtraWarheads_DamageOverrides {}
			, ExtraWarheads_DetonationChances {}
			, ExtraWarheads_FullDetonation {}
			, AmbientDamage_Warhead {}
			, AmbientDamage_IgnoreTarget { false }
			, AttachEffect_RequiredTypes {}
			, AttachEffect_DisallowedTypes {}
			, AttachEffect_RequiredGroups {}
			, AttachEffect_DisallowedGroups {}
			, AttachEffect_RequiredMinCounts {}
			, AttachEffect_RequiredMaxCounts {}
			, AttachEffect_DisallowedMinCounts {}
			, AttachEffect_DisallowedMaxCounts {}
			, AttachEffect_CheckOnFirer { false }
			, AttachEffect_IgnoreFromSameSource { false }
			, KickOutPassengers { true }
		{ }

		int GetBurstDelay(int burstIndex) const;

		bool HasRequiredAttachedEffects(TechnoClass* pTechno, TechnoClass* pFirer) const;

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void Initialize() override;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;

		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<WeaponTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static double OldRadius;

	static void DetonateAt(WeaponTypeClass* pThis, AbstractClass* pTarget, TechnoClass* pOwner, HouseClass* pFiringHouse = nullptr);
	static void DetonateAt(WeaponTypeClass* pThis, AbstractClass* pTarget, TechnoClass* pOwner, int damage, HouseClass* pFiringHouse = nullptr);
	static void DetonateAt(WeaponTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, HouseClass* pFiringHouse = nullptr, AbstractClass* pTarget = nullptr);
	static void DetonateAt(WeaponTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, int damage, HouseClass* pFiringHouse = nullptr, AbstractClass* pTarget = nullptr);
	static int GetRangeWithModifiers(WeaponTypeClass* pThis, TechnoClass* pFirer);
	static int GetRangeWithModifiers(WeaponTypeClass* pThis, TechnoClass* pFirer, int range);
};
