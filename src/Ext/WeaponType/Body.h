#pragma once
#include <BulletClass.h>
#include <WeaponTypeClass.h>
#include <DiskLaserClass.h>
#include <EBolt.h>
#include <ParticleSystemTypeClass.h>
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
		Nullable<ColorStruct> Bolt_Color[3];
		Valueable<bool> Bolt_Disable[3];
		Nullable<ParticleSystemTypeClass*> Bolt_ParticleSystem;
		Valueable<int> Bolt_Arcs;
		Valueable<int> Bolt_Duration;
		Nullable<bool> Bolt_FollowFLH;
		Nullable<bool> Strafing;
		Nullable<int> Strafing_Shots;
		Valueable<bool> Strafing_SimulateBurst;
		Valueable<bool> Strafing_UseAmmoPerShot;
		Valueable<bool> Strafing_TargetCell;
		Nullable<int> Strafing_EndDelay;
		Valueable<AffectedTarget> CanTarget;
		Valueable<AffectedHouse> CanTargetHouses;
		Valueable<double> CanTarget_MaxHealth;
		Valueable<double> CanTarget_MinHealth;
		ValueableVector<int> Burst_Delays;
		Valueable<bool> Burst_FireWithinSequence;
		Valueable<bool> Burst_NoDelay;
		Valueable<AreaFireTarget> AreaFire_Target;
		Valueable<WeaponTypeClass*> FeedbackWeapon;
		Valueable<bool> Laser_IsSingleColor;
		Valueable<bool> VisualScatter;
		Nullable<PartialVector2D<int>> ROF_RandomDelay;
		ValueableVector<int> ChargeTurret_Delays;
		Valueable<bool> OmniFire_TurnToTarget;
		Valueable<bool> FireOnce_ResetSequence;
		Valueable<bool> TurretRecoil_Suppress;
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
		Valueable<Leptons> KeepRange;
		Valueable<bool> KeepRange_AllowAI;
		Valueable<bool> KeepRange_AllowPlayer;
		Valueable<int> KeepRange_EarlyStopFrame;
		Valueable<bool> KickOutPassengers;
		Nullable<ColorStruct> Beam_Color;
		Valueable<int> Beam_Duration;
		Valueable<double> Beam_Amplitude;
		Valueable<bool> Beam_IsHouseColor;
		Valueable<int> LaserThickness;
		Nullable<PartialVector2D<int>> DelayedFire_Duration;
		Valueable<bool> DelayedFire_SkipInTransport;
		Valueable<AnimTypeClass*> DelayedFire_Animation;
		Nullable<AnimTypeClass*> DelayedFire_OpenToppedAnimation;
		Valueable<bool> DelayedFire_AnimIsAttached;
		Valueable<bool> DelayedFire_CenterAnimOnFirer;
		Valueable<bool> DelayedFire_RemoveAnimOnNoDelay;
		Valueable<bool> DelayedFire_PauseFiringSequence;
		Valueable<bool> DelayedFire_OnlyOnInitialBurst;
		Nullable<CoordStruct> DelayedFire_AnimOffset;
		Valueable<bool> DelayedFire_AnimOnTurret;

		bool SkipWeaponPicking;

		ExtData(WeaponTypeClass* OwnerObject) : Extension<WeaponTypeClass>(OwnerObject)
			, DiskLaser_Radius { DiskLaserClass::Radius }
			, ProjectileRange { Leptons(100000) }
			, RadType {}
			, Bolt_Color {}
			, Bolt_Disable { Valueable<bool>(false) }
			, Bolt_ParticleSystem {}
			, Bolt_Arcs { 8 }
			, Bolt_Duration { 17 }
			, Bolt_FollowFLH {}
			, Strafing { }
			, Strafing_Shots {}
			, Strafing_SimulateBurst { false }
			, Strafing_UseAmmoPerShot { false }
			, Strafing_TargetCell { false }
			, Strafing_EndDelay {}
			, CanTarget { AffectedTarget::All }
			, CanTargetHouses { AffectedHouse::All }
			, CanTarget_MaxHealth { 1.0 }
			, CanTarget_MinHealth { 0.0 }
			, Burst_Delays {}
			, Burst_FireWithinSequence { false }
			, Burst_NoDelay { false }
			, AreaFire_Target { AreaFireTarget::Base }
			, FeedbackWeapon {}
			, Laser_IsSingleColor { false }
			, VisualScatter { false }
			, ROF_RandomDelay {}
			, ChargeTurret_Delays {}
			, OmniFire_TurnToTarget { false }
			, FireOnce_ResetSequence { true }
			, TurretRecoil_Suppress { false }
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
			, KeepRange { Leptons(0) }
			, KeepRange_AllowAI { false }
			, KeepRange_AllowPlayer { false }
			, KeepRange_EarlyStopFrame { 0 }
			, KickOutPassengers { true }
			, Beam_Color {}
			, Beam_Duration { 15 }
			, Beam_Amplitude { 40.0 }
			, Beam_IsHouseColor { false }
			, LaserThickness { 3 }
			, SkipWeaponPicking { true }
			, DelayedFire_Duration {}
			, DelayedFire_SkipInTransport { false }
			, DelayedFire_Animation {}
			, DelayedFire_OpenToppedAnimation {}
			, DelayedFire_AnimIsAttached { true }
			, DelayedFire_CenterAnimOnFirer { false }
			, DelayedFire_RemoveAnimOnNoDelay { false }
			, DelayedFire_PauseFiringSequence { false }
			, DelayedFire_OnlyOnInitialBurst { false }
			, DelayedFire_AnimOffset {}
			, DelayedFire_AnimOnTurret { true }
		{ }

		int GetBurstDelay(int burstIndex) const;
		bool HasRequiredAttachedEffects(TechnoClass* pTechno, TechnoClass* pFirer) const;
		bool IsHealthInThreshold(TechnoClass* pTarget) const;

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
	static int GetTechnoKeepRange(WeaponTypeClass* pThis, TechnoClass* pFirer, bool isMinimum);
};
