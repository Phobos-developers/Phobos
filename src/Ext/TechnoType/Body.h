#pragma once
#include <Ext/ObjectType/Body.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <New/Type/AttachEffectTypeClass.h>
#include <New/Type/ShieldTypeClass.h>
#include <New/Type/LaserTrailTypeClass.h>
#include <New/Type/DigitalDisplayTypeClass.h>
#include <New/Type/SelectBoxTypeClass.h>
#include <New/Type/Affiliated/InterceptorTypeClass.h>
#include <New/Type/Affiliated/PassengerDeletionTypeClass.h>
#include <New/Type/Affiliated/DroppodTypeClass.h>
#include <New/Type/Affiliated/TiberiumEaterTypeClass.h>
#include <New/Type/Affiliated/CreateUnitTypeClass.h>

class Matrix3D;
class ParticleSystemTypeClass;
class TechnoTypeExt : public ObjectTypeExt
{
public:
	using base_type = TechnoTypeClass;

	// deprecated: the pre-rework nested data class is now the extension class itself
	using ExtData [[deprecated("use the extension class itself instead")]] = TechnoTypeExt;

	static constexpr DWORD Canary = 0x11111111;

public:
	// typed owner accessor
	TechnoTypeClass* OwnerObject() const
	{
		return static_cast<TechnoTypeClass*>(this->GetAttachedObject());
	}

	Valueable<bool> HealthBar_Hide;
	Valueable<bool> HealthBar_HidePips;
	Valueable<bool> HealthBar_Permanent;
	Valueable<bool> HealthBar_Permanent_PipScale;
	Valueable<CSFText> UIDescription;
	Valueable<bool> LowSelectionPriority;
	Valueable<bool> LowDeployPriority;
	std::vector<PhobosFixedString<0x20>> WeaponGroupAs;
	Nullable<AffectedHouse> RadarJamHouses;
	Nullable<int> RadarJamDelay;
	ValueableVector<BuildingTypeClass*> RadarJamAffect;
	ValueableVector<BuildingTypeClass*> RadarJamIgnore;
	Valueable<float> FactoryPlant_Multiplier;
	Valueable<Leptons> MindControlRangeLimit;
	Nullable<bool> MindControl_IgnoreSize;
	Valueable<int> MindControlSize;
	Nullable<AffectedHouse> MindControlLink_VisibleToHouse;

	std::unique_ptr<InterceptorTypeClass> InterceptorType;

	Valueable<PartialVector3D<int>> TurretOffset;
	Nullable<bool> TurretShadow;
	Valueable<int> ShadowIndex_Frame;
	std::map<int, int> ShadowIndices;
	Valueable<bool> Spawner_LimitRange;
	Valueable<int> Spawner_ExtraLimitRange;
	int SpawnerRange;
	int EliteSpawnerRange;
	Nullable<int> Spawner_DelayFrames;
	Nullable<bool> Spawner_AttackImmediately;
	Nullable<bool> Spawner_UseTurretFacing;
	Nullable<bool> Harvester_Counted;
	Nullable<bool> Promote_IncludeSpawns;
	Valueable<bool> ImmuneToCrit;
	Nullable<bool> MultiMindControl_ReleaseVictim;
	Valueable<int> CameoPriority;
	PhobosPCXFile AltCameoPCX;
	Valueable<bool> NoManualMove;
	Nullable<int> InitialStrength;
	Nullable<bool> ReloadInTransport;
	Valueable<bool> ForbidParallelAIQueues;
	Valueable<bool> IgnoreForBaseCenter;

	int TintColorAirstrike;
	Nullable<int> LaserTargetColor;
	Nullable<ColorStruct> AirstrikeLineColor;

	Valueable<ShieldTypeClass*> ShieldType;
	std::unique_ptr<PassengerDeletionTypeClass> PassengerDeletionType;
	std::unique_ptr<DroppodTypeClass> DroppodType;
	std::unique_ptr<TiberiumEaterTypeClass> TiberiumEaterType;

	Nullable<AutoDeathBehavior> AutoDeath_Behavior;
	Nullable<bool> AutoDeath_AllowLimboed;
	ValueableVector<AnimTypeClass*> AutoDeath_VanishAnimation;
	Valueable<bool> AutoDeath_OnAmmoDepletion;
	Valueable<bool> AutoDeath_OnOwnerChange;
	Nullable<bool> AutoDeath_OnOwnerChange_IgnoreRevertOnExit;
	Nullable<bool> AutoDeath_OnOwnerChange_HumanToComputer;
	Nullable<bool> AutoDeath_OnOwnerChange_ComputerToHuman;
	Valueable<int> AutoDeath_AfterDelay;
	ValueableVector<TechnoTypeClass*> AutoDeath_TechnosDontExist;
	Valueable<bool> AutoDeath_TechnosDontExist_Any;
	Nullable<bool> AutoDeath_TechnosDontExist_AllowLimboed;
	Valueable<AffectedHouse> AutoDeath_TechnosDontExist_Houses;
	ValueableVector<TechnoTypeClass*> AutoDeath_TechnosExist;
	Valueable<bool> AutoDeath_TechnosExist_Any;
	Nullable<bool> AutoDeath_TechnosExist_AllowLimboed;
	Valueable<AffectedHouse> AutoDeath_TechnosExist_Houses;
	Valueable<PowerStatus> AutoDeath_PlayerPowerState;
	Valueable<int> AutoDeath_PlayerMoney_Max;
	Valueable<int> AutoDeath_PlayerMoney_Min;

	NullableIdx<VocClass> SellSound;
	NullableIdx<VoxClass> EVA_Sold;

	Nullable<bool> CombatAlert;
	Nullable<bool> CombatAlert_NotBuilding;
	Nullable<bool> CombatAlert_UseFeedbackVoice;
	Nullable<bool> CombatAlert_UseAttackVoice;
	Nullable<bool> CombatAlert_UseEVA;
	NullableIdx<VoxClass> CombatAlert_EVA;

	NullableIdx<VocClass> VoiceCreated;

	ValueableVector<AnimTypeClass*> WarpOut;
	ValueableVector<AnimTypeClass*> WarpIn;
	ValueableVector<AnimTypeClass*> Chronoshift_WarpOut;
	ValueableVector<AnimTypeClass*> Chronoshift_WarpIn;
	ValueableVector<AnimTypeClass*> WarpAway;
	Nullable<bool> ChronoTrigger;
	Nullable<int> ChronoDistanceFactor;
	Nullable<int> ChronoMinimumDelay;
	Nullable<int> ChronoRangeMinimum;
	Nullable<int> ChronoDelay;
	Nullable<int> ChronoSpherePreDelay;
	Nullable<int> ChronoSphereDelay;

	Valueable<WeaponTypeClass*> WarpInWeapon;
	Nullable<WeaponTypeClass*> WarpInMinRangeWeapon;
	Valueable<WeaponTypeClass*> WarpOutWeapon;
	Valueable<bool> WarpInWeapon_UseDistanceAsDamage;

	std::vector<std::vector<CoordStruct>> WeaponBurstFLHs;
	std::vector<std::vector<CoordStruct>> EliteWeaponBurstFLHs;
	std::vector<CoordStruct> AlternateFLHs;
	Nullable<bool> AlternateFLH_OnTurret;
	Nullable<bool> AlternateFLH_ApplyVehicle;

	Nullable<bool> DestroyAnim_Random;

	Nullable<bool> UseDisguiseMovementSpeed;

	Nullable<int> OpenTopped_RangeBonus;
	Nullable<float> OpenTopped_DamageMultiplier;
	Nullable<int> OpenTopped_WarpDistance;
	Nullable<bool> OpenTopped_IgnoreRangefinding;
	Nullable<bool> OpenTopped_AllowFiringIfDeactivated;
	Nullable<bool> OpenTopped_AllowFiringIfAttackedByLocomotor;
	Nullable<bool> OpenTopped_ShareTransportTarget;
	Nullable<bool> OpenTopped_UseTransportRangeModifiers;
	Nullable<bool> OpenTopped_CheckTransportDisableWeapons;
	Nullable<bool> OpenTopped_DecloakToFire;
	Nullable<bool> OpenTopped_FireWhileMoving;
	Nullable<int> OpenTransport_RangeBonus;
	Nullable<float> OpenTransport_DamageMultiplier;
	Nullable<bool> OpenTransport_FireWhileMoving;

	Valueable<bool> AutoTargetOwnPosition;
	Valueable<bool> AutoTargetOwnPosition_Self;

	Valueable<bool> NoSecondaryWeaponFallback;
	Valueable<bool> NoSecondaryWeaponFallback_AllowAA;
	Nullable<bool> AllowWeaponSelectAgainstWalls;

	Nullable<bool> JumpjetRotateOnCrash;
	Nullable<int> ShadowSizeCharacteristicHeight;

	Valueable<CSFText> EnemyUIName;

	bool ForceWeapon_Check;
	Valueable<int> ForceWeapon_Naval_Decloaked;
	Valueable<int> ForceWeapon_Cloaked;
	Valueable<int> ForceWeapon_Disguised;
	Valueable<int> ForceWeapon_UnderEMP;
	Nullable<bool> ForceWeapon_InRange_TechnoOnly;
	ValueableVector<int> ForceWeapon_InRange;
	ValueableVector<double> ForceWeapon_InRange_Overrides;
	Nullable<bool> ForceWeapon_InRange_ApplyRangeModifiers;
	ValueableVector<int> ForceAAWeapon_InRange;
	ValueableVector<double> ForceAAWeapon_InRange_Overrides;
	Nullable<bool> ForceAAWeapon_InRange_ApplyRangeModifiers;
	Valueable<int> ForceWeapon_Buildings;
	Valueable<int> ForceWeapon_Defenses;
	Valueable<int> ForceWeapon_Infantry;
	Valueable<int> ForceWeapon_Naval_Units;
	Valueable<int> ForceWeapon_Units;
	Valueable<int> ForceWeapon_Aircraft;
	Valueable<int> ForceAAWeapon_Infantry;
	Valueable<int> ForceAAWeapon_Units;
	Valueable<int> ForceAAWeapon_Aircraft;

	Valueable<bool> Ammo_Shared;
	Valueable<int> Ammo_Shared_Group;

	Nullable<SelfHealGainType> SelfHealGainType;
	Nullable<bool> Passengers_SyncOwner;
	Nullable<bool> Passengers_SyncOwner_RevertOnExit;

	Nullable<bool> IronCurtain_KeptOnDeploy;
	Nullable<IronCurtainEffect> IronCurtain_Effect;
	Nullable<WarheadTypeClass*> IronCurtain_KillWarhead;
	Nullable<bool> ForceShield_KeptOnDeploy;
	Nullable<IronCurtainEffect> ForceShield_Effect;
	Nullable<WarheadTypeClass*> ForceShield_KillWarhead;
	Nullable<bool> Explodes_KillPassengers;
	Nullable<bool> DriverKilled_KeptPassengers;
	Nullable<bool> DriverKilled_KillPassengers;
	Nullable<int> DeployFireWeapon;
	Valueable<TargetZoneScanType> TargetZoneScanType;

	Nullable<Leptons> AreaGuardRange;
	Valueable<Leptons> MaxGuardRange;

	Promotable<SHPStruct*> Insignia;
	Valueable<Vector3D<int>> InsigniaFrames;
	Promotable<int> InsigniaFrame;
	Nullable<bool> Insignia_ShowEnemy;
	std::vector<Promotable<SHPStruct*>> Insignia_Weapon;
	std::vector<Promotable<int>> InsigniaFrame_Weapon;
	std::vector<Valueable<Vector3D<int>>> InsigniaFrames_Weapon;
	std::vector<Promotable<SHPStruct*>> Insignia_Passengers;
	std::vector<Promotable<int>> InsigniaFrame_Passengers;
	std::vector<Valueable<Vector3D<int>>> InsigniaFrames_Passengers;

	Valueable<bool> DigitalDisplay_Disable;
	ValueableVector<DigitalDisplayTypeClass*> DigitalDisplayTypes;

	Nullable<SelectBoxTypeClass*> SelectBox;
	Valueable<bool> HideSelectBox;

	Valueable<int> AmmoPipFrame;
	Valueable<int> EmptyAmmoPipFrame;
	Valueable<int> AmmoPipWrapStartFrame;
	Nullable<Point2D> AmmoPipSize;
	Valueable<Point2D> AmmoPipOffset;

	Valueable<bool> ShowSpawnsPips;
	Valueable<int> SpawnsPipFrame;
	Valueable<int> EmptySpawnsPipFrame;
	Nullable<Point2D> SpawnsPipSize;
	Valueable<Point2D> SpawnsPipOffset;

	Valueable<TechnoTypeClass*> Convert_Undeploy;
	Valueable<TechnoTypeClass*> Convert_HumanToComputer;
	Valueable<TechnoTypeClass*> Convert_ComputerToHuman;
	Nullable<bool> Convert_ResetMindControl;

	Nullable<ColorStruct> Tint_Color;
	Valueable<double> Tint_Intensity;
	Valueable<AffectedHouse> Tint_VisibleToHouses;

	Valueable<WeaponTypeClass*> RevengeWeapon;
	Valueable<AffectedHouse> RevengeWeapon_AffectsHouse;

	AEAttachInfoTypeClass AttachEffects;

	Nullable<bool> RecountBurst;

	ValueableVector<TechnoTypeClass*> BuildLimitGroup_Types;
	ValueableVector<int> BuildLimitGroup_Nums;
	Valueable<int> BuildLimitGroup_Factor;
	Nullable<bool> BuildLimitGroup_ContentIfAnyMatch;
	Nullable<bool> BuildLimitGroup_NotBuildableIfQueueMatch;
	ValueableVector<TechnoTypeClass*> BuildLimitGroup_ExtraLimit_Types;
	ValueableVector<int> BuildLimitGroup_ExtraLimit_Nums;
	ValueableVector<int> BuildLimitGroup_ExtraLimit_MaxCount;
	Valueable<int> BuildLimitGroup_ExtraLimit_MaxNum;

	Nullable<bool> AmphibiousEnter;
	Nullable<bool> AmphibiousUnload;
	Nullable<bool> NoQueueUpToEnter;
	Nullable<int> NoQueueUpToEnter_BoardDistance;
	Nullable<bool> NoQueueUpToUnload;

	Valueable<int> RateDown_Delay;
	Valueable<bool> RateDown_Reset;
	Valueable<int> RateDown_Cover_Value;
	Valueable<int> RateDown_Cover_AmmoBelow;

	Nullable<bool> NoRearm_UnderEMP;
	Nullable<bool> NoRearm_Temporal;
	Nullable<bool> NoReload_UnderEMP;
	Nullable<bool> NoReload_Temporal;

	std::bitset<AdditionalAbilityCount> AdditionalVeteranAbilities;
	std::bitset<AdditionalAbilityCount> AdditionalEliteAbilities;
	Nullable<double> VeteranReload;
	Nullable<double> VeteranEmptyReload;
	Nullable<double> VeteranRange;
	Nullable<double> VeteranCritChance;

	Nullable<AnimTypeClass*> Wake;
	Nullable<AnimTypeClass*> Wake_Grapple;
	Nullable<AnimTypeClass*> Wake_Sinking;
	Nullable<bool> MakesWake;

	Valueable<float> CrashSpin_Multiplier;

	Nullable<int> AINormalTargetingDelay;
	Nullable<int> PlayerNormalTargetingDelay;
	Nullable<int> AIGuardAreaTargetingDelay;
	Nullable<int> PlayerGuardAreaTargetingDelay;
	Nullable<int> AIAttackMoveTargetingDelay;
	Nullable<int> PlayerAttackMoveTargetingDelay;
	Nullable<bool> DistributeTargetingFrame;

	Nullable<bool> AttackMove_Aggressive;
	Nullable<bool> AttackMove_UpdateTarget;

	Nullable<bool> ApproachTarget_StopWhenInRange;
	Valueable<bool> ApproachTarget_PursuitTarget;

	Valueable<bool> BunkerableAnyway;
	Valueable<bool> KeepTargetOnMove;
	Valueable<int> KeepTargetOnMove_Weapon;
	Valueable<bool> KeepTargetOnMove_NoMorePursuit;
	Valueable<Leptons> KeepTargetOnMove_ExtraDistance;

	Valueable<int> Power;

	Nullable<bool> AllowAirstrike;

	Nullable<TechnoTypeClass*> Image_ConditionYellow;
	Nullable<TechnoTypeClass*> Image_ConditionRed;
	bool NeedDamagedImage;

	Nullable<int> InitialSpawnsNumber;
	ValueableVector<AircraftTypeClass*> Spawns_Queue;

	Nullable<Leptons> Spawner_RecycleRange;
	ValueableVector<AnimTypeClass*> Spawner_RecycleAnim;
	Valueable<CoordStruct> Spawner_RecycleCoord;
	Nullable<bool> Spawner_RecycleOnTurret;

	ValueableVector<AnimTypeClass*> Promote_VeteranAnimation;
	ValueableVector<AnimTypeClass*> Promote_EliteAnimation;

	Nullable<AffectedHouse> RadarInvisibleToHouse;

	struct LaserTrailDataEntry
	{
		ValueableIdx<LaserTrailTypeClass> idxType;
		Valueable<CoordStruct> FLH;
		Valueable<bool> IsOnTurret;
		LaserTrailTypeClass* GetType() const { return LaserTrailTypeClass::Array[idxType].get(); }
	};

	std::vector<LaserTrailDataEntry> LaserTrailData;

	Valueable<bool> SuppressKillWeapons;
	ValueableVector<WeaponTypeClass*> SuppressKillWeapons_Types;

	Nullable<bool> DigitalDisplay_Health_FakeAtDisguise;

	NullableVector<int> Overload_Count;
	NullableVector<int> Overload_Damage;
	NullableVector<int> Overload_Frames;
	NullableIdx<VocClass> Overload_DeathSound;
	Nullable<ParticleSystemTypeClass*> Overload_ParticleSys;
	Nullable<int> Overload_ParticleSysCount;

	Nullable<double> FallingDownDamage;
	Nullable<double> FallingDownDamage_Water;
	Nullable<bool> FallingDownDamage_AllowEMP;

	Valueable<int> Ammo_AutoConvertMinimumAmount;
	Valueable<int> Ammo_AutoConvertMaximumAmount;
	Nullable<TechnoTypeClass*> Ammo_AutoConvertType;

	//Nullable<int> SecondaryFire;

	Nullable<bool> DebrisTypes_Limit;
	ValueableVector<int> DebrisMinimums;

	Valueable<int> EngineerRepairAmount;

	Valueable<bool> AttackMove_Follow;
	Valueable<bool> AttackMove_Follow_IncludeAir;
	Valueable<bool> AttackMove_Follow_IfMindControlIsFull;

	Valueable<bool> MultiWeapon;
	ValueableVector<bool> MultiWeapon_IsSecondary;
	Valueable<int> MultiWeapon_SelectCount;
	bool ReadMultiWeapon;
	Vector2D<ThreatType> ThreatTypes;
	Vector2D<int> CombatDamages;

	ValueableIdx<VocClass> VoiceIFVRepair;
	ValueableVector<int> VoiceWeaponAttacks;
	ValueableVector<int> VoiceEliteWeaponAttacks;

	ValueableVector<TechnoTypeClass*> TeamMember_ConsideredAs;

	Vector2D<bool> AttackFriendlies;

	Nullable<int> DrainMoneyFrameDelay;
	Nullable<int> DrainMoneyAmount;
	Nullable<AnimTypeClass*> DrainAnimationType;
	Nullable<bool> DrainMoneyDisplay;
	Nullable<AffectedHouse> DrainMoneyDisplay_Houses;
	Valueable<Point2D> DrainMoneyDisplay_Offset;
	Nullable<bool> DrainMoneyDisplay_OnTarget;
	Nullable<bool> DrainMoneyDisplay_OnTarget_UseDisplayIncome;

	Nullable<Mission> ParadropMission;
	Nullable<Mission> AIParadropMission;
	Nullable<int> ParadropDelay;
	Nullable<int> ParadropEndDelay;

	Nullable<int> PenetratesTransport_Level;
	Valueable<double> PenetratesTransport_PassThroughMultiplier;
	Valueable<double> PenetratesTransport_FatalRateMultiplier;
	Valueable<double> PenetratesTransport_DamageMultiplier;

	Nullable<bool> JumpjetClimbIgnoreBuilding;

	bool ExtraThreat_Enabled;
	Nullable<double> ExtraThreat_IsThreat;
	Valueable<bool> AlwaysConsideredThreat;
	Nullable<double> ExtraThreat_InRange;
	Nullable<double> ExtraThreatCoefficient_InRangeDistance;
	Nullable<double> ExtraThreatCoefficient_Facing;
	Nullable<double> ExtraThreatCoefficient_DistanceToLastTarget;

	Nullable<Powerup> DropCrate;

	Valueable<double> Convert_Health_AbovePercent;
	Valueable<double> Convert_Health_BelowPercent;
	Nullable<TechnoTypeClass*> Convert_Health;

	Nullable<bool> ExitThroughRoof;
	Valueable<bool> PsychicDetectable;

	ValueableVector<AnimTypeClass*> CloakAnims;
	ValueableVector<AnimTypeClass*> DecloakAnims;
	Nullable<bool> Cloak_KickOutParasite;

	// Ares 0.2
	Valueable<int> RadarJamRadius;

	// Ares 0.9
	Nullable<int> InhibitorRange;
	Nullable<int> DesignatorRange;

	// Ares 0.A
	PhobosFixedString<0x20> GroupAs;

	// Ares 0.C
	Valueable<int> NoAmmoWeapon;
	Valueable<int> NoAmmoAmount;

	// Ares 2.0
	Valueable<bool> Passengers_BySize;
	Valueable<TechnoTypeClass*> Convert_Deploy;

	// Ares 3.0
	Nullable<bool> Unsellable;
	Nullable<bool> KeepAlive;

	TechnoTypeExt(TechnoTypeClass* OwnerObject) : ObjectTypeExt(OwnerObject)
		, HealthBar_Hide { false }
		, HealthBar_HidePips { false }
		, HealthBar_Permanent { false }
		, HealthBar_Permanent_PipScale { false }
		, UIDescription {}
		, LowSelectionPriority { false }
		, LowDeployPriority { false }
		, WeaponGroupAs {}
		, RadarJamHouses {}
		, RadarJamDelay {}
		, RadarJamAffect {}
		, RadarJamIgnore {}
		, FactoryPlant_Multiplier { 1.0f }
		, MindControlRangeLimit {}
		, MindControl_IgnoreSize {}
		, MindControlSize { 1 }
		, MindControlLink_VisibleToHouse{}

		, InterceptorType { nullptr }

		, TurretOffset { { 0, 0, 0 } }
		, TurretShadow { }
		, ShadowIndices { }
		, ShadowIndex_Frame { 0 }
		, Spawner_LimitRange { false }
		, Spawner_ExtraLimitRange { 0 }
		, SpawnerRange { 0 }
		, EliteSpawnerRange { 0 }
		, Spawner_DelayFrames {}
		, Spawner_AttackImmediately {}
		, Spawner_UseTurretFacing {}
		, Harvester_Counted {}
		, Promote_IncludeSpawns {}
		, ImmuneToCrit { false }
		, MultiMindControl_ReleaseVictim {}
		, CameoPriority { 0 }
		, AltCameoPCX {}
		, NoManualMove { false }
		, InitialStrength {}
		, ReloadInTransport {}
		, ForbidParallelAIQueues { false }
		, IgnoreForBaseCenter { false }
		, TintColorAirstrike { 0 }
		, LaserTargetColor {}
		, AirstrikeLineColor {}
		, ShieldType { nullptr }
		, PassengerDeletionType { nullptr }

		, WarpOut {}
		, WarpIn {}
		, Chronoshift_WarpOut {}
		, Chronoshift_WarpIn {}
		, WarpAway {}
		, ChronoTrigger {}
		, ChronoDistanceFactor {}
		, ChronoMinimumDelay {}
		, ChronoRangeMinimum {}
		, ChronoDelay {}
		, ChronoSpherePreDelay {}
		, ChronoSphereDelay {}
		, WarpInWeapon {}
		, WarpInMinRangeWeapon {}
		, WarpOutWeapon {}
		, WarpInWeapon_UseDistanceAsDamage { false }

		, LaserTrailData {}
		, AlternateFLH_OnTurret {}
		, AlternateFLH_ApplyVehicle {}
		, DestroyAnim_Random {}

		, UseDisguiseMovementSpeed {}

		, OpenTopped_RangeBonus {}
		, OpenTopped_DamageMultiplier {}
		, OpenTopped_WarpDistance {}
		, OpenTopped_IgnoreRangefinding {}
		, OpenTopped_AllowFiringIfAttackedByLocomotor {}
		, OpenTopped_AllowFiringIfDeactivated {}
		, OpenTopped_ShareTransportTarget {}
		, OpenTopped_UseTransportRangeModifiers {}
		, OpenTopped_CheckTransportDisableWeapons {}
		, OpenTopped_DecloakToFire {}
		, OpenTopped_FireWhileMoving {}
		, OpenTransport_RangeBonus {}
		, OpenTransport_DamageMultiplier {}
		, OpenTransport_FireWhileMoving {}

		, AutoTargetOwnPosition { false }
		, AutoTargetOwnPosition_Self { false }
		, NoSecondaryWeaponFallback { false }
		, NoSecondaryWeaponFallback_AllowAA { false }
		, AllowWeaponSelectAgainstWalls {}
		, JumpjetRotateOnCrash {}
		, ShadowSizeCharacteristicHeight { }

		, AutoDeath_Behavior { }
		, AutoDeath_AllowLimboed {}
		, AutoDeath_VanishAnimation {}
		, AutoDeath_OnAmmoDepletion { false }
		, AutoDeath_OnOwnerChange { false }
		, AutoDeath_OnOwnerChange_IgnoreRevertOnExit {}
		, AutoDeath_OnOwnerChange_HumanToComputer {}
		, AutoDeath_OnOwnerChange_ComputerToHuman {}
		, AutoDeath_AfterDelay { 0 }
		, AutoDeath_TechnosDontExist {}
		, AutoDeath_TechnosDontExist_Any { false }
		, AutoDeath_TechnosDontExist_AllowLimboed {}
		, AutoDeath_TechnosDontExist_Houses { AffectedHouse::Owner }
		, AutoDeath_TechnosExist {}
		, AutoDeath_TechnosExist_Any { true }
		, AutoDeath_TechnosExist_AllowLimboed {}
		, AutoDeath_TechnosExist_Houses { AffectedHouse::Owner }
		, AutoDeath_PlayerPowerState { PowerStatus::None }
		, AutoDeath_PlayerMoney_Max { -1 }
		, AutoDeath_PlayerMoney_Min { -1 }

		, SellSound {}
		, EVA_Sold {}

		, CombatAlert {}
		, CombatAlert_NotBuilding {}
		, CombatAlert_UseFeedbackVoice {}
		, CombatAlert_UseAttackVoice {}
		, CombatAlert_UseEVA {}
		, CombatAlert_EVA {}

		, EnemyUIName {}

		, VoiceCreated {}

		, ForceWeapon_Check { false }
		, ForceWeapon_Naval_Decloaked { -1 }
		, ForceWeapon_Cloaked { -1 }
		, ForceWeapon_Disguised { -1 }
		, ForceWeapon_UnderEMP { -1 }
		, ForceWeapon_InRange_TechnoOnly {}
		, ForceWeapon_InRange {}
		, ForceWeapon_InRange_Overrides {}
		, ForceWeapon_InRange_ApplyRangeModifiers {}
		, ForceAAWeapon_InRange {}
		, ForceAAWeapon_InRange_Overrides {}
		, ForceAAWeapon_InRange_ApplyRangeModifiers {}
		, ForceWeapon_Buildings { -1 }
		, ForceWeapon_Defenses { -1 }
		, ForceWeapon_Infantry { -1 }
		, ForceWeapon_Naval_Units { -1 }
		, ForceWeapon_Units { -1 }
		, ForceWeapon_Aircraft { -1 }
		, ForceAAWeapon_Infantry { -1 }
		, ForceAAWeapon_Units { -1 }
		, ForceAAWeapon_Aircraft { -1 }

		, Ammo_Shared { false }
		, Ammo_Shared_Group { -1 }

		, SelfHealGainType {}
		, Passengers_SyncOwner {}
		, Passengers_SyncOwner_RevertOnExit {}

		, IronCurtain_KeptOnDeploy {}
		, IronCurtain_Effect {}
		, IronCurtain_KillWarhead {}
		, ForceShield_KeptOnDeploy {}
		, ForceShield_Effect {}
		, ForceShield_KillWarhead {}

		, Explodes_KillPassengers {}
		, DriverKilled_KeptPassengers {}
		, DriverKilled_KillPassengers {}
		, DeployFireWeapon {}
		, TargetZoneScanType { TargetZoneScanType::Same }

		, AreaGuardRange {}
		, MaxGuardRange { Leptons(4096) }

		, Insignia {}
		, InsigniaFrames { { -1, -1, -1 } }
		, InsigniaFrame { -1 }
		, Insignia_ShowEnemy {}
		, Insignia_Weapon {}
		, InsigniaFrame_Weapon {}
		, InsigniaFrames_Weapon {}
		, Insignia_Passengers {}
		, InsigniaFrame_Passengers {}
		, InsigniaFrames_Passengers {}

		, DigitalDisplay_Disable { false }
		, DigitalDisplayTypes {}

		, SelectBox {}
		, HideSelectBox { false }

		, AmmoPipFrame { 13 }
		, EmptyAmmoPipFrame { -1 }
		, AmmoPipWrapStartFrame { 14 }
		, AmmoPipSize {}
		, AmmoPipOffset { { 0,0 } }

		, ShowSpawnsPips { true }
		, SpawnsPipFrame { 1 }
		, EmptySpawnsPipFrame { 0 }
		, SpawnsPipSize {}
		, SpawnsPipOffset { { 0,0 } }

		, DroppodType {}
		, TiberiumEaterType {}

		, Convert_Undeploy { }
		, Convert_HumanToComputer { }
		, Convert_ComputerToHuman { }
		, Convert_ResetMindControl {}

		, Tint_Color {}
		, Tint_Intensity { 0.0 }
		, Tint_VisibleToHouses { AffectedHouse::All }

		, RevengeWeapon {}
		, RevengeWeapon_AffectsHouse { AffectedHouse::All }

		, AttachEffects {}

		, RecountBurst {}

		, BuildLimitGroup_Types {}
		, BuildLimitGroup_Nums {}
		, BuildLimitGroup_Factor { 1 }
		, BuildLimitGroup_ContentIfAnyMatch {}
		, BuildLimitGroup_NotBuildableIfQueueMatch {}
		, BuildLimitGroup_ExtraLimit_Types {}
		, BuildLimitGroup_ExtraLimit_Nums {}
		, BuildLimitGroup_ExtraLimit_MaxCount {}
		, BuildLimitGroup_ExtraLimit_MaxNum { 0 }

		, AmphibiousEnter {}
		, AmphibiousUnload {}
		, NoQueueUpToEnter {}
		, NoQueueUpToEnter_BoardDistance {}
		, NoQueueUpToUnload {}

		, RateDown_Delay { 0 }
		, RateDown_Reset { false }
		, RateDown_Cover_Value { 0 }
		, RateDown_Cover_AmmoBelow { -2 }

		, NoRearm_UnderEMP {}
		, NoRearm_Temporal {}
		, NoReload_UnderEMP {}
		, NoReload_Temporal {}

		, AdditionalVeteranAbilities {}
		, AdditionalEliteAbilities {}
		, VeteranReload {}
		, VeteranEmptyReload {}
		, VeteranRange {}
		, VeteranCritChance {}

		, Wake { }
		, Wake_Grapple { }
		, Wake_Sinking { }
		, MakesWake { }

		, CrashSpin_Multiplier { 1.0f }

		, AINormalTargetingDelay {}
		, PlayerNormalTargetingDelay {}
		, AIGuardAreaTargetingDelay {}
		, PlayerGuardAreaTargetingDelay {}
		, AIAttackMoveTargetingDelay {}
		, PlayerAttackMoveTargetingDelay {}
		, DistributeTargetingFrame {}

		, DigitalDisplay_Health_FakeAtDisguise {}

		, AttackMove_Aggressive {}
		, AttackMove_UpdateTarget {}

		, ApproachTarget_StopWhenInRange {}
		, ApproachTarget_PursuitTarget { false }

		, BunkerableAnyway { false }
		, KeepTargetOnMove { false }
		, KeepTargetOnMove_Weapon { -1 }
		, KeepTargetOnMove_NoMorePursuit { true }
		, KeepTargetOnMove_ExtraDistance { Leptons(0) }

		, Power { }

		, AllowAirstrike { }

		, Image_ConditionYellow { }
		, Image_ConditionRed { }
		, NeedDamagedImage { false }

		, InitialSpawnsNumber { }
		, Spawns_Queue { }

		, Spawner_RecycleRange {}
		, Spawner_RecycleAnim { }
		, Spawner_RecycleCoord { {0,0,0} }
		, Spawner_RecycleOnTurret {}

		, SuppressKillWeapons { false }
		, SuppressKillWeapons_Types { }

		, Promote_VeteranAnimation { }
		, Promote_EliteAnimation { }

		, RadarInvisibleToHouse {}

		, Overload_Count {}
		, Overload_Damage {}
		, Overload_Frames {}
		, Overload_DeathSound {}
		, Overload_ParticleSys {}
		, Overload_ParticleSysCount {}

		, FallingDownDamage {}
		, FallingDownDamage_Water {}
		, FallingDownDamage_AllowEMP {}

		, Ammo_AutoConvertMinimumAmount { -1 }
		, Ammo_AutoConvertMaximumAmount { -1 }
		, Ammo_AutoConvertType { nullptr }

		//, SecondaryFire {}

		, DebrisTypes_Limit {}
		, DebrisMinimums {}

		, EngineerRepairAmount { 0 }

		, AttackMove_Follow { false }
		, AttackMove_Follow_IncludeAir { false }
		, AttackMove_Follow_IfMindControlIsFull { false }

		, MultiWeapon { false }
		, MultiWeapon_IsSecondary {}
		, MultiWeapon_SelectCount { 2 }
		, ReadMultiWeapon { false }
		, ThreatTypes { ThreatType::Normal,ThreatType::Normal }
		, CombatDamages { 0,0 }

		, VoiceIFVRepair { -1 }
		, VoiceWeaponAttacks {}
		, VoiceEliteWeaponAttacks {}

		, TeamMember_ConsideredAs {}

		, AttackFriendlies { false,false }

		, DrainMoneyFrameDelay {}
		, DrainMoneyAmount {}
		, DrainAnimationType {}
		, DrainMoneyDisplay {}
		, DrainMoneyDisplay_Houses {}
		, DrainMoneyDisplay_Offset { Point2D::Empty }
		, DrainMoneyDisplay_OnTarget {}
		, DrainMoneyDisplay_OnTarget_UseDisplayIncome {}

		, ParadropMission {}
		, AIParadropMission {}

		, PenetratesTransport_Level {}
		, PenetratesTransport_PassThroughMultiplier { 1.0 }
		, PenetratesTransport_FatalRateMultiplier { 1.0 }
		, PenetratesTransport_DamageMultiplier { 1.0 }

		, JumpjetClimbIgnoreBuilding {}

		, ExtraThreat_Enabled { false }
		, ExtraThreat_IsThreat {}
		, AlwaysConsideredThreat { false }
		, ExtraThreat_InRange {}
		, ExtraThreatCoefficient_InRangeDistance {}
		, ExtraThreatCoefficient_Facing {}
		, ExtraThreatCoefficient_DistanceToLastTarget {}

		, DropCrate {}

		, Convert_Health_AbovePercent { -1.0 }
		, Convert_Health_BelowPercent { -1.0 }
		, Convert_Health {}
		
		, PsychicDetectable { true }

		, ExitThroughRoof {}

		, CloakAnims {}
		, DecloakAnims {}
		, Cloak_KickOutParasite {}

		// Ares 0.2
		, RadarJamRadius { 0 }

		// Ares 0.9
		, InhibitorRange {}
		, DesignatorRange {}
			
		// Ares 0.A
		, GroupAs { NONE_STR }
			
		// Ares 0.C
		, NoAmmoWeapon { -1 }
		, NoAmmoAmount { 0 }
			
		// Ares 2.0
		, Passengers_BySize { true }
		, Convert_Deploy { }

		// Ares 3.0
		, Unsellable {}
		, KeepAlive {}
	{ }

	virtual ~TechnoTypeExt() = default;
	virtual void LoadFromINIFile(CCINIClass* pINI) override;

	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	void ApplyTurretOffset(Matrix3D* mtx, double factor = 1.0);
	void CalculateSpawnerRange();
	bool IsSecondary(int nWeaponIndex) const;
	const std::string GetGunnerID(int idx) const;

	int SelectForceWeapon(TechnoClass* pThis, AbstractClass* pTarget) const;
	int SelectMultiWeapon(TechnoClass* const pThis, AbstractClass* const pTarget) const;

	void UpdateAdditionalAttributes();

	// Ares 0.2
	bool CameoIsVeteran(HouseClass* pHouse) const;

	// Ares 0.A
	const char* GetSelectionGroupID() const;

protected:
	// callable from the concrete leaf type exts (e.g. InfantryTypeExt) that read
	// their own art-INI burst FLHs through this shared parser
	void ParseBurstFLHs(INI_EX& exArtINI, const char* pArtSection, std::vector<std::vector<CoordStruct>>& nFLH, std::vector<std::vector<CoordStruct>>& nEFlh, const char* pPrefixTag);

private:
	template <typename T>
	void Serialize(T& Stm);

	void ParseVoiceWeaponAttacks(INI_EX& exINI, const char* pSection, ValueableVector<int>& n, ValueableVector<int>& nE);

public:
	// TechnoTypeExt is never instantiated and has no container of its own: instances are
	// concrete leaves (UnitTypeExt/InfantryTypeExt/AircraftTypeExt/BuildingTypeExt)
	// tracked by their own containers. The polymorphic fetch reads the inline slot directly.
	static TechnoTypeExt* Fetch(const TechnoTypeClass* pThis)
	{
		return AbstractExt::Fetch<TechnoTypeExt>(pThis);
	}

	static TechnoTypeExt* TryFetch(const TechnoTypeClass* pThis)
	{
		return AbstractExt::TryFetch<TechnoTypeExt>(pThis);
	}

	// deprecated stand-in for the pre-rework container of all TechnoTypeClass extensions
	static inline CompatExtMap<TechnoTypeExt, TechnoTypeClass> ExtMap {};
	static bool SelectWeaponMutex;

	static void ApplyTurretOffset(TechnoTypeClass* pType, Matrix3D* mtx, double factor = 1.0, int turIdx = -1);
	static TechnoTypeClass* GetTechnoType(ObjectTypeClass* pType);

	static TechnoClass* CreateUnit(CreateUnitTypeClass* pCreateUnit, DirType facing, DirType* secondaryFacing,
	CoordStruct location, HouseClass* pOwner, TechnoClass* pInvoker, HouseClass* pInvokerHouse);

	static WeaponTypeClass* GetWeaponType(TechnoTypeClass* pThis, int weaponIndex, bool isElite);

	// Ares 0.A
	static const char* GetSelectionGroupID(ObjectTypeClass* pType);
	static bool HasSelectionGroupID(ObjectTypeClass* pType, const char* pID);
};

