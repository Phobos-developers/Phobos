#pragma once

#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Savegame.h>

#include <BulletClass.h>

class BulletTypeExt;

enum class TrajectoryFlag : int
{
	Invalid = -1,
	Straight = 0,
	Bombard = 1,
	Disperse = 2
};

enum class TrajectoryCheckReturnType : int
{
	ExecuteGameCheck = 0,
	SkipGameCheck = 1,
	SatisfyGameCheck = 2,
	Detonate = 3
};
class PhobosTrajectory;
class PhobosTrajectoryType
{
public:
	PhobosTrajectoryType() { }
	PhobosTrajectoryType(TrajectoryFlag flag) : Flag { flag }, Trajectory_Speed { 100.0 }
	{ }

	virtual ~PhobosTrajectoryType() noexcept = default;
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	virtual bool Save(PhobosStreamWriter& Stm) const;

	virtual void Read(CCINIClass* const pINI, const char* pSection) = 0;
	virtual std::unique_ptr<PhobosTrajectory> CreateInstance() const = 0;

	TrajectoryFlag Flag;
	Valueable<double> Trajectory_Speed;
};

class PhobosTrajectory
{
public:
	PhobosTrajectory(noinit_t) { }
	PhobosTrajectory(TrajectoryFlag flag, double speed = 100.0) : Flag { flag }, Speed { speed }
	{ }

	virtual ~PhobosTrajectory() noexcept = default;
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	virtual bool Save(PhobosStreamWriter& Stm) const;

	virtual void OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity) = 0;
	virtual bool OnAI(BulletClass* pBullet) = 0;
	virtual void OnAIPreDetonate(BulletClass* pBullet) = 0;
	virtual void OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition) = 0;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(BulletClass* pBullet) = 0;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno) = 0;

	TrajectoryFlag Flag;
	double Speed;
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
