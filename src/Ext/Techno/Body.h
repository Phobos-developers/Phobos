#pragma once

#include <Ext/TechnoType/Body.h>
#include <Ext/Radio/Body.h>
#include <Utilities/Container.h>
#include <Utilities/Detach.h>
#include <Utilities/TemplateDef.h>
#include <New/Entity/ShieldClass.h>
#include <New/Entity/LaserTrailClass.h>
#include <New/Entity/AttachEffectClass.h>

class AirstrikeClass;
class BulletClass;

class TechnoExt : public RadioExt, public Detach::Listener<AirstrikeClass>
{
public:
	using base_type = TechnoClass;

	// deprecated: the pre-rework nested data class is now the extension class itself
	using ExtData [[deprecated("use the extension class itself instead")]] = TechnoExt;

	static constexpr DWORD Canary = 0x55555555;

public:
	// typed owner accessor
	TechnoClass* OwnerObject() const
	{
		return static_cast<TechnoClass*>(this->GetAttachedObject());
	}

	TechnoTypeExt* TypeExtData;
	std::unique_ptr<ShieldClass> Shield;
	std::vector<std::unique_ptr<LaserTrailClass>> LaserTrails;
	std::vector<std::unique_ptr<AttachEffectClass>> AttachedEffects;
	AttachEffectTechnoProperties AE;
	std::vector<EBolt*> ElectricBolts; // EBolts are not serialized so do not serialize this either.
	int AnimRefCount; // Used to keep track of how many times this techno is referenced in anims f.ex Invoker, ParentBuilding etc., for pointer invalidation.
	CDTimerClass PassengerDeletionTimer;
	ShieldTypeClass* CurrentShieldType;
	CDTimerClass ChargeTurretTimer; // Used for charge turrets instead of RearmTimer if weapon has ChargeTurret.Delays set.
	CDTimerClass AutoDeathTimer;
	AnimTypeClass* MindControlRingAnimType;
	int DamageNumberOffset;
	bool HasBeenPlacedOnMap; // Set to true on first Unlimbo() call.
	bool ForceFullRearmDelay;
	bool LastRearmWasFullDelay;
	bool CanCloakDuringRearm; // Current rearm timer was started by DecloakToFire=no weapon.
	int WHAnimRemainingCreationInterval;
	WeaponTypeClass* LastWeaponType;
	CellClass* FiringObstacleCell; // Set on firing if there is an obstacle cell between target and techno, used for updating WaveClass target etc.
	bool IsDetachingForCloak; // Used for checking animation detaching, set to true before calling Detach_All() on techno when this anim is attached to and to false after when cloaking only.
	int BeControlledThreatFrame;
	DWORD LastTargetID;
	int AccumulatedGattlingValue;
	bool ShouldUpdateGattlingValue;
	int AttachedEffectInvokerCount;

	bool DelayedFireSequencePaused;
	int DelayedFireWeaponIndex;
	CDTimerClass DelayedFireTimer;
	AnimClass* CurrentDelayedFireAnim;

	AirstrikeClass* AirstrikeTargetingMe;

	bool IsSelected;

	// cache tint values
	int TintColorOwner;
	int TintColorAllies;
	int TintColorEnemies;
	int TintIntensityOwner;
	int TintIntensityAllies;
	int TintIntensityEnemies;

	bool SpecialTracked;
	bool FallingDownTracked;

	bool OnParachuted; // This is just a temporary patch. TODO: fully check HasParachuted and correct its maintenance method.
	bool HoverShutdown;
	CoordStruct LastTargetCrd;
	CDTimerClass LastTargetCrdClearTimer;

	bool ShouldBeDead;

	int DropCrate; // Drop crate on death, modified by map action
	Powerup DropCrateType;

	bool PreventCrewEscape;

	TechnoExt(TechnoClass* OwnerObject) : RadioExt(OwnerObject)
		, TypeExtData { nullptr }
		, Shield {}
		, LaserTrails {}
		, AttachedEffects {}
		, AE {}
		, ElectricBolts {}
		, AnimRefCount { 0 }
		, PassengerDeletionTimer {}
		, CurrentShieldType { nullptr }
		, ChargeTurretTimer {}
		, AutoDeathTimer {}
		, MindControlRingAnimType { nullptr }
		, DamageNumberOffset { INT32_MIN }
		, HasBeenPlacedOnMap { false }
		, ForceFullRearmDelay { false }
		, LastRearmWasFullDelay { false }
		, CanCloakDuringRearm { false }
		, WHAnimRemainingCreationInterval { 0 }
		, LastWeaponType {}
		, FiringObstacleCell {}
		, IsDetachingForCloak { false }
		, BeControlledThreatFrame { 0 }
		, LastTargetID { 0xFFFFFFFF }
		, AccumulatedGattlingValue { 0 }
		, ShouldUpdateGattlingValue { false }
		, AirstrikeTargetingMe { nullptr }
		, DelayedFireSequencePaused { false }
		, DelayedFireWeaponIndex { -1 }
		, DelayedFireTimer {}
		, CurrentDelayedFireAnim { nullptr }
		, AttachedEffectInvokerCount { 0 }
		, IsSelected { false }
		, TintColorOwner { 0 }
		, TintColorAllies { 0 }
		, TintColorEnemies { 0 }
		, TintIntensityOwner { 0 }
		, TintIntensityAllies { 0 }
		, TintIntensityEnemies { 0 }
		, SpecialTracked { false }
		, FallingDownTracked { false }
		, OnParachuted { false }
		, HoverShutdown { false }
		, LastTargetCrd { CoordStruct::Empty }
		, LastTargetCrdClearTimer {}
		, ShouldBeDead { false }
		, DropCrate { -1 }
		, DropCrateType { Powerup::Money }
		, PreventCrewEscape { false }
	{ }

	void OnEarlyUpdate();

	// the extension state that goes with TechnoClass::Init
	void InitializeState(TechnoTypeClass* pType = nullptr);

	// the techno was created while a savegame was loading, so TechnoClass::Init found
	// no extension to initialize; catch up now that there is one
	virtual void OnDeferredAllocation() override { this->InitializeState(); }

	// True while the object is hidden underground (subterranean units); false for
	// everything else. Overridden by UnitExt, which owns the burrow state.
	virtual bool IsBurrowedState() const { return false; }

	// True while the object is inside a tunnel (foot units); false for everything
	// else. Overridden by FootExt, which owns the tunnel state.
	virtual bool IsInTunnelState() const { return false; }

	void ApplyInterceptor();
	bool CheckDeathConditions(bool isInLimbo = false);
	void EatPassengers();
	void UpdateShield();
	void ApplySpawnLimitRange();
	void UpdateLaserTrails();
	void UpdateAttachEffects();
	void UpdateGattlingRateDownReset();
	void UpdateCumulativeAttachEffects(AttachEffectTypeClass* pAttachEffectType, bool createAnim = false);
	bool RecalculateStatMultipliers(AttachEffectClass* pAttachEffect = nullptr);
	void UpdateTemporal();
	void UpdateMindControlAnim();
	void UpdateRecountBurst();
	void UpdateRearmInEMPState();
	void UpdateRearmInTemporal();
	void InitializeLaserTrails();
	void InitializeAttachEffects();
	void UpdateSelfOwnedAttachEffects();
	bool HasAttachedEffects(std::vector<AttachEffectTypeClass*> attachEffectTypes, bool requireAll, bool ignoreSameSource, TechnoClass* pInvoker, AbstractClass* pSource, std::vector<int> const* minCounts, std::vector<int> const* maxCounts) const;
	int GetAttachedEffectCumulativeCount(AttachEffectTypeClass* pAttachEffectType, bool ignoreSameSource = false, TechnoClass* pInvoker = nullptr, AbstractClass* pSource = nullptr) const;
	void InitializeDisplayInfo();
	void ApplyMindControlRangeLimit();
	int ApplyForceWeaponInRange(AbstractClass* pTarget);
	void ResetDelayedFireTimer();
	void UpdateTintValues();
	void UpdateLastTargetCrd();
	int GetSight();

	static bool CanReceiveEvent(TechnoClass* pThis, HouseClass* pHouse);

	virtual ~TechnoExt() override;
	virtual void OnDetach(AirstrikeClass* pTarget, bool removed) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);

public:
	// TechnoExt is never instantiated and has no container of its own: instances are
	// concrete leaves (UnitExt/InfantryExt/AircraftExt/BuildingExt) tracked by their
	// own containers. The polymorphic fetch reads the inline slot directly.
	static TechnoExt* Fetch(const TechnoClass* pThis)
	{
		return AbstractExt::Fetch<TechnoExt>(pThis);
	}

	static TechnoExt* TryFetch(const TechnoClass* pThis)
	{
		return AbstractExt::TryFetch<TechnoExt>(pThis);
	}

	// deprecated stand-in for the pre-rework container of all TechnoClass extensions
	static inline CompatExtMap<TechnoExt, TechnoClass> ExtMap {};

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static bool IsActive(TechnoClass* pThis);

	static bool IsHarvesting(TechnoClass* pThis);
	static bool HasAvailableDock(TechnoClass* pThis);
	static bool HasRadioLinkWithDock(TechnoClass* pThis);


	static Matrix3D GetTransform(TechnoClass* pThis, VoxelIndexKey* pKey = nullptr, bool isShadow = false);
	static Matrix3D GetFLHMatrix(TechnoClass* pThis, const CoordStruct& flh, bool isOnTurret, double factor = 1.0, bool isShadow = false, int turIdx = -1);
	static Matrix3D TransformFLHForTurret(TechnoClass* pThis, Matrix3D mtx, bool isOnTurret, double factor = 1.0, int turIdx = -1);
	static CoordStruct GetFLHAbsoluteCoords(TechnoClass* pThis, const CoordStruct& flh, bool isOnTurret = false, int turIdx = -1);

	static CoordStruct GetBurstFLH(TechnoClass* pThis, int weaponIndex, bool& FLHFound);

	static void ChangeOwnerMissionFix(FootClass* pThis);
	static void KillSelf(TechnoClass* pThis, AutoDeathBehavior deathOption, const std::vector<AnimTypeClass*>& pVanishAnimation, bool isInLimbo = false);
	static void ObjectKilledBy(TechnoClass* pThis, TechnoClass* pKiller);
	static void UpdateSharedAmmo(TechnoClass* pThis);
	static bool HasAdditionalAbility(TechnoClass* pThis, AdditionalAbility ability);
	static double GetCurrentSpeedMultiplier(FootClass* pThis);
	static double GetCurrentFirepowerMultiplier(TechnoClass* pThis);
	static double GetCurrentArmorMultiplier(TechnoClass* pThis, TechnoTypeClass* pType, HouseClass* pSourceHouse = nullptr, WarheadTypeClass* pWarhead = nullptr);
	static double CalculateArmorMultipliers(TechnoClass* pThis, WarheadTypeClass* pWarhead, HouseClass* pSourceHouse, bool hitAnim = false);
	static void DrawSelfHealPips(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds);
	static void DrawInsignia(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds);
	static void ApplyGainedSelfHeal(TechnoClass* pThis);
	static void SyncInvulnerability(TechnoClass* pFrom, TechnoClass* pTo);
	static CoordStruct PassengerKickOutLocation(TechnoClass* pThis, FootClass* pPassenger, int maxAttempts);
	static bool AllowedTargetByZone(TechnoClass* pThis, TechnoClass* pTarget, TargetZoneScanType zoneScanType, WeaponTypeClass* pWeapon = nullptr, bool useZone = false, int zone = -1);
	static void UpdateAttachedAnimLayers(TechnoClass* pThis);
	static bool ConvertToType(FootClass* pThis, TechnoTypeClass* toType);
	static bool IsTypeImmune(TechnoClass* pThis, TechnoClass* pSource);
	static int GetTintColor(TechnoClass* pThis, bool invulnerability, bool airstrike, bool berserk);
	static int GetCustomTintColor(TechnoClass* pThis);
	static int GetCustomTintIntensity(TechnoClass* pThis);
	static void ApplyCustomTintValues(TechnoClass* pThis, int& color, int& intensity);
	static Point2D GetScreenLocation(TechnoClass* pThis);
	static Point2D GetFootSelectBracketPosition(TechnoClass* pThis, Anchor anchor, bool isInfantry);
	static Point2D GetBuildingSelectBracketPosition(TechnoClass* pThis, TechnoTypeClass* pType, BuildingSelectBracketPosition bracketPosition);
	static void DrawSelectBox(TechnoClass* pThis, const Point2D* pLocation, const RectangleStruct* pBounds, bool drawBefore = false);
	static void ProcessDigitalDisplays(TechnoClass* pThis);
	static int GetDropCrateIndex(TechnoClass* pThis);
	static void GetValuesForDisplay(TechnoClass* pThis, TechnoTypeClass* pType, DisplayInfoType infoType, int& value, int& maxValue, int infoIndex);
	static bool IsValidTechno(TechnoClass* pTechno, bool checkIfInTransportOrAbsorbed = true);
	static bool IsValidTechno(AbstractClass* pObject, bool checkIfInTransportOrAbsorbed = true);
	static void GetDigitalDisplayFakeHealth(TechnoClass* pThis, int& value, int& maxValue);
	static void CreateDelayedFireAnim(TechnoClass* pThis, AnimTypeClass* pAnimType, int weaponIndex, bool attach, bool center, bool removeOnNoDelay, bool onTurret, CoordStruct firingCoords);
	static bool HandleDelayedFireWithPauseSequence(TechnoClass* pThis, WeaponTypeClass* pWeapon, int weaponIndex, int frame, int firingFrame);
	static bool IsHealthInThreshold(TechnoClass* pObject, double min, double max);
	static void ShowPromoteAnim(TechnoClass* pThis);
	static void ClickedApproachObject(FootClass* pThis, ObjectClass* pObject);
	static bool CanBeRecruitedFix(FootClass* pThis, HouseClass* pHouse);

	static bool EjectRandomly(FootClass* pEjectee, const CoordStruct& coords, int distance, bool select);
	static bool EjectSurvivor(FootClass* pSurvivor, CoordStruct coords, bool select);
	static bool __fastcall ApplyKillDriver(TechnoClass** pData, void*, HouseClass* pToHouse, TechnoClass* pKiller, bool resetVeterancy);

	// WeaponHelpers.cpp
	static int PickWeaponIndex(TechnoClass* pThis, TechnoClass* pTargetTechno, AbstractClass* pTarget, int weaponIndexOne, int weaponIndexTwo, bool allowFallback = true, bool allowAAFallback = true);
	static void FireWeaponAtSelf(TechnoClass* pThis, WeaponTypeClass* pWeaponType);
	static bool CanFireNoAmmoWeapon(TechnoClass* pThis, int weaponIndex);
	static bool CanFireNoAmmoWeapon(TechnoClass* pThis, TechnoTypeClass* pType, int weaponIndex);
	static WeaponTypeClass* GetDeployFireWeapon(TechnoClass* pThis, TechnoTypeClass* pType, int& weaponIndex);
	static WeaponTypeClass* GetDeployFireWeapon(TechnoClass* pThis, TechnoTypeClass* pType);
	static WeaponTypeClass* GetCurrentWeapon(TechnoClass* pThis, TechnoTypeClass* pType, int& weaponIndex, bool getSecondary = false);
	static WeaponTypeClass* GetCurrentWeapon(TechnoClass* pThis, TechnoTypeClass* pType, bool getSecondary = false);
	static int GetWeaponIndexAgainstWall(TechnoClass* pThis, OverlayTypeClass* pWallOverlayType);
	static void ApplyKillWeapon(TechnoClass* pThis, TechnoClass* pSource, WarheadTypeClass* pWH);
	static void ApplyRevengeWeapon(TechnoClass* pThis, TechnoClass* pSource, WarheadTypeClass* pWH);
	static bool TryToCreateCrate(CoordStruct location, Powerup selectedPowerup = Powerup::Money, int maxCellRange = 10);
	static bool MultiWeaponCanFire(TechnoClass* const pThis, AbstractClass* const pTarget, WeaponTypeClass* const pWeaponType);
	static bool HasWeaponsDisabled(TechnoClass* pThis);
	static FireError GetFireErrorIgnoreDisableWeapons(TechnoClass* pThis, AbstractClass* pTarget, int weaponIndex, bool ignoreRange);
};

