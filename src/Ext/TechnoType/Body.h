#pragma once
#include <TechnoTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <New/Type/ShieldTypeClass.h>
#include <New/Type/LaserTrailTypeClass.h>
#include <New/Type/Affiliated/InterceptorTypeClass.h>
#include <New/Type/Affiliated/PassengerDeletionTypeClass.h>
#include <New/Type/DigitalDisplayTypeClass.h>

class Matrix3D;

class TechnoTypeExt
{
public:
	using base_type = TechnoTypeClass;

	static constexpr DWORD Canary = 0x11111111;
	static constexpr size_t ExtPointerOffset = 0xDF4;

	class ExtData final : public Extension<TechnoTypeClass>
	{
	public:
		Valueable<bool> HealthBar_Hide;
		Valueable<CSFText> UIDescription;
		Valueable<bool> LowSelectionPriority;
		PhobosFixedString<0x20> GroupAs;
		Valueable<int> RadarJamRadius;
		Nullable<int> InhibitorRange;
		Nullable<int> DesignatorRange;
		Valueable<Leptons> MindControlRangeLimit;

		std::unique_ptr<InterceptorTypeClass> InterceptorType;

		Valueable<PartialVector3D<int>> TurretOffset;
		Nullable<bool> TurretShadow;
		ValueableVector<int> ShadowIndices;
		Valueable<bool> Spawner_LimitRange;
		Valueable<int> Spawner_ExtraLimitRange;
		Nullable<int> Spawner_DelayFrames;
		Nullable<bool> Harvester_Counted;
		Valueable<bool> Promote_IncludeSpawns;
		Valueable<bool> ImmuneToCrit;
		Valueable<bool> MultiMindControl_ReleaseVictim;
		Valueable<int> CameoPriority;
		Valueable<bool> NoManualMove;
		Nullable<int> InitialStrength;

		Valueable<ShieldTypeClass*> ShieldType;
		std::unique_ptr<PassengerDeletionTypeClass> PassengerDeletionType;

		Nullable<AutoDeathBehavior> AutoDeath_Behavior;
		Nullable<AnimTypeClass*> AutoDeath_VanishAnimation;
		Valueable<bool> AutoDeath_OnAmmoDepletion;
		Valueable<int> AutoDeath_AfterDelay;
		ValueableVector<TechnoTypeClass*> AutoDeath_TechnosDontExist;
		Valueable<bool> AutoDeath_TechnosDontExist_Any;
		Valueable<bool> AutoDeath_TechnosDontExist_AllowLimboed;
		Valueable<AffectedHouse> AutoDeath_TechnosDontExist_Houses;
		ValueableVector<TechnoTypeClass*> AutoDeath_TechnosExist;
		Valueable<bool> AutoDeath_TechnosExist_Any;
		Valueable<bool> AutoDeath_TechnosExist_AllowLimboed;
		Valueable<AffectedHouse> AutoDeath_TechnosExist_Houses;

		Valueable<SlaveChangeOwnerType> Slaved_OwnerWhenMasterKilled;
		NullableIdx<VocClass> SlavesFreeSound;
		NullableIdx<VocClass> SellSound;
		NullableIdx<VoxClass> EVA_Sold;

		NullableIdx<VocClass> VoiceCreated;

		Nullable<AnimTypeClass*> WarpOut;
		Nullable<AnimTypeClass*> WarpIn;
		Nullable<AnimTypeClass*> WarpAway;
		Nullable<bool> ChronoTrigger;
		Nullable<int> ChronoDistanceFactor;
		Nullable<int> ChronoMinimumDelay;
		Nullable<int> ChronoRangeMinimum;
		Nullable<int> ChronoDelay;

		Nullable<WeaponTypeClass*> WarpInWeapon;
		Nullable<WeaponTypeClass*> WarpInMinRangeWeapon;
		Nullable<WeaponTypeClass*> WarpOutWeapon;
		Valueable<bool> WarpInWeapon_UseDistanceAsDamage;

		ValueableVector<AnimTypeClass*> OreGathering_Anims;
		ValueableVector<int> OreGathering_Tiberiums;
		ValueableVector<int> OreGathering_FramesPerDir;

		std::vector<std::vector<CoordStruct>> WeaponBurstFLHs;
		std::vector<std::vector<CoordStruct>> EliteWeaponBurstFLHs;
		std::vector<CoordStruct> AlternateFLHs;

		Valueable<bool> DestroyAnim_Random;
		Valueable<bool> NotHuman_RandomDeathSequence;

		Nullable<InfantryTypeClass*> DefaultDisguise;
		Valueable<bool> UseDisguiseMovementSpeed;

		Nullable<int> OpenTopped_RangeBonus;
		Nullable<float> OpenTopped_DamageMultiplier;
		Nullable<int> OpenTopped_WarpDistance;
		Valueable<bool> OpenTopped_IgnoreRangefinding;
		Valueable<bool> OpenTopped_AllowFiringIfDeactivated;
		Valueable<bool> OpenTopped_ShareTransportTarget;

		Valueable<bool> AutoFire;
		Valueable<bool> AutoFire_TargetSelf;

		Valueable<bool> NoSecondaryWeaponFallback;
		Valueable<bool> NoSecondaryWeaponFallback_AllowAA;

		Valueable<int> NoAmmoWeapon;
		Valueable<int> NoAmmoAmount;

		Valueable<bool> JumpjetRotateOnCrash;

		Valueable<bool> DeployingAnim_AllowAnyDirection;
		Valueable<bool> DeployingAnim_KeepUnitVisible;
		Valueable<bool> DeployingAnim_ReverseForUndeploy;
		Valueable<bool> DeployingAnim_UseUnitDrawer;

		Valueable<CSFText> EnemyUIName;
		Valueable<int> ForceWeapon_Naval_Decloaked;
		Valueable<int> ForceWeapon_Cloaked;
		Valueable<int> ForceWeapon_Disguised;

		Valueable<bool> Ammo_Shared;
		Valueable<int> Ammo_Shared_Group;

		Nullable<SelfHealGainType> SelfHealGainType;
		Valueable<bool> Passengers_SyncOwner;
		Valueable<bool> Passengers_SyncOwner_RevertOnExit;

		Nullable<bool> IronCurtain_KeptOnDeploy;
		Nullable<IronCurtainEffect> IronCurtain_Effect;
		Nullable<WarheadTypeClass*> IronCurtain_KillWarhead;
		Valueable<bool> Explodes_KillPassengers;
		Nullable<int> DeployFireWeapon;
		Valueable<TargetZoneScanType> TargetZoneScanType;

		Promotable<SHPStruct*> Insignia;
		Valueable<Vector3D<int>> InsigniaFrames;
		Promotable<int> InsigniaFrame;
		Nullable<bool> Insignia_ShowEnemy;
		std::vector<Promotable<SHPStruct*>> Insignia_Weapon;
		std::vector<Promotable<int>> InsigniaFrame_Weapon;
		std::vector<Vector3D<int>> InsigniaFrames_Weapon;

		Nullable<bool> TiltsWhenCrushes_Vehicles;
		Nullable<bool> TiltsWhenCrushes_Overlays;
		Nullable<double> CrushForwardTiltPerFrame;
		Valueable<double> CrushOverlayExtraForwardTilt;
		Valueable<double> CrushSlowdownMultiplier;

		Valueable<bool> DigitalDisplay_Disable;
		ValueableVector<DigitalDisplayTypeClass*> DigitalDisplayTypes;

		Valueable<int> AmmoPip;
		Valueable<int> EmptyAmmoPip;
		Valueable<int> PipWrapAmmoPip;
		Nullable<Point2D> AmmoPipSize;

		struct LaserTrailDataEntry
		{
			ValueableIdx<LaserTrailTypeClass> idxType;
			Valueable<CoordStruct> FLH;
			Valueable<bool> IsOnTurret;

			bool Load(PhobosStreamReader& stm, bool registerForChange);
			bool Save(PhobosStreamWriter& stm) const;

		private:
			template <typename T>
			bool Serialize(T& stm);
		};

		ValueableVector<LaserTrailDataEntry> LaserTrailData;

		Nullable<CoordStruct> PronePrimaryFireFLH;
		Nullable<CoordStruct> ProneSecondaryFireFLH;
		Nullable<CoordStruct> DeployedPrimaryFireFLH;
		Nullable<CoordStruct> DeployedSecondaryFireFLH;
		std::vector<std::vector<CoordStruct>> CrouchedWeaponBurstFLHs;
		std::vector<std::vector<CoordStruct>> EliteCrouchedWeaponBurstFLHs;
		std::vector<std::vector<CoordStruct>> DeployedWeaponBurstFLHs;
		std::vector<std::vector<CoordStruct>> EliteDeployedWeaponBurstFLHs;

		ExtData(TechnoTypeClass* OwnerObject) : Extension<TechnoTypeClass>(OwnerObject)
			, HealthBar_Hide { false }
			, UIDescription {}
			, LowSelectionPriority { false }
			, GroupAs { NONE_STR }
			, RadarJamRadius { 0 }
			, InhibitorRange {}
			, DesignatorRange { }
			, MindControlRangeLimit {}

			, InterceptorType { nullptr }

			, TurretOffset { { 0, 0, 0 } }
			, TurretShadow { }
			, ShadowIndices { }
			, Spawner_LimitRange { false }
			, Spawner_ExtraLimitRange { 0 }
			, Spawner_DelayFrames {}
			, Harvester_Counted {}
			, Promote_IncludeSpawns { false }
			, ImmuneToCrit { false }
			, MultiMindControl_ReleaseVictim { false }
			, CameoPriority { 0 }
			, NoManualMove { false }
			, InitialStrength {}
			, ShieldType {}
			, PassengerDeletionType { nullptr}

			, WarpOut {}
			, WarpIn {}
			, WarpAway {}
			, ChronoTrigger {}
			, ChronoDistanceFactor {}
			, ChronoMinimumDelay {}
			, ChronoRangeMinimum {}
			, ChronoDelay {}
			, WarpInWeapon {}
			, WarpInMinRangeWeapon {}
			, WarpOutWeapon {}
			, WarpInWeapon_UseDistanceAsDamage { false }

			, OreGathering_Anims {}
			, OreGathering_Tiberiums {}
			, OreGathering_FramesPerDir {}
			, LaserTrailData {}
			, DestroyAnim_Random { true }
			, NotHuman_RandomDeathSequence { false }

			, DefaultDisguise {}
			, UseDisguiseMovementSpeed {}

			, OpenTopped_RangeBonus {}
			, OpenTopped_DamageMultiplier {}
			, OpenTopped_WarpDistance {}
			, OpenTopped_IgnoreRangefinding { false }
			, OpenTopped_AllowFiringIfDeactivated { true }
			, OpenTopped_ShareTransportTarget { true }

			, AutoFire { false }
			, AutoFire_TargetSelf { false }
			, NoSecondaryWeaponFallback { false }
			, NoSecondaryWeaponFallback_AllowAA { false }
			, NoAmmoWeapon { -1 }
			, NoAmmoAmount { 0 }
			, JumpjetRotateOnCrash { true }

			, DeployingAnim_AllowAnyDirection { false }
			, DeployingAnim_KeepUnitVisible { false }
			, DeployingAnim_ReverseForUndeploy { true }
			, DeployingAnim_UseUnitDrawer { true }

			, AutoDeath_Behavior { }
			, AutoDeath_VanishAnimation {}
			, AutoDeath_OnAmmoDepletion { false }
			, AutoDeath_AfterDelay { 0 }
			, AutoDeath_TechnosDontExist {}
			, AutoDeath_TechnosDontExist_Any { false }
			, AutoDeath_TechnosDontExist_AllowLimboed { false }
			, AutoDeath_TechnosDontExist_Houses { AffectedHouse::Owner }
			, AutoDeath_TechnosExist {}
			, AutoDeath_TechnosExist_Any { true }
			, AutoDeath_TechnosExist_AllowLimboed { true }
			, AutoDeath_TechnosExist_Houses { AffectedHouse::Owner }

			, Slaved_OwnerWhenMasterKilled { SlaveChangeOwnerType::Killer }
			, SlavesFreeSound {}
			, SellSound {}
			, EVA_Sold {}
			, EnemyUIName {}

			, ForceWeapon_Naval_Decloaked { -1 }
			, ForceWeapon_Cloaked { -1 }
			, ForceWeapon_Disguised { -1 }

			, Ammo_Shared { false }
			, Ammo_Shared_Group { -1 }

			, SelfHealGainType {}
			, Passengers_SyncOwner { false }
			, Passengers_SyncOwner_RevertOnExit { true }

			, PronePrimaryFireFLH {}
			, ProneSecondaryFireFLH {}
			, DeployedPrimaryFireFLH {}
			, DeployedSecondaryFireFLH {}

			, IronCurtain_KeptOnDeploy {}
			, IronCurtain_Effect {}
			, IronCurtain_KillWarhead {}

			, Explodes_KillPassengers { true }
			, DeployFireWeapon {}
			, TargetZoneScanType { TargetZoneScanType::Same }

			, Insignia {}
			, InsigniaFrames { { -1, -1, -1 } }
			, InsigniaFrame { -1 }
			, Insignia_ShowEnemy {}
			, Insignia_Weapon {}
			, InsigniaFrame_Weapon {}
			, InsigniaFrames_Weapon {}

			, TiltsWhenCrushes_Vehicles {}
			, TiltsWhenCrushes_Overlays {}
			, CrushSlowdownMultiplier { 0.2 }
			, CrushForwardTiltPerFrame {}
			, CrushOverlayExtraForwardTilt { 0.02 }

			, DigitalDisplay_Disable { false }
			, DigitalDisplayTypes {}

			, AmmoPip { 13 }
			, EmptyAmmoPip { -1 }
			, PipWrapAmmoPip { 14 }
			, AmmoPipSize {}
		{ }

		virtual ~ExtData() = default;
		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void Initialize() override;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

		void ApplyTurretOffset(Matrix3D* mtx, double factor = 1.0);

		// Ares 0.A
		const char* GetSelectionGroupID() const;

	private:
		template <typename T>
		void Serialize(T& Stm);

		void ParseBurstFLHs(INI_EX& exArtINI, const char* pArtSection, std::vector<std::vector<CoordStruct>>& nFLH, std::vector<std::vector<CoordStruct>>& nEFlh, const char* pPrefixTag);

	};

	class ExtContainer final : public Container<TechnoTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static void ApplyTurretOffset(TechnoTypeClass* pType, Matrix3D* mtx, double factor = 1.0);
	static TechnoTypeClass* GetTechnoType(ObjectTypeClass* pType);

	// Ares 0.A
	static const char* GetSelectionGroupID(ObjectTypeClass* pType);
	static bool HasSelectionGroupID(ObjectTypeClass* pType, const char* pID);
};
