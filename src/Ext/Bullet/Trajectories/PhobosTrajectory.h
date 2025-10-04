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

enum class TrajectoryFacing : unsigned char
{
	Velocity = 0,
	Spin = 1,
	Stable = 2,
	Target = 3,
	Destination = 4,
	FirerBody = 5,
	FirerTurret = 6
};

enum class TrajectoryStatus : unsigned char
{
	None = 0x0,
	Detonate = 0x1,
	Vanish = 0x2,
	Bounce = 0x4
};
MAKE_ENUM_FLAGS(TrajectoryStatus);

class PhobosTrajectory;
class PhobosTrajectoryType
{
public:
	PhobosTrajectoryType() :
		Speed { 100.0 }
		, BulletROT { 0 }
		, BulletFacing { TrajectoryFacing::Velocity }
		, BulletFacingOnPlane { false }
		, MirrorCoord { true }
		, Ranged { false }
	{ }

	Valueable<double> Speed; // The speed that a projectile should reach
	Valueable<int> BulletROT; // The rotational speed of the projectile image that does not affect the direction of movement
	Valueable<TrajectoryFacing> BulletFacing; // Image facing
	Valueable<bool> BulletFacingOnPlane; // Image facing only on horizontal plane
	Valueable<bool> MirrorCoord; // Should mirror offset
	bool Ranged; // Auto set

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
	static constexpr double LowSpeedOffset = 32.0;

	PhobosTrajectory() { }
	PhobosTrajectory(PhobosTrajectoryType const* pTrajType, BulletClass* pBullet) :
		Bullet { pBullet }
		, MovingVelocity { BulletVelocity::Empty }
		, MovingSpeed { 0 }
		, RemainingDistance { 1 }
		, CurrentBurst { 0 }
	{ }

	BulletClass* Bullet; // Bullet attached to
	BulletVelocity MovingVelocity; // The vector used for calculating speed
	double MovingSpeed; // The current speed value
	int RemainingDistance; // Remaining distance from the self explosion location
	int CurrentBurst; // Current burst index, mirror is required for negative numbers

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
	virtual void SetBulletNewTarget(AbstractClass* const pTarget);
	virtual bool CalculateBulletVelocity(const double speed);
	virtual void MultiplyBulletVelocity(const double ratio, const bool shouldDetonate);

	static void RotateVector(BulletVelocity& vector, const BulletVelocity& aim, const double turningRadian);
	static void RotateAboutTheAxis(BulletVelocity& vector, BulletVelocity& axis, const double radian);

	void OnFacingUpdate();

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
