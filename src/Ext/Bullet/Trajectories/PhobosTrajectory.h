#pragma once

#include <BulletClass.h>

#include <Ext/WeaponType/Body.h>
#include <Utilities/EnumFunctions.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Savegame.h>

enum class TrajectoryFlag : int
{
	Invalid = -1,
	Straight = 0,
	Bombard = 1,
	Missile = 2,
	Engrave = 3,
	Parabola = 4,
	Tracing = 5
};

enum class TrajectoryCheckReturnType : int
{
	ExecuteGameCheck = 0,
	SkipGameCheck = 1,
	SatisfyGameCheck = 2,
	Detonate = 3
};

enum class TrajectoryFacing : int
{
	Velocity = 0,
	Spin = 1,
	Stable = 2,
	Target = 3,
	Destination = 4,
	FirerBody = 5,
	FirerTurret = 6
};

class PhobosTrajectory;
class PhobosTrajectoryType
{
public:
	PhobosTrajectoryType() :
		Speed { 100.0 }
		, Duration { 0 }
		, TolerantTime { -1 }
		, CreateCapacity { -1 }
		, BulletROT { 0 }
		, BulletFacing { TrajectoryFacing::Velocity }
		, RetargetInterval { 1 }
		, RetargetRadius { 0 }
		, Synchronize { false }
		, MirrorCoord { true }
		, PeacefulVanish {}
		, ApplyRangeModifiers { false }
		, UseDisperseCoord { false }
		, RecordSourceCoord { false }

		, PassDetonate { false }
		, PassDetonateWarhead {}
		, PassDetonateDamage {}
		, PassDetonateDelay { 1 }
		, PassDetonateInitialDelay { 0 }
		, PassDetonateLocal { false }
		, ProximityImpact { 0 }
		, ProximityWarhead {}
		, ProximityDamage {}
		, ProximityRadius { Leptons(179) }
		, ProximityDirect { false }
		, ProximityMedial { false }
		, ProximityAllies { false }
		, ProximityFlight { false }
		, ThroughVehicles { true }
		, ThroughBuilding { true }
		, DamageEdgeAttenuation { 1.0 }
		, DamageCountAttenuation { 1.0 }

		, DisperseWeapons {}
		, DisperseBursts {}
		, DisperseCounts {}
		, DisperseDelays {}
		, DisperseCycle { 0 }
		, DisperseInitialDelay { 0 }
		, DisperseEffectiveRange { Leptons(0) }
		, DisperseSeparate { false }
		, DisperseRetarget { false }
		, DisperseLocation { false }
		, DisperseTendency { false }
		, DisperseHolistic { false }
		, DisperseMarginal { false }
		, DisperseDoRepeat { false }
		, DisperseSuicide { true }
		, DisperseFromFirer {}
		, DisperseFaceCheck { false }
		, DisperseForceFire { true }
		, DisperseCoord { { 0, 0, 0 } }
	{ }

	Valueable<double> Speed; // The speed that a projectile should reach
	Valueable<int> Duration; // The existence time of projectile
	Valueable<int> TolerantTime; // The tolerance time for the projectile to lose its target, after which it will explode
	Valueable<int> CreateCapacity; // Only take effect when the number of trajectory fired by its firer on the map is less than this value
	Valueable<int> BulletROT; // The rotational speed of the projectile image that does not affect the direction of movement
	Valueable<TrajectoryFacing> BulletFacing; // Image facing
	Valueable<int> RetargetInterval; // Wait before attempting to searching for a new target each time we fail to do so
	Valueable<double> RetargetRadius; // Searching for a new target after losing it
	Valueable<bool> Synchronize; // Synchronize the target of its launcher
	Valueable<bool> MirrorCoord; // Should mirror offset
	Nullable<bool> PeacefulVanish; // Disappear directly when about to detonate
	Valueable<bool> ApplyRangeModifiers; // Apply range bonus
	Valueable<bool> UseDisperseCoord; // Use the recorded launch location
	Valueable<bool> RecordSourceCoord; // Record the launch location

	Valueable<bool> PassDetonate; // Detonate the warhead while moving
	Valueable<WarheadTypeClass*> PassDetonateWarhead; // The pass warhead used
	Nullable<int> PassDetonateDamage; // The damage caused by the pass warhead
	Valueable<int> PassDetonateDelay; // Detonation interval
	Valueable<int> PassDetonateInitialDelay; // Detonation initial delay
	Valueable<bool> PassDetonateLocal; // Detonate at ground level
	Valueable<int> ProximityImpact; // How many times can proximity warhead be triggered
	Valueable<WarheadTypeClass*> ProximityWarhead; // The proximity warhead used
	Nullable<int> ProximityDamage; // The damage caused by the proximity warhead
	Valueable<Leptons> ProximityRadius; // How large is the scope of impact
	Valueable<bool> ProximityDirect; // Not detonating the warhead, but directly causing it to receive the damage
	Valueable<bool> ProximityMedial; // When judged as passing through, detonate at bullet position
	Valueable<bool> ProximityAllies; // Does the friendly army accept the judgment
	Valueable<bool> ProximityFlight; // Does the air forces accept the judgment
	Valueable<bool> ThroughVehicles; // Vehicles judged as normal
	Valueable<bool> ThroughBuilding; // Building judged as normal
	Valueable<double> DamageEdgeAttenuation; // The ratio of distance to damage
	Valueable<double> DamageCountAttenuation; // The ratio of count to damage

	ValueableVector<WeaponTypeClass*> DisperseWeapons; // Weapons fired towards the surroundings
	ValueableVector<int> DisperseBursts; // How many times does each weapon burst
	ValueableVector<int> DisperseCounts; // How many times does each group fire
	ValueableVector<int> DisperseDelays; // Cooling time after weapon launch
	Valueable<int> DisperseCycle; // How many rounds of weapons can be fired
	Valueable<int> DisperseInitialDelay; // How long will it take to start firing weapons
	Valueable<Leptons> DisperseEffectiveRange; // How close should it get before start firing weapons
	Valueable<bool> DisperseSeparate; // Launch by weapon or by group
	Valueable<bool> DisperseRetarget; // Automatically research for targets
	Valueable<bool> DisperseLocation; // Where to search for the enemy from
	Valueable<bool> DisperseTendency; // The every first weapon will attack the original target
	Valueable<bool> DisperseHolistic; // Can select targets from different locations
	Valueable<bool> DisperseMarginal; // Can attack trees, stones, bullets, etc
	Valueable<bool> DisperseDoRepeat; // Can repeatedly attack a same target
	Valueable<bool> DisperseSuicide; // Self destruct after all weapons are launched
	Nullable<bool> DisperseFromFirer; // Fire from the firer's position
	Valueable<bool> DisperseFaceCheck; // Check the orientation before launching the weapon
	Valueable<bool> DisperseForceFire; // Ignore the no target state before launching the weapon
	Valueable<CoordStruct> DisperseCoord; // The firing position when fired from the bullet

	virtual ~PhobosTrajectoryType() noexcept = default;
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	virtual bool Save(PhobosStreamWriter& Stm) const;
	virtual TrajectoryFlag Flag() const { return TrajectoryFlag::Invalid; }
	virtual void Read(CCINIClass* const pINI, const char* pSection);
	[[nodiscard]] virtual std::unique_ptr<PhobosTrajectory> CreateInstance(BulletClass* pBullet) const = 0;

private:
	template <typename T>
	void Serialize(T& Stm);
};

class PhobosTrajectory
{
public:
	PhobosTrajectory() { }
	PhobosTrajectory(PhobosTrajectoryType const* trajType, BulletClass* pBullet) :
		Bullet { pBullet }
		, MovingVelocity { BulletVelocity::Empty }
		, MovingSpeed { 0 }
		, DurationTimer {}
		, TolerantTimer {}
		, RetargetTimer {}
		, FirepowerMult { 1.0 }
		, AttenuationRange { 0 }
		, RemainingDistance { 1 }
		, TargetInTheAir { false }
		, TargetIsTechno { false }
		, NotMainWeapon { false }
		, ShouldDetonate { false }
		, FLHCoord { CoordStruct::Empty }
		, CurrentBurst { 0 }
		, CountOfBurst { 0 }

		, PassDetonateDamage { 0 }
		, PassDetonateTimer {}
		, ProximityImpact { trajType->ProximityImpact }
		, ProximityDamage { 0 }
		, ExtraCheck { nullptr }
		, TheCasualty {}

		, DisperseIndex { 0 }
		, DisperseCount { 0 }
		, DisperseCycle { trajType->DisperseCycle }
		, DisperseTimer {}
	{ }

	BulletClass* Bullet; // Bullet attached to
	BulletVelocity MovingVelocity; // The vector used for calculating speed
	double MovingSpeed; // The current speed value
	CDTimerClass DurationTimer; // Bullet existence timer
	CDTimerClass TolerantTimer; // Target tolerance timer
	CDTimerClass RetargetTimer; // Target searching timer
	double FirepowerMult; // Inherited firepower bonus
	int AttenuationRange; // Maximum range
	int RemainingDistance; // Remaining distance from the self explosion location
	bool TargetInTheAir; // Is the original target the Air Force
	bool TargetIsTechno; // Is the original target a techno type
	bool NotMainWeapon; // Does it ignore the launcher
	bool ShouldDetonate; // Should detonate when checking before and after moving
	CoordStruct FLHCoord; // Launch FLH
	int CurrentBurst; // Current burst index, mirror is required for negative numbers
	int CountOfBurst; // Upper limit of burst counts

	int PassDetonateDamage; // Current damage caused by the pass warhead
	CDTimerClass PassDetonateTimer; // Detonation interval timer
	int ProximityImpact; // How many times can proximity warhead be triggered
	int ProximityDamage; // Current damage caused by the proximity warhead
	TechnoClass* ExtraCheck; // The obstacle, no taken out for use in next frame
	std::map<int, int> TheCasualty; // <UniqueID, Frames>, only for recording existence to check whether have damaged

	int DisperseIndex; // Launch weapon group Index
	int DisperseCount; // Launch weapon group remaining times
	int DisperseCycle; // Launch weapon times remaining rounds
	CDTimerClass DisperseTimer; // Cooling timer for launching weapons

	virtual ~PhobosTrajectory() noexcept = default;
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	virtual bool Save(PhobosStreamWriter& Stm) const;
	virtual TrajectoryFlag Flag() const { return TrajectoryFlag::Invalid; }
	virtual void OnUnlimbo();
	virtual bool OnEarlyUpdate();
	virtual bool OnVelocityCheck();
	virtual void OnVelocityUpdate(BulletVelocity* pSpeed, BulletVelocity* pPosition);
	virtual TrajectoryCheckReturnType OnDetonateUpdate(const CoordStruct& position);
	virtual void OnPreDetonate();
	virtual const PhobosTrajectoryType* GetType() const = 0;
	virtual void OpenFire();
	virtual bool GetCanHitGround() const { return true; }
	virtual CoordStruct GetRetargetCenter() const { return this->Bullet->TargetCoords; }
	virtual void SetBulletNewTarget(AbstractClass* const pTarget);
	virtual bool CalculateBulletVelocity(const double speed);
	virtual void MultiplyBulletVelocity(const double ratio, const bool shouldDetonate);

	static inline double Get2DDistance(const CoordStruct& coords)
	{
		return Point2D { coords.X, coords.Y }.Magnitude();
	}
	static inline double Get2DDistance(const CoordStruct& source, const CoordStruct& target)
	{
		return Point2D { source.X, source.Y }.DistanceFrom(Point2D { target.X, target.Y });
	}
	static inline double Get2DVelocity(const BulletVelocity& velocity)
	{
		return Vector2D<double>{ velocity.X, velocity.Y }.Magnitude();
	}
	static inline double Get2DOpRadian(const CoordStruct& source, const CoordStruct& target)
	{
		return Math::atan2(target.Y - source.Y , target.X - source.X);
	}
	static inline BulletVelocity Coord2Vector(const CoordStruct& coords)
	{
		return BulletVelocity { static_cast<double>(coords.X), static_cast<double>(coords.Y), static_cast<double>(coords.Z) };
	}
	static inline CoordStruct Vector2Coord(const BulletVelocity& velocity)
	{
		return CoordStruct { static_cast<int>(velocity.X), static_cast<int>(velocity.Y), static_cast<int>(velocity.Z) };
	}
	static inline BulletVelocity HorizontalRotate(const CoordStruct& coords, const double radian)
	{
		return BulletVelocity { coords.X * Math::cos(radian) + coords.Y * Math::sin(radian), coords.X * Math::sin(radian) - coords.Y * Math::cos(radian), static_cast<double>(coords.Z) };
	}
	static inline Point2D Coord2Point(const CoordStruct& coords)
	{
		return Point2D { coords.X, coords.Y };
	}
	static inline CoordStruct Point2Coord(const Point2D& point, const int z = 0)
	{
		return CoordStruct { point.X, point.Y, z };
	}
	static inline Point2D PointRotate(const Point2D& point, const double radian)
	{
		return Point2D { static_cast<int>(point.X * Math::cos(radian) + point.Y * Math::sin(radian)), static_cast<int>(point.X * Math::sin(radian) - point.Y * Math::cos(radian)) };
	}
	static inline bool CheckTechnoIsInvalid(const TechnoClass* const pTechno)
	{
		return (!pTechno->IsAlive || !pTechno->IsOnMap || pTechno->InLimbo || pTechno->IsSinking || pTechno->Health <= 0);
	}
	static inline bool CheckWeaponCanTarget(const WeaponTypeExt::ExtData* const pWeaponExt, TechnoClass* const pFirer, TechnoClass* const pTarget)
	{
		return !pWeaponExt || (EnumFunctions::IsTechnoEligible(pTarget, pWeaponExt->CanTarget) && pWeaponExt->HasRequiredAttachedEffects(pTarget, pFirer));
	}
	static inline bool CheckWeaponValidness(HouseClass* const pHouse, const TechnoClass* const pTechno, const CellClass* const pCell, const AffectedHouse flags)
	{
		if (pHouse == pTechno->Owner)
			return (flags & AffectedHouse::Owner) != AffectedHouse::None;
		else if (pHouse->IsAlliedWith(pTechno->Owner) || pTechno->IsDisguisedAs(pHouse))
			return (flags & AffectedHouse::Allies) != AffectedHouse::None;
		else if ((flags & AffectedHouse::Enemies) == AffectedHouse::None)
			return false;

		return pTechno->CloakState != CloakState::Cloaked || pCell->Sensors_InclHouse(pHouse->ArrayIndex);
	}
	static inline void SetNewDamage(int& damage, const double ratio)
	{
		if (damage)
		{
			if (const auto newDamage = static_cast<int>(damage * ratio))
				damage = newDamage;
			else
				damage = Math::sgn(damage);
		}
	}
	static inline TechnoClass* GetSurfaceFirer(TechnoClass* pFirer)
	{
		for (auto pTrans = pFirer; pTrans; pTrans = pTrans->Transporter)
			pFirer = pTrans;

		return pFirer;
	}
	static std::vector<CellStruct> GetCellsInRectangle(const CellStruct bottomStaCell, const CellStruct leftMidCell, const CellStruct rightMidCell, const CellStruct topEndCell);
	static void RotateVector(BulletVelocity& vector, const BulletVelocity& aim, double turningRadian);
	static void RotateAboutTheAxis(BulletVelocity& vector, BulletVelocity& axis, double radian);

	bool OnFacingCheck();
	void OnFacingUpdate();
	bool FireAdditionals();
	void DetonateOnObstacle();
	bool CheckSynchronize();
	bool CheckTolerantTime();

	std::vector<CellClass*> GetCellsInProximityRadius();
	bool CheckThroughAndSubjectInCell(CellClass* pCell, HouseClass* pOwner);
	void CalculateNewDamage();
	void PassWithDetonateAt();
	void PrepareForDetonateAt();
	void ProximityDetonateAt(HouseClass* pOwner, TechnoClass* pTarget);
	int GetTheTrueDamage(int damage, bool self);
	double GetExtraDamageMultiplier();

	bool BulletRetargetTechno();
	void GetTechnoFLHCoord();
	CoordStruct GetWeaponFireCoord(TechnoClass* pTechno);
	bool PrepareDisperseWeapon();
	bool FireDisperseWeapon(TechnoClass* pFirer, const CoordStruct& sourceCoord, HouseClass* pOwner);
	void CreateDisperseBullets(TechnoClass* pTechno, const CoordStruct& sourceCoord, WeaponTypeClass* pWeapon, AbstractClass* pTarget, HouseClass* pOwner, int curBurst, int maxBurst);

private:
	template <typename T>
	void Serialize(T& Stm);
};

/*
* This is a guidance to tell you how to add your trajectory here
* Firstly, we just image the game coordinate into a 2D-plain
*
* ZAxis
*   |			        	 TargetCoord
*   |
*   |
*   |
*   |
*   |  SourceCoord
*   O-------------------------------------XYPlain
*
* Then our problem just turns into:
* Find an equation whose curve just passes both two coord
* And the curve is just your trajectory
*
* Luckily, what we need to implement this is just calculate the velocity during the movement
* So a possible way is to find out the equation and find the derivative of it, which is just the velocity
* And the just code on it!
*
* There is a SampleTrajectory already, you can just copy it and do some edits.
* Following that, you can create a fun trajectory very easily. - secsome
*
*                                           ^*##^
*                *###$                     *##^*#*
*               ##^ $##                  ^##^   ^##
*              ##     ##$               ^##      ^##
*             ##       $#*  ^^^  ^^^^^^$#*         ##    ^$*###################*$^
*            *#^        ^###############$          ^#######*$^    ^#*****#^  ^$*####$^
*           ^#$          $##^  ##*  $##^            ^*$            #*****#       ^#####*^
*           ##                                                     $#####^       *#***####$^
*          ##                                            $###$       ^$^         ^#####* ^###^
*  *#**$  ^#^                                         *###$^                      ^$$$$     *##
*  $$$*#####                                          ^^                                      ##*
*        $#^       ^###*        *$        ####         $*####*^                                ^##
* ^$***$$#*        $####        ##       ^####^       ^**$$$*#^                                  ##^
* $#***$##^         ^$^      $######^      $$                                                     ##^
*       ##                    $^  ^^                                                               ##
*      $#^                                                                                          ##
*      ##                                                                                           $#^
*     ^#$                                                                                            ##
*     *#                                                                                             ^#$
*     ##                                                                                              ##
*     #*                                                                                              $#
*    ^#$                                                                                               ###################*^
*    $#                                                                                                $###^  ^#**#^   #*###*
*    $#                                                                                                $***    ****    **$*##
*    $#                                                                                                $###^   *#*#^   #####$
*    $#                                                                                                *#**##############*$
*    $#                                                                                                #*
*    ^#^                                                                                              ^#^
*     #$                                                                                              *#
*     ##                                                                                              #*
*     *#^                                                                                            *#
*      ##                                                                                            #*
*      $#^                                                                                          ##
*       ##                                                                                         *#^
*       ^##                                                                                       $#$
*        ^##                                                                                     $#*
*          ##                                                                                   $#*
*           ##*                                                                                *#*
*            ^##$                                                                             ##^
*              $##$                                                                         *##
*                $##*^                                                                   ^###^
*                  ^##  ^###**$$$$$$$$$$$   ^$$$$$$$$$$$$$$$$$$****$   *##############  ^##$
*                    #####$$*****#########^##*#########*#**###*****##$##$^$$$$^$$^^^^##*##
*                     ^$^                *##^                       ***               ^$
*
*/

// I removed most part but kept a few so that you know what you are eating
class TrajectoryTypePointer
{
	std::unique_ptr<PhobosTrajectoryType> _ptr {};
public:
	explicit TrajectoryTypePointer(TrajectoryFlag flag);
	explicit TrajectoryTypePointer() { }
	TrajectoryTypePointer(const TrajectoryTypePointer&) = delete;
	TrajectoryTypePointer& operator=(const TrajectoryTypePointer&) = delete;
	void LoadFromINI(CCINIClass* pINI, const char* pSection);
	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;
	[[nodiscard]] PhobosTrajectoryType* operator->() const noexcept { return _ptr.get(); }
	[[nodiscard]] PhobosTrajectoryType* get() const noexcept { return _ptr.get(); }
	operator bool() const noexcept { return _ptr.get() != nullptr; }
};

class TrajectoryPointer
{
	std::unique_ptr<PhobosTrajectory> _ptr;
public:
	TrajectoryPointer(std::nullptr_t) : _ptr { nullptr } { }
	TrajectoryPointer(const TrajectoryPointer&) = delete;
	TrajectoryPointer& operator=(const TrajectoryPointer&) = delete;
	TrajectoryPointer& operator=(std::unique_ptr<PhobosTrajectory>&& ptr) noexcept { _ptr = std::move(ptr); return *this; }
	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;
	[[nodiscard]] PhobosTrajectory* operator->() const noexcept { return _ptr.get(); }
	[[nodiscard]] PhobosTrajectory* get() const noexcept { return _ptr.get(); }
	operator bool() const noexcept { return _ptr.get() != nullptr; }
	operator std::unique_ptr<PhobosTrajectory>() noexcept { return std::move(_ptr); }
};
