#pragma once
#include <InfantryClass.h>
#include <AnimClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Macro.h>
#include <New/Entity/ShieldClass.h>
#include <New/Entity/LaserTrailClass.h>
#include <New/Entity/AttachEffectClass.h>

class BulletClass;

class TechnoExt
{
public:
	using base_type = TechnoClass;

	static constexpr DWORD Canary = 0x55555555;
	static constexpr size_t ExtPointerOffset = 0x34C;

	class ExtData final : public Extension<TechnoClass>
	{
	public:
		TechnoTypeExt::ExtData* TypeExtData;
		std::unique_ptr<ShieldClass> Shield;
		std::vector<LaserTrailClass> LaserTrails;
		std::vector<std::unique_ptr<AttachEffectClass>> AttachedEffects;
		AttachEffectTechnoProperties AE;
		bool ReceiveDamage;
		bool LastKillWasTeamTarget;
		CDTimerClass PassengerDeletionTimer;
		ShieldTypeClass* CurrentShieldType;
		int LastWarpDistance;
		CDTimerClass ChargeTurretTimer; // Used for charge turrets instead of RearmTimer if weapon has ChargeTurret.Delays set.
		CDTimerClass AutoDeathTimer;
		AnimTypeClass* MindControlRingAnimType;
		int DamageNumberOffset;
		int Strafe_BombsDroppedThisRound;
		int CurrentAircraftWeaponIndex;
		bool IsInTunnel;
		bool IsBurrowed;
		bool HasBeenPlacedOnMap; // Set to true on first Unlimbo() call.
		CDTimerClass DeployFireTimer;
		bool SkipTargetChangeResetSequence;
		bool ForceFullRearmDelay;
		bool LastRearmWasFullDelay;
		bool CanCloakDuringRearm; // Current rearm timer was started by DecloakToFire=no weapon.
		int WHAnimRemainingCreationInterval;
		bool CanCurrentlyDeployIntoBuilding; // Only set on UnitClass technos with DeploysInto set in multiplayer games, recalculated once per frame so no need to serialize.
		CellClass* FiringObstacleCell; // Set on firing if there is an obstacle cell between target and techno, used for updating WaveClass target etc.
		bool IsDetachingForCloak; // Used for checking animation detaching, set to true before calling Detach_All() on techno when this anim is attached to and to false after when cloaking only.

		// Used for Passengers.SyncOwner.RevertOnExit instead of TechnoClass::InitialOwner / OriginallyOwnedByHouse,
		// as neither is guaranteed to point to the house the TechnoClass had prior to entering transport and cannot be safely overridden.
		HouseClass* OriginalPassengerOwner;
		bool HasRemainingWarpInDelay;          // Converted from object with Teleport Locomotor to one with a different Locomotor while still phasing in OR set if ChronoSphereDelay > 0.
		int LastWarpInDelay;                   // Last-warp in delay for this unit, used by HasCarryoverWarpInDelay.
		bool IsBeingChronoSphered;             // Set to true on units currently being ChronoSphered, does not apply to Ares-ChronoSphere'd buildings or Chrono reinforcements.

		ExtData(TechnoClass* OwnerObject) : Extension<TechnoClass>(OwnerObject)
			, TypeExtData { nullptr }
			, Shield {}
			, LaserTrails {}
			, AttachedEffects {}
			, AE {}
			, ReceiveDamage { false }
			, LastKillWasTeamTarget { false }
			, PassengerDeletionTimer {}
			, CurrentShieldType { nullptr }
			, LastWarpDistance {}
			, ChargeTurretTimer {}
			, AutoDeathTimer {}
			, MindControlRingAnimType { nullptr }
			, DamageNumberOffset { INT32_MIN }
			, Strafe_BombsDroppedThisRound { 0 }
			, CurrentAircraftWeaponIndex {}
			, IsInTunnel { false }
			, IsBurrowed { false }
			, HasBeenPlacedOnMap { false }
			, DeployFireTimer {}
			, SkipTargetChangeResetSequence { false }
			, ForceFullRearmDelay { false }
			, LastRearmWasFullDelay { false }
			, CanCloakDuringRearm { false }
			, WHAnimRemainingCreationInterval { 0 }
			, CanCurrentlyDeployIntoBuilding { false }
			, FiringObstacleCell {}
			, IsDetachingForCloak { false }
			, OriginalPassengerOwner {}
			, HasRemainingWarpInDelay { false }
			, LastWarpInDelay { 0 }
			, IsBeingChronoSphered { false }
		{ }

		void OnEarlyUpdate();

		void ApplyInterceptor();
		bool CheckDeathConditions(bool isInLimbo = false);
		void DepletedAmmoActions();
		void EatPassengers();
		void UpdateShield();
		void UpdateOnTunnelEnter();
		void ApplySpawnLimitRange();
		void UpdateTypeData(TechnoTypeClass* currentType);
		void UpdateLaserTrails();
		void UpdateAttachEffects();
		void UpdateCumulativeAttachEffects(AttachEffectTypeClass* pAttachEffectType, AttachEffectClass* pRemoved = nullptr);
		void RecalculateStatMultipliers();
		void UpdateTemporal();
		void UpdateMindControlAnim();
		void InitializeLaserTrails();
		void InitializeAttachEffects();
		void UpdateSelfOwnedAttachEffects();
		bool HasAttachedEffects(std::vector<AttachEffectTypeClass*> attachEffectTypes, bool requireAll, bool ignoreSameSource, TechnoClass* pInvoker, AbstractClass* pSource, std::vector<int> const* minCounts, std::vector<int> const* maxCounts) const;
		int GetAttachedEffectCumulativeCount(AttachEffectTypeClass* pAttachEffectType, bool ignoreSameSource = false, TechnoClass* pInvoker = nullptr, AbstractClass* pSource = nullptr) const;

		virtual ~ExtData() override;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<TechnoExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static bool IsActive(TechnoClass* pThis);

	static bool IsHarvesting(TechnoClass* pThis);
	static bool HasAvailableDock(TechnoClass* pThis);

	static CoordStruct GetFLHAbsoluteCoords(TechnoClass* pThis, CoordStruct flh, bool turretFLH = false);

	static CoordStruct GetBurstFLH(TechnoClass* pThis, int weaponIndex, bool& FLHFound);
	static CoordStruct GetSimpleFLH(InfantryClass* pThis, int weaponIndex, bool& FLHFound);

	static void ChangeOwnerMissionFix(FootClass* pThis);
	static void KillSelf(TechnoClass* pThis, AutoDeathBehavior deathOption, AnimTypeClass* pVanishAnimation, bool isInLimbo = false);
	static void ApplyMindControlRangeLimit(TechnoClass* pThis);
	static void ObjectKilledBy(TechnoClass* pThis, TechnoClass* pKiller);
	static void UpdateSharedAmmo(TechnoClass* pThis);
	static double GetCurrentSpeedMultiplier(FootClass* pThis);
	static void DrawSelfHealPips(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds);
	static void DrawInsignia(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds);
	static void ApplyGainedSelfHeal(TechnoClass* pThis);
	static void SyncInvulnerability(TechnoClass* pFrom, TechnoClass* pTo);
	static CoordStruct PassengerKickOutLocation(TechnoClass* pThis, FootClass* pPassenger, int maxAttempts);
	static bool AllowedTargetByZone(TechnoClass* pThis, TechnoClass* pTarget, TargetZoneScanType zoneScanType, WeaponTypeClass* pWeapon = nullptr, bool useZone = false, int zone = -1);
	static void UpdateAttachedAnimLayers(TechnoClass* pThis);
	static bool ConvertToType(FootClass* pThis, TechnoTypeClass* toType);
	static bool CanDeployIntoBuilding(UnitClass* pThis, bool noDeploysIntoDefaultValue = false);
	static bool IsTypeImmune(TechnoClass* pThis, TechnoClass* pSource);
	static int GetTintColor(TechnoClass* pThis, bool invulnerability, bool airstrike, bool berserk);
	static int GetCustomTintColor(TechnoClass* pThis);
	static int GetCustomTintIntensity(TechnoClass* pThis);
	static void ApplyCustomTintValues(TechnoClass* pThis, int& color, int& intensity);
	static Point2D GetScreenLocation(TechnoClass* pThis);
	static Point2D GetFootSelectBracketPosition(TechnoClass* pThis, Anchor anchor);
	static Point2D GetBuildingSelectBracketPosition(TechnoClass* pThis, BuildingSelectBracketPosition bracketPosition);
	static void ProcessDigitalDisplays(TechnoClass* pThis);
	static void GetValuesForDisplay(TechnoClass* pThis, DisplayInfoType infoType, int& value, int& maxValue);

	// WeaponHelpers.cpp
	static int PickWeaponIndex(TechnoClass* pThis, TechnoClass* pTargetTechno, AbstractClass* pTarget, int weaponIndexOne, int weaponIndexTwo, bool allowFallback = true, bool allowAAFallback = true);
	static void FireWeaponAtSelf(TechnoClass* pThis, WeaponTypeClass* pWeaponType);
	static bool CanFireNoAmmoWeapon(TechnoClass* pThis, int weaponIndex);
	static WeaponTypeClass* GetDeployFireWeapon(TechnoClass* pThis, int& weaponIndex);
	static WeaponTypeClass* GetDeployFireWeapon(TechnoClass* pThis);
	static WeaponTypeClass* GetCurrentWeapon(TechnoClass* pThis, int& weaponIndex, bool getSecondary = false);
	static WeaponTypeClass* GetCurrentWeapon(TechnoClass* pThis, bool getSecondary = false);
	static int GetWeaponIndexAgainstWall(TechnoClass* pThis, OverlayTypeClass* pWallOverlayType);
};
