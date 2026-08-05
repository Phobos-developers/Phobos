#pragma once

#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class AnimTypeClass;
class MouseCursor;
class SuperWeaponTypeClass;
class TechnoTypeClass;
class VocClass;
class WarheadTypeClass;
class DigitalDisplayTypeClass;
class SelectBoxTypeClass;

class RulesExt
{
public:
	using base_type = RulesClass;

	static constexpr DWORD Canary = 0x12341234;

	class ExtData final : public Extension<RulesClass>
	{
	public:
		std::vector<std::vector<TechnoTypeClass*>> AITargetTypesLists;
		std::vector<std::vector<ScriptTypeClass*>> AIScriptsLists;

		Valueable<int> Storage_TiberiumIndex;
		Valueable<float> HarvesterDumpAmount;
		Nullable<int> InfantryGainSelfHealCap;
		Nullable<int> UnitsGainSelfHealCap;
		Valueable<bool> GainSelfHealAllowMultiplayPassive;
		Valueable<bool> GainSelfHealFromPlayerControl;
		Valueable<bool> GainSelfHealFromAllies;
		Valueable<bool> EnemyInsignia;
		Valueable<AffectedHouse> DisguiseBlinkingVisibility;
		Valueable<int> ChronoSparkleDisplayDelay;
		Valueable<ChronoSparkleDisplayPosition> ChronoSparkleBuildingDisplayPositions;
		Valueable<int> ChronoSpherePreDelay;
		Valueable<int> ChronoSphereDelay;
		ValueableIdx<SuperWeaponTypeClass> AIChronoSphereSW;
		ValueableIdx<SuperWeaponTypeClass> AIChronoWarpSW;
		int SubterraneanSpeed;
		Valueable<int> SubterraneanHeight;
		Nullable<int> AISuperWeaponDelay;
		Valueable<bool> UseGlobalRadApplicationDelay;
		Valueable<int> RadApplicationDelay_Building;
		Valueable<int> RadBuildingDamageMaxCount;
		Valueable<bool> RadSiteWarhead_Detonate;
		Valueable<bool> RadSiteWarhead_Detonate_Full;
		Valueable<bool> RadHasOwner;
		Valueable<bool> RadHasInvoker;
		Valueable<bool> ShieldApplyArmorMult;
		Valueable<double> JumpjetCrash;
		Valueable<bool> JumpjetNoWobbles;
		Valueable<bool> JumpjetRotateOnCrash;

		Nullable<WarheadTypeClass*> VeinholeWarhead;

		PhobosFixedString<32u> MissingCameo;

		TranslucencyLevel PlacementGrid_Translucency;
		Nullable<TranslucencyLevel> PlacementGrid_TranslucencyWithPreview;
		Valueable<bool> PlacementPreview;
		TranslucencyLevel PlacementPreview_Translucency;

		Valueable<bool> SuperWeaponTimer_Percentage;
		Valueable<bool> SuperWeaponSidebar_AllowByDefault;

		Nullable<double> ConditionYellow_Terrain;
		Nullable<double> Shield_ConditionYellow;
		Nullable<double> Shield_ConditionRed;
		Valueable<Vector3D<int>> Pips_Shield;
		Nullable<SHPStruct*> Pips_Shield_Background;
		Valueable<Vector3D<int>> Pips_Shield_Building;
		Nullable<int> Pips_Shield_Building_Empty;
		Valueable<Point2D> Pips_SelfHeal_Infantry;
		Valueable<Point2D> Pips_SelfHeal_Units;
		Valueable<Point2D> Pips_SelfHeal_Buildings;
		Valueable<Point2D> Pips_SelfHeal_Infantry_Offset;
		Valueable<Point2D> Pips_SelfHeal_Units_Offset;
		Valueable<Point2D> Pips_SelfHeal_Buildings_Offset;
		Valueable<Point2D> Pips_Generic_Size;
		Valueable<Point2D> Pips_Generic_Buildings_Size;
		Valueable<Point2D> Pips_Ammo_Size;
		Valueable<Point2D> Pips_Ammo_Buildings_Size;
		ValueableVector<int> Pips_Tiberiums_Frames;
		Valueable<int> Pips_Tiberiums_EmptyFrame;
		ValueableVector<int> Pips_Tiberiums_DisplayOrder;
		Valueable<int> Pips_Tiberiums_WeedFrame;
		Valueable<int> Pips_Tiberiums_WeedEmptyFrame;

		Valueable<bool> HeightShadowScaling;
		Valueable<double> HeightShadowScaling_MinScale;
		double AirShadowBaseScale_log;

		Valueable<bool> ExtendedAircraftMissions;
		Valueable<int> ExtendedAircraftMissions_UnlandDamage;
		Valueable<EdgeType> AircraftSpawnFromEdge;
		Valueable<EdgeType> AircraftRetreatToEdge;
		Valueable<bool> AmphibiousEnter;
		Valueable<bool> AmphibiousUnload;
		Valueable<bool> NoQueueUpToEnter;
		Valueable<int> NoQueueUpToEnter_BoardDistance;
		Valueable<bool> NoQueueUpToUnload;
		Nullable<bool> NoQueueUpToEnter_Buildings;
		Nullable<bool> NoQueueUpToUnload_Buildings;

		Valueable<bool> JumpjetTilt;
		Valueable<double> JumpjetTilt_ForwardAccelFactor;
		Valueable<double> JumpjetTilt_ForwardSpeedFactor;
		Valueable<double> JumpjetTilt_SidewaysRotationFactor;
		Valueable<double> JumpjetTilt_SidewaysSpeedFactor;

		Valueable<bool> Spawner_AttackImmediately;
		Valueable<bool> Spawner_UseTurretFacing;
		Valueable<Leptons> Spawner_RecycleRange;
		Valueable<bool> Spawner_RecycleOnTurret;
		Valueable<bool> Promote_IncludeSpawns;
		Valueable<AffectedHouse> RadarJamHouses;
		Valueable<int> RadarJamDelay;
		Valueable<bool> MindControl_IgnoreSize;
		Valueable<bool> MultiMindControl_ReleaseVictim;
		Valueable<AffectedHouse> MindControlLink_VisibleToHouse;
		Valueable<bool> AlternateFLH_OnTurret;
		Valueable<bool> AlternateFLH_ApplyVehicle;
		Valueable<bool> DestroyAnim_Random;
		Valueable<bool> UseDisguiseMovementSpeed;
		Valueable<bool> Convert_ResetMindControl;
		Valueable<bool> BuildLimitGroup_ContentIfAnyMatch;
		Valueable<bool> BuildLimitGroup_NotBuildableIfQueueMatch;
		Valueable<bool> DigitalDisplay_Health_FakeAtDisguise;
		Valueable<int> Overload_ParticleSysCount;
		Valueable<double> FallingDownDamage;
		Valueable<bool> FallingDownDamage_AllowEMP;

		Valueable<bool> ForceWeapon_InRange_TechnoOnly;
		Valueable<bool> ForceWeapon_InRange_ApplyRangeModifiers;
		Valueable<bool> ForceAAWeapon_InRange_ApplyRangeModifiers;

		Valueable<bool> BuildingProductionQueue;

		Valueable<bool> AllowParallelAIQueues;
		Valueable<bool> ForbidParallelAIQueues_Aircraft;
		Valueable<bool> ForbidParallelAIQueues_Building;
		Valueable<bool> ForbidParallelAIQueues_Infantry;
		Valueable<bool> ForbidParallelAIQueues_Navy;
		Valueable<bool> ForbidParallelAIQueues_Vehicle;

		Valueable<bool> EnablePowerSurplus;
		Valueable<int> PowerSurplus_ScaleToDrainAmount;

		Valueable<bool> DisplayIncome;
		Valueable<int> DisplayIncome_Delay;
		Valueable<bool> DisplayIncome_AllowAI;
		Valueable<AffectedHouse> DisplayIncome_Houses;

		Valueable<bool> DrainMoneyDisplay;
		Valueable<AffectedHouse> DrainMoneyDisplay_Houses;
		Valueable<bool> DrainMoneyDisplay_OnTarget;
		Valueable<bool> DrainMoneyDisplay_OnTarget_UseDisplayIncome;

		Valueable<bool> AllowDeployControlledMCV;

		Valueable<bool> TypeSelectUseIFVMode;

		Valueable<bool> IronCurtain_KeptOnDeploy;
		Valueable<IronCurtainEffect> IronCurtain_EffectOnOrganics;
		Nullable<WarheadTypeClass*> IronCurtain_KillOrganicsWarhead;
		Valueable<bool> ForceShield_KeptOnDeploy;
		Valueable<IronCurtainEffect> ForceShield_EffectOnOrganics;
		Nullable<WarheadTypeClass*> ForceShield_KillOrganicsWarhead;

		Valueable<bool> AllowWeaponSelectAgainstWalls;

		Valueable<double> IronCurtain_ExtraTintIntensity;
		Valueable<double> ForceShield_ExtraTintIntensity;
		Valueable<bool> ColorAddUse8BitRGB;
		Valueable<ColorStruct> AirstrikeLineColor;
		Valueable<int> AirstrikeLineZAdjust;

		Valueable<bool> Strafing_SimulateBurst;
		Valueable<bool> Strafing_UseAmmoPerShot;
		Valueable<bool> Strafing_TargetCell;
		Valueable<bool> OmniFire_TurnToTarget;
		Valueable<bool> AmbientDamage_IgnoreTarget;
		Valueable<bool> KeepRange_AllowAI;
		Valueable<bool> KeepRange_AllowPlayer;
		Valueable<int> KeepRange_EarlyStopFrame;
		Valueable<bool> AircraftWeapon_KickOutPassengers;
		Valueable<double> CrushSlowdownMultiplier;
		Valueable<bool> SkipCrushSlowdown;

		Valueable<bool> LaserPositionUpdate_StopOnFirerConvert;
		Valueable<int> LaserZAdjust;
		Valueable<int> EBoltZAdjust;
		Valueable<bool> EBoltZAdjust_ClampInitialDepthForBuilding;

		Valueable<PartialVector2D<int>> ROF_RandomDelay;
		Valueable<ColorStruct> ToolTip_Background_Color;
		Valueable<int> ToolTip_Background_Opacity;
		Valueable<float> ToolTip_Background_BlurSize;

		Valueable<bool> CrateOnlyOnLand;
		Valueable<int> UnitCrateVehicleCap;
		Valueable<int> FreeMCV_CreditsThreshold;
		Valueable<AffectedHouse> RadialIndicatorVisibility;
		Valueable<bool> DrawTurretShadow;
		ValueableIdx<ColorScheme> AnimRemapDefaultColorScheme;
		ValueableIdx<ColorScheme> TimerBlinkColorScheme;

		ValueableVector<DigitalDisplayTypeClass*> Buildings_DefaultDigitalDisplayTypes;
		ValueableVector<DigitalDisplayTypeClass*> Infantry_DefaultDigitalDisplayTypes;
		ValueableVector<DigitalDisplayTypeClass*> Vehicles_DefaultDigitalDisplayTypes;
		ValueableVector<DigitalDisplayTypeClass*> Aircraft_DefaultDigitalDisplayTypes;

		Valueable<SelectBoxTypeClass*> DefaultInfantrySelectBox;
		Valueable<SelectBoxTypeClass*> DefaultUnitSelectBox;

		Valueable<Leptons> VisualScatter_Min;
		Valueable<Leptons> VisualScatter_Max;

		Valueable<bool> ShowDesignatorRange;
		Valueable<bool> ShowPowerPlantEnhancerRange;
		Valueable<bool> IsVoiceCreatedGlobal;
		Valueable<bool> SetTabBySelectingFactory;
		Valueable<int> SelectionFlashDuration;
		Valueable<int> SetRecruitableOnLiberate;
		Nullable<AnimTypeClass*> DropPodTrailer;
		AnimTypeClass* DropPodDefaultTrailer;
		SHPStruct* PodImage;
		Valueable<bool> DrawInsignia_OnlyOnSelected;
		Valueable<Point2D> DrawInsignia_AdjustPos_Infantry;
		Valueable<Point2D> DrawInsignia_AdjustPos_Buildings;
		Nullable<BuildingSelectBracketPosition> DrawInsignia_AdjustPos_BuildingsAnchor;
		Valueable<Point2D> DrawInsignia_AdjustPos_Units;
		Valueable<bool> DrawInsignia_UsePixelSelectionBracketDelta;
		ValueableVector<AnimTypeClass*> Promote_VeteranAnimation;
		ValueableVector<AnimTypeClass*> Promote_EliteAnimation;

		Valueable<bool> JumpjetClimbPredictHeight;
		Valueable<bool> JumpjetClimbWithoutCutOut;
		Valueable<bool> JumpjetClimbIgnoreBuilding;

		Valueable<bool> MergeBuildingDamage;

		Valueable<double> DamageOwnerMultiplier;
		Valueable<double> DamageAlliesMultiplier;
		Valueable<double> DamageEnemiesMultiplier;
		Nullable<double> DamageOwnerMultiplier_NotAffectsEnemies;
		Nullable<double> DamageAlliesMultiplier_NotAffectsEnemies;
		Nullable<double> DamageOwnerMultiplier_Berzerk;
		Nullable<double> DamageAlliesMultiplier_Berzerk;
		Nullable<double> DamageEnemiesMultiplier_Berzerk;

		Valueable<double> AircraftLevelLightMultiplier;
		Valueable<double> JumpjetLevelLightMultiplier;

		Valueable<bool> CombatAlert;
		Nullable<bool> CombatAlert_Default;
		Valueable<bool> CombatAlert_IgnoreBuilding;
		Valueable<bool> CombatAlert_SuppressIfInScreen;
		Valueable<int> CombatAlert_Interval;
		Valueable<bool> CombatAlert_SuppressIfAllyDamage;
		Valueable<bool> CombatAlert_MakeAVoice;
		Valueable<bool> CombatAlert_UseFeedbackVoice;
		Valueable<bool> CombatAlert_UseAttackVoice;
		Valueable<bool> CombatAlert_UseEVA;

		Nullable<Vector3D<float>> VoxelLightSource;
		// Nullable<Vector3D<float>> VoxelShadowLightSource;
		Valueable<bool> UseFixedVoxelLighting;

		Valueable<bool> AIAutoDeployMCV;
		Valueable<bool> AISetBaseCenter;
		Valueable<bool> AIBiasSpawnCell;
		Valueable<bool> AIForbidConYard;
		Valueable<bool> AINodeWallsOnly;
		Valueable<bool> AICleanWallNode;

		Valueable<bool> AttackMove_Aggressive;
		Valueable<bool> AttackMove_UpdateTarget;

		Valueable<int> MindControl_ThreatDelay;

		Valueable<bool> RecountBurst;
		Valueable<bool> NoRearm_UnderEMP;
		Valueable<bool> NoRearm_Temporal;
		Valueable<bool> NoReload_UnderEMP;
		Valueable<bool> NoReload_Temporal;
		Valueable<bool> NoTurret_TrackTarget;

		Valueable<bool> GatherWhenMCVDeploy;
		Valueable<bool> AIFireSale;
		Valueable<int> AIFireSaleDelay;
		Valueable<bool> AIAllToHunt;
		Valueable<bool> RepairBaseNodes;

		Valueable<bool> FixRepairStepCost;

		Valueable<bool> WarheadParticleAlphaImageIsLightFlash;
		Valueable<int> CombatLightDetailLevel;
		Valueable<bool> CombatLightDetailLevel_CheckColored;
		Valueable<int> LightFlashAlphaImageDetailLevel;

		Valueable<bool> UseRetintFix;

		Nullable<int> AINormalTargetingDelay;
		Nullable<int> PlayerNormalTargetingDelay;
		Nullable<int> AIGuardAreaTargetingDelay;
		Nullable<int> PlayerGuardAreaTargetingDelay;
		Nullable<int> AIAttackMoveTargetingDelay;
		Nullable<int> PlayerAttackMoveTargetingDelay;
		Valueable<bool> DistributeTargetingFrame;
		Valueable<bool> DistributeTargetingFrame_AIOnly;

		Valueable<bool> CanTargetAI_IronCurtained;
		Valueable<bool> CanTarget_IronCurtained;
		Valueable<bool> AutoTarget_IronCurtained;

		Valueable<bool> BuildingWaypoints;
		Valueable<bool> BuildingTypeSelectable;

		Valueable<double> ProneSpeed_Crawls;
		Valueable<double> ProneSpeed_NoCrawls;

		Valueable<double> DamagedSpeed;

		Valueable<bool> HarvesterScanAfterUnload;

		Valueable<bool> AnimCraterDestroyTiberium;

		Valueable<AffectedHouse> BerzerkTargeting;
		Valueable<bool> AllowBerzerkOnAllies;

		Valueable<bool> AttackMove_IgnoreWeaponCheck;

		NullableIdx<AnimTypeClass> Parasite_GrappleAnim;
		Nullable<bool> Parasite_AllowWaterExit;

		// cache tint color
		int TintColorIronCurtain;
		int TintColorForceShield;
		int TintColorBerserk;

		Valueable<bool> InfantryAutoDeploy;

		Valueable<int> AdjacentWallDamage;

		Valueable<int> WarheadAnimZAdjust;

		Valueable<bool> IvanBombAttachToCenter;

		Valueable<bool> FallingDownTargetingFix;
		Valueable<bool> AIAirTargetingFix;

		Valueable<bool> ReloadInTransport;
		Valueable<bool> OpenTopped_IgnoreRangefinding;
		Valueable<bool> OpenTopped_AllowFiringIfDeactivated;
		Valueable<bool> OpenTopped_AllowFiringIfAttackedByLocomotor;
		Valueable<bool> OpenTopped_ShareTransportTarget;
		Valueable<bool> OpenTopped_UseTransportRangeModifiers;
		Valueable<bool> OpenTopped_CheckTransportDisableWeapons;
		Valueable<bool> OpenTopped_DecloakToFire;
		Valueable<bool> OpenTopped_FireWhileMoving;
		Valueable<int> OpenTransport_RangeBonus;
		Valueable<float> OpenTransport_DamageMultiplier;
		Valueable<bool> OpenTransport_FireWhileMoving;

		Valueable<bool> Passengers_SyncOwner;
		Valueable<bool> Passengers_SyncOwner_RevertOnExit;

		Valueable<bool> Explodes_KillPassengers;
		Valueable<bool> Explodes_DuringBuildup;

		Valueable<bool> AircraftFiringForceScatter;

		Valueable<bool> HoverDrownable;

		Valueable<bool> Arcing_AllowElevationInaccuracy;

		Valueable<bool> Terrain_IsPassable;
		Valueable<bool> Tibtree_IsPassable;
		Valueable<bool> Terrain_CanBeBuiltOn;
		Valueable<bool> Tibtree_CanBeBuiltOn;

		Nullable<bool> Sinkable;
		Valueable<bool> Sinkable_SquidGrab;
		Valueable<int> SinkSpeed;

		Valueable<bool> CreateAnimsOnZeroDamage;
		Valueable<bool> Conventional_IgnoreUnits;
		Valueable<bool> DecloakDamagedTargets;
		Valueable<bool> ShakeIsLocal;
		Valueable<bool> ApplyModifiersOnNegativeDamage;
		Valueable<bool> AllowDamageOnSelf;
		Valueable<bool> Debris_Conventional;
		Valueable<bool> Parasite_DisableParticleSystem;

		Valueable<bool> ProjectileInterceptable;
		Valueable<bool> Interceptor_GuardRange_IsCylindrical;
		Valueable<bool> Interceptor_ApplyFirepowerMult;

		Valueable<bool> SortCameoByName;

		Valueable<bool> BuildingRadioLink_SyncOwner;

		Valueable<Leptons> ExtraRange_TargetMoving;
		Valueable<bool> ExtraRange_TargetMoving_CloseRangeOnly;
		Valueable<Leptons> ExtraRange_FirerMoving;
		Valueable<Leptons> ExtraRange_Prefiring;
		Valueable<bool> ExtraRange_Prefiring_IncludeBurst;

		Valueable<bool> ApplyPerTargetEffectsOnDetonate;
		Valueable<bool> AffectsInvokerOnly_IgnoreInvokerState;

		Valueable<bool> FiringAnim_Update;

		Valueable<bool> ExtendedPlayerRepair;

		Valueable<bool> AutoTarget_NoThreatBuildings;
		Valueable<bool> AutoTargetAI_NoThreatBuildings;

		Valueable<Mission> ParadropMission;
		Valueable<Mission> AIParadropMission;
		Valueable<int> ParadropDelay;
		Valueable<int> ParadropEndDelay;

		Valueable<bool> DefaultToGuardArea;
		Valueable<int> LeptonMindControlOffset;
		Valueable<int> MindControlRingOffset;

		Valueable<bool> DisableOveroptimizationInTargeting;

		Valueable<bool> CylinderRangefinding;

		Valueable<int> PenetratesTransport_Level;

		Valueable<bool> UnitsUnsellable;

		Valueable<bool> DriverKilled_KeptPassengers;
		Valueable<bool> DriverKilled_KillPassengers;
		Valueable<double> ExtraThreat_IsThreat;
		Valueable<double> ExtraThreat_InRange;
		Valueable<double> ExtraThreatCoefficient_InRangeDistance;
		Valueable<double> ExtraThreatCoefficient_Facing;
		Valueable<double> ExtraThreatCoefficient_DistanceToLastTarget;
		Valueable<bool> BalloonHoverPathingFix;

		Valueable<bool> WalkLocomotorMakesWake;
		Valueable<bool> DriveLocomotorMakesWake;
		Valueable<bool> HoverLocomotorMakesWake;
		Valueable<bool> ShipLocomotorMakesWake;

		Valueable<StackingMode> Psychedelic_StackingMode;

		Valueable<bool> Shrapnel_AffectsGround;
		Valueable<bool> Shrapnel_AffectsBuildings;
		Valueable<bool> Shrapnel_UseWeaponTargeting;
		Valueable<bool> Shrapnel_IgnoreHitBuildings;
		Valueable<bool> Shrapnel_ObeyWarheadTriggerConditions;

		Valueable<bool> ReturnWeapon_ApplyFirepowerMult;

		Valueable<bool> Splits_TargetingDistance_Cylindrical;
		Valueable<bool> Splits_AllowRepeatTargets;
		Valueable<bool> Splits_UseWeaponTargeting;
		Valueable<bool> Airburst_UseCluster;
		Valueable<bool> Airburst_TargetAsSource_SkipHeight;
		Valueable<bool> AirburstWeapon_ApplyFirepowerMult;
		Valueable<bool> AirburstWeapon_UseFiringEffects;
		Valueable<bool> AirburstWeapon_HeadToTarget;

		Valueable<bool> AnimDamage_DealtByInvoker;
		Valueable<bool> AnimDamage_ApplyFirepowerMult;

		Valueable<bool> Crit_ApplyChancePerTarget;
		Valueable<bool> Crit_ExtraDamage_ApplyFirepowerMult;
		Valueable<bool> Crit_AnimOnAffectedTargets;
		Valueable<bool> Crit_SuppressWhenIntercepted;
		Valueable<bool> ReturnWarhead_ApplyChancePerTarget;

		Nullable<PartialVector2D<int>> BuildingGuardRetryDelay;

		Valueable<bool> Vertical_AircraftFix;

		Valueable<bool> Temporal_ApplyVersus;
		Valueable<bool> Temporal_ApplyMultiplier;

		Valueable<bool> DiscardOn_Sequences_Immediate;
		Valueable<bool> DiscardOn_MoveBasedOnDestination;
		Valueable<bool> DiscardOn_ConsiderHarvestingAsStationary;
		Valueable<bool> RemoveMindControl_Silent;
		Valueable<bool> MindControl_Permanent_ReplaceSilent;
		Nullable<bool> FlyNoWobbles;

		Valueable<AnimTypeClass*> DefaultLandingAnim;
		Nullable<AnimTypeClass*> DefaultLandingAnim_Dropship;
		Nullable<AnimTypeClass*> DefaultLandingAnim_Carryall;

		Valueable<DynamicTeamDelayType> TeamDelays_DynamicType;
		Valueable<Vector3D<int>> TeamDelays_Count[8];

		Valueable<Mission> BerzerkMission;

		Valueable<int> BunkerStateUpdateDelay;

		Valueable<bool> AllowChatBoxInSinglePlayer;

		Valueable<bool> NotHuman_RandomDeathSequence;
		Valueable<bool> OnlyUseLandSequences;
		Valueable<bool> SecondaryFireSequenceLandOnly;
		Valueable<bool> AutoRemoveEarliestBeacon;
		Valueable<bool> AllowBeaconHotKeyInSinglePlayer;
		
		Valueable<int> StartFacing;
		Valueable<bool> StartFacing_Random;

		Valueable<bool> AutoDeath_AllowLimboed;
		Valueable<bool> AutoDeath_OnOwnerChange_IgnoreRevertOnExit;
		Valueable<bool> AutoDeath_TechnosDontExist_AllowLimboed;
		Valueable<bool> AutoDeath_TechnosExist_AllowLimboed;

		Valueable<bool> AircraftDockingDir_DefaultToPoseDir;
		Nullable<int> PoseDir_Production;
		Nullable<int> PoseDir_Field;

		Valueable<bool> ApproachTarget_StopWhenInRange;

		ExtData(RulesClass* OwnerObject) : Extension<RulesClass>(OwnerObject)
			, Storage_TiberiumIndex { -1 }
			, HarvesterDumpAmount { 0.0f }
			, InfantryGainSelfHealCap {}
			, UnitsGainSelfHealCap {}
			, GainSelfHealAllowMultiplayPassive { true }
			, GainSelfHealFromPlayerControl { false }
			, GainSelfHealFromAllies { false }
			, EnemyInsignia { true }
			, DisguiseBlinkingVisibility { AffectedHouse::Owner }
			, ChronoSparkleDisplayDelay { 24 }
			, ChronoSparkleBuildingDisplayPositions { ChronoSparkleDisplayPosition::OccupantSlots }
			, ChronoSpherePreDelay { 60 }
			, ChronoSphereDelay { 0 }
			, AIChronoSphereSW {}
			, AIChronoWarpSW {}
			, SubterraneanSpeed { 19 }
			, SubterraneanHeight { -256 }
			, AISuperWeaponDelay {}
			, UseGlobalRadApplicationDelay { true }
			, RadApplicationDelay_Building { 0 }
			, RadBuildingDamageMaxCount { -1 }
			, RadSiteWarhead_Detonate { false }
			, RadSiteWarhead_Detonate_Full { true }
			, RadHasOwner { false }
			, RadHasInvoker { false }
			, ShieldApplyArmorMult { false }
			, JumpjetCrash { 5.0 }
			, JumpjetNoWobbles { false }
			, JumpjetRotateOnCrash { true }
			, VeinholeWarhead {}
			, MissingCameo { GameStrings::XXICON_SHP }

			, PlacementGrid_Translucency { 0 }
			, PlacementGrid_TranslucencyWithPreview { }
			, PlacementPreview { false }
			, PlacementPreview_Translucency { 75 }

			, SuperWeaponTimer_Percentage { false }
			, SuperWeaponSidebar_AllowByDefault { false }

			, Shield_ConditionYellow { }
			, Shield_ConditionRed { }
			, Pips_Shield { { -1,-1,-1 } }
			, Pips_Shield_Background { }
			, Pips_Shield_Building { { -1,-1,-1 } }
			, Pips_Shield_Building_Empty { }
			, Pips_SelfHeal_Infantry { { 13, 20 } }
			, Pips_SelfHeal_Units { { 13, 20 } }
			, Pips_SelfHeal_Buildings { { 13, 20 } }
			, Pips_SelfHeal_Infantry_Offset { { 25, -35 } }
			, Pips_SelfHeal_Units_Offset { { 33, -32 } }
			, Pips_SelfHeal_Buildings_Offset { { 15, 10 } }
			, Pips_Generic_Size { { 4, 0 } }
			, Pips_Generic_Buildings_Size { { 4, 2 } }
			, Pips_Ammo_Size { { 4, 0 } }
			, Pips_Ammo_Buildings_Size { { 4, 2 } }
			, Pips_Tiberiums_Frames {}
			, Pips_Tiberiums_EmptyFrame { 0 }
			, Pips_Tiberiums_DisplayOrder {}
			, Pips_Tiberiums_WeedFrame { 1 }
			, Pips_Tiberiums_WeedEmptyFrame { 0 }

			, HeightShadowScaling { false }
			, HeightShadowScaling_MinScale { 0.0 }
			, AirShadowBaseScale_log { 0.693376137 }

			, ExtendedAircraftMissions { false }
			, ExtendedAircraftMissions_UnlandDamage { -1 }
			, AircraftSpawnFromEdge { EdgeType::Owner }
			, AircraftRetreatToEdge { EdgeType::Owner }
			, AmphibiousEnter { false }
			, AmphibiousUnload { false }
			, NoQueueUpToEnter { false }
			, NoQueueUpToEnter_BoardDistance { 384 }
			, NoQueueUpToUnload { false }
			, NoQueueUpToEnter_Buildings {}
			, NoQueueUpToUnload_Buildings {}

			, JumpjetTilt { false }
			, JumpjetTilt_ForwardAccelFactor { 1.0 }
			, JumpjetTilt_ForwardSpeedFactor { 1.0 }
			, JumpjetTilt_SidewaysRotationFactor { 1.0 }
			, JumpjetTilt_SidewaysSpeedFactor { 1.0 }

			, Spawner_AttackImmediately { false }
			, Spawner_UseTurretFacing { false }
			, Spawner_RecycleRange { Leptons(-1) }
			, Spawner_RecycleOnTurret { false }
			, Promote_IncludeSpawns { false }
			, RadarJamHouses { AffectedHouse::Enemies }
			, RadarJamDelay { 30 }
			, MindControl_IgnoreSize { true }
			, MultiMindControl_ReleaseVictim { false }
			, MindControlLink_VisibleToHouse { AffectedHouse::All }
			, AlternateFLH_OnTurret { true }
			, AlternateFLH_ApplyVehicle { false }
			, DestroyAnim_Random { true }
			, UseDisguiseMovementSpeed { false }
			, Convert_ResetMindControl { false }
			, BuildLimitGroup_ContentIfAnyMatch { false }
			, BuildLimitGroup_NotBuildableIfQueueMatch { false }
			, DigitalDisplay_Health_FakeAtDisguise { true }
			, Overload_ParticleSysCount { 5 }
			, FallingDownDamage { 1.0 }
			, FallingDownDamage_AllowEMP { true }

			, ForceWeapon_InRange_TechnoOnly { true }
			, ForceWeapon_InRange_ApplyRangeModifiers { false }
			, ForceAAWeapon_InRange_ApplyRangeModifiers { false }

			, BuildingProductionQueue { false }

			, AllowParallelAIQueues { true }
			, ForbidParallelAIQueues_Aircraft { false }
			, ForbidParallelAIQueues_Building { false }
			, ForbidParallelAIQueues_Infantry { false }
			, ForbidParallelAIQueues_Navy { false }
			, ForbidParallelAIQueues_Vehicle { false }

			, EnablePowerSurplus { false }
			, PowerSurplus_ScaleToDrainAmount { 0 }

			, AllowDeployControlledMCV { false }

			, TypeSelectUseIFVMode { false }

			, IronCurtain_KeptOnDeploy { true }
			, IronCurtain_EffectOnOrganics { IronCurtainEffect::Kill }
			, IronCurtain_KillOrganicsWarhead { }
			, ForceShield_KeptOnDeploy { false }
			, ForceShield_EffectOnOrganics { IronCurtainEffect::Kill }
			, ForceShield_KillOrganicsWarhead { }
			, IronCurtain_ExtraTintIntensity { 0.0 }
			, ForceShield_ExtraTintIntensity { 0.0 }
			, AllowWeaponSelectAgainstWalls { false }
			, ColorAddUse8BitRGB { false }
			, AirstrikeLineColor { { 255, 0, 0 } }
			, AirstrikeLineZAdjust { 0 }
			, Strafing_SimulateBurst { false }
			, Strafing_UseAmmoPerShot { false }
			, Strafing_TargetCell { false }
			, OmniFire_TurnToTarget { false }
			, AmbientDamage_IgnoreTarget { false }
			, KeepRange_AllowAI { false }
			, KeepRange_AllowPlayer { false }
			, KeepRange_EarlyStopFrame { 0 }
			, AircraftWeapon_KickOutPassengers { true }
			, CrushSlowdownMultiplier { 0.2 }
			, SkipCrushSlowdown { false }
			, LaserPositionUpdate_StopOnFirerConvert { false }
			, LaserZAdjust { 0 }
			, EBoltZAdjust { 0 }
			, EBoltZAdjust_ClampInitialDepthForBuilding { true }
			, ROF_RandomDelay { { 0 ,2 } }
			, ToolTip_Background_Color { { 0, 0, 0 } }
			, ToolTip_Background_Opacity { 100 }
			, ToolTip_Background_BlurSize { 0.0f }
			, DisplayIncome { false }
			, DisplayIncome_Delay { 15 }
			, DisplayIncome_AllowAI { true }
			, DisplayIncome_Houses { AffectedHouse::All }
			, DrainMoneyDisplay { false }
			, DrainMoneyDisplay_Houses { AffectedHouse::All }
			, DrainMoneyDisplay_OnTarget { false }
			, DrainMoneyDisplay_OnTarget_UseDisplayIncome { true }
			, CrateOnlyOnLand { false }
			, UnitCrateVehicleCap { 50 }
			, FreeMCV_CreditsThreshold { 1500 }
			, RadialIndicatorVisibility { AffectedHouse::Allies }
			, DrawTurretShadow { false }
			, IsVoiceCreatedGlobal { false }
			, SetTabBySelectingFactory { false }
			, SelectionFlashDuration { 0 }
			, SetRecruitableOnLiberate { -1 }
			, DrawInsignia_OnlyOnSelected { false }
			, DrawInsignia_AdjustPos_Infantry { { 5, 2 } }
			, DrawInsignia_AdjustPos_Buildings { { 10, 6 } }
			, DrawInsignia_AdjustPos_BuildingsAnchor {}
			, DrawInsignia_AdjustPos_Units { { 10, 6 } }
			, DrawInsignia_UsePixelSelectionBracketDelta { { false } }
			, Promote_VeteranAnimation {}
			, Promote_EliteAnimation {}
			, AnimRemapDefaultColorScheme { 0 }
			, TimerBlinkColorScheme { 5 }
			, Buildings_DefaultDigitalDisplayTypes {}
			, Infantry_DefaultDigitalDisplayTypes {}
			, Vehicles_DefaultDigitalDisplayTypes {}
			, Aircraft_DefaultDigitalDisplayTypes {}
			, DefaultInfantrySelectBox {}
			, DefaultUnitSelectBox {}
			, VisualScatter_Min { Leptons(8) }
			, VisualScatter_Max { Leptons(32) }
			, ShowDesignatorRange { true }
			, ShowPowerPlantEnhancerRange { true }
			, DropPodTrailer { }
			, DropPodDefaultTrailer { }
			, PodImage { }
			, JumpjetClimbPredictHeight { false }
			, JumpjetClimbWithoutCutOut { false }
			, JumpjetClimbIgnoreBuilding { false }

			, DamageOwnerMultiplier { 1.0 }
			, DamageAlliesMultiplier { 1.0 }
			, DamageEnemiesMultiplier { 1.0 }
			, DamageOwnerMultiplier_NotAffectsEnemies {}
			, DamageAlliesMultiplier_NotAffectsEnemies {}
			, DamageOwnerMultiplier_Berzerk {}
			, DamageAlliesMultiplier_Berzerk {}
			, DamageEnemiesMultiplier_Berzerk {}
			, AircraftLevelLightMultiplier { 1.0 }
			, JumpjetLevelLightMultiplier { 0.0 }
			, VoxelLightSource { }
			// , VoxelShadowLightSource { }

			, CombatAlert { false }
			, CombatAlert_Default {}
			, CombatAlert_IgnoreBuilding { true }
			, CombatAlert_SuppressIfInScreen { true }
			, CombatAlert_Interval { 150 }
			, CombatAlert_SuppressIfAllyDamage { true }
			, CombatAlert_MakeAVoice { true }
			, CombatAlert_UseFeedbackVoice { true }
			, CombatAlert_UseAttackVoice { true }
			, CombatAlert_UseEVA { true }
			, UseFixedVoxelLighting { false }
			, AIAutoDeployMCV { true }
			, AISetBaseCenter { true }
			, AIBiasSpawnCell { false }
			, AIForbidConYard { false }
			, AINodeWallsOnly { false }
			, AICleanWallNode { false }
			, AttackMove_Aggressive { false }
			, AttackMove_UpdateTarget { false }
			, MindControl_ThreatDelay { 0 }
			, RecountBurst { false }
			, NoRearm_UnderEMP { false }
			, NoRearm_Temporal { false }
			, NoReload_UnderEMP { false }
			, NoReload_Temporal { false }
			, NoTurret_TrackTarget { false }
			, GatherWhenMCVDeploy { true }
			, AIFireSale { true }
			, AIFireSaleDelay { 0 }
			, AIAllToHunt { true }
			, RepairBaseNodes { false }
			, FixRepairStepCost { false }
			, WarheadParticleAlphaImageIsLightFlash { false }
			, CombatLightDetailLevel { 0 }
			, CombatLightDetailLevel_CheckColored { false }
			, LightFlashAlphaImageDetailLevel { 0 }
			, UseRetintFix { true }
			, AINormalTargetingDelay {}
			, PlayerNormalTargetingDelay {}
			, AIGuardAreaTargetingDelay {}
			, PlayerGuardAreaTargetingDelay {}
			, AIAttackMoveTargetingDelay {}
			, PlayerAttackMoveTargetingDelay {}
			, DistributeTargetingFrame { false }
			, DistributeTargetingFrame_AIOnly { true }
			, CanTargetAI_IronCurtained { false }
			, CanTarget_IronCurtained { true }
			, AutoTarget_IronCurtained { true }
			, BuildingWaypoints { false }
			, BuildingTypeSelectable { false }
			, ProneSpeed_Crawls { 0.67 }
			, ProneSpeed_NoCrawls { 1.5 }

			, DamagedSpeed { 0.75 }

			, HarvesterScanAfterUnload { false }

			, AnimCraterDestroyTiberium { true }

			, BerzerkTargeting { AffectedHouse::All }
			, AllowBerzerkOnAllies { false }

			, TintColorIronCurtain { 0 }
			, TintColorForceShield { 0 }
			, TintColorBerserk { 0 }

			, AttackMove_IgnoreWeaponCheck { false }

			, Parasite_GrappleAnim {}
			, Parasite_AllowWaterExit {}
			, InfantryAutoDeploy { false }
			, AdjacentWallDamage { 200 }

			, WarheadAnimZAdjust { -15 }

			, IvanBombAttachToCenter { false }

			, FallingDownTargetingFix { false }
			, AIAirTargetingFix { false }

			, ReloadInTransport { false }
			, OpenTopped_IgnoreRangefinding { false }
			, OpenTopped_AllowFiringIfDeactivated { true }
			, OpenTopped_AllowFiringIfAttackedByLocomotor { true }
			, OpenTopped_ShareTransportTarget { true }
			, OpenTopped_UseTransportRangeModifiers { false }
			, OpenTopped_CheckTransportDisableWeapons { false }
			, OpenTopped_DecloakToFire { false }
			, OpenTopped_FireWhileMoving { true }
			, OpenTransport_RangeBonus { 0 }
			, OpenTransport_DamageMultiplier { 1.0f }
			, OpenTransport_FireWhileMoving { true }

			, Passengers_SyncOwner { false }
			, Passengers_SyncOwner_RevertOnExit { true }

			, Explodes_KillPassengers { true }
			, Explodes_DuringBuildup { true }

			, AircraftFiringForceScatter { true }

			, HoverDrownable { true }

			, Arcing_AllowElevationInaccuracy { true }

			, Terrain_IsPassable { false }
			, Tibtree_IsPassable { false }
			, Terrain_CanBeBuiltOn { false }
			, Tibtree_CanBeBuiltOn { false }

			, Sinkable {}
			, Sinkable_SquidGrab { true }
			, SinkSpeed { 5 }

			, CreateAnimsOnZeroDamage { false }
			, Conventional_IgnoreUnits { false }
			, DecloakDamagedTargets { true }
			, ShakeIsLocal { false }
			, ApplyModifiersOnNegativeDamage { false }
			, AllowDamageOnSelf { false }
			, Debris_Conventional { false }
			, Parasite_DisableParticleSystem { false }

			, ProjectileInterceptable { false }
			, Interceptor_GuardRange_IsCylindrical { false }
			, Interceptor_ApplyFirepowerMult { true }

			, SortCameoByName { false }

			, MergeBuildingDamage { false }

			, BuildingRadioLink_SyncOwner { true }

			, ApplyPerTargetEffectsOnDetonate { true }
			, AffectsInvokerOnly_IgnoreInvokerState { true }

			, ExtraRange_TargetMoving { Leptons(0) }
			, ExtraRange_TargetMoving_CloseRangeOnly { false }
			, ExtraRange_FirerMoving { Leptons(0) }
			, ExtraRange_Prefiring { Leptons(0) }
			, ExtraRange_Prefiring_IncludeBurst { true }

			, AutoTarget_NoThreatBuildings { false }
			, AutoTargetAI_NoThreatBuildings { true }

			, ParadropMission { Mission::Guard }
			, AIParadropMission { Mission::Hunt }
			, ParadropDelay { 5 }
			, ParadropEndDelay { 5 }

			, DefaultToGuardArea { false }
			, LeptonMindControlOffset { 70 }
			, MindControlRingOffset { 140 }

			, CylinderRangefinding { false }

			, PenetratesTransport_Level { 10 }

			, UnitsUnsellable { false }

			, DriverKilled_KeptPassengers { false }
			, DriverKilled_KillPassengers { false }
			, DisableOveroptimizationInTargeting { false }
			, ExtraThreat_IsThreat { 0.0 }
			, ExtraThreat_InRange { 0.0 }
			, ExtraThreatCoefficient_InRangeDistance { 0.0 }
			, ExtraThreatCoefficient_Facing { 0.0 }
			, ExtraThreatCoefficient_DistanceToLastTarget { 0.0 }

			, BalloonHoverPathingFix { false }

			, WalkLocomotorMakesWake { false }
			, DriveLocomotorMakesWake { true }
			, HoverLocomotorMakesWake { true }
			, ShipLocomotorMakesWake { true }
			, FiringAnim_Update { false }
			, ExtendedPlayerRepair { false }

			, Psychedelic_StackingMode { StackingMode::Override }
			, Shrapnel_AffectsGround { false }
			, Shrapnel_AffectsBuildings { false }
			, Shrapnel_UseWeaponTargeting { false }
			, Shrapnel_IgnoreHitBuildings { false }
			, Shrapnel_ObeyWarheadTriggerConditions { true }
			, ReturnWeapon_ApplyFirepowerMult { false }
			, Splits_TargetingDistance_Cylindrical { false }
			, Splits_AllowRepeatTargets { false }
			, Splits_UseWeaponTargeting { false }
			, Airburst_UseCluster { false }
			, Airburst_TargetAsSource_SkipHeight { false }
			, AirburstWeapon_ApplyFirepowerMult { false }
			, AirburstWeapon_UseFiringEffects { false }
			, AirburstWeapon_HeadToTarget { false }
			, AnimDamage_DealtByInvoker { false }
			, AnimDamage_ApplyFirepowerMult { false }
			, Crit_ApplyChancePerTarget { false }
			, Crit_ExtraDamage_ApplyFirepowerMult { false }
			, Crit_AnimOnAffectedTargets { false }
			, Crit_SuppressWhenIntercepted { false }
			, ReturnWarhead_ApplyChancePerTarget { false }
			, BuildingGuardRetryDelay {}
			, Vertical_AircraftFix { true }
			, Temporal_ApplyVersus { false }
			, Temporal_ApplyMultiplier { false }
			, DiscardOn_Sequences_Immediate { true }
			, DiscardOn_MoveBasedOnDestination { false }
			, DiscardOn_ConsiderHarvestingAsStationary { true }
			, RemoveMindControl_Silent { false }
			, MindControl_Permanent_ReplaceSilent { false }

			, FlyNoWobbles {}

			, DefaultLandingAnim { nullptr }
			, DefaultLandingAnim_Dropship {}
			, DefaultLandingAnim_Carryall {}

			, TeamDelays_DynamicType { DynamicTeamDelayType::StartingPoint }
			, TeamDelays_Count {}

			, BerzerkMission { Mission::Hunt }

			, BunkerStateUpdateDelay { 15 }

			, AllowChatBoxInSinglePlayer { false }

			, NotHuman_RandomDeathSequence { false }
			, OnlyUseLandSequences { false }
			, SecondaryFireSequenceLandOnly { true }

			, AutoRemoveEarliestBeacon { false }

			, AllowBeaconHotKeyInSinglePlayer { false }

			, StartFacing { 0 }
			, StartFacing_Random { false }

			, AutoDeath_AllowLimboed { true }
			, AutoDeath_OnOwnerChange_IgnoreRevertOnExit { false }
			, AutoDeath_TechnosDontExist_AllowLimboed { false }
			, AutoDeath_TechnosExist_AllowLimboed { false }

			, AircraftDockingDir_DefaultToPoseDir{ true }
			, PoseDir_Production {}
			, PoseDir_Field{}

			, ApproachTarget_StopWhenInRange { false }
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void LoadBeforeTypeData(RulesClass* pThis, CCINIClass* pINI);
		virtual void LoadAfterTypeData(RulesClass* pThis, CCINIClass* pINI);
		virtual void InitializeConstants() override;
		void InitializeAfterTypeData(RulesClass* pThis);
		void InitializeAfterAllLoaded();

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

		void ReplaceVoxelLightSources();

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

private:
	static std::unique_ptr<ExtData> Data;

public:
	static IStream* g_pStm;

	static void Allocate(RulesClass* pThis);
	static void Remove(RulesClass* pThis);

	static void LoadFromINIFile(RulesClass* pThis, CCINIClass* pINI);
	static void LoadBeforeTypeData(RulesClass* pThis, CCINIClass* pINI);
	static void LoadAfterTypeData(RulesClass* pThis, CCINIClass* pINI);

	static ExtData* Global()
	{
		return Data.get();
	}

	static void Clear()
	{
		Allocate(RulesClass::Instance);
	}
};
