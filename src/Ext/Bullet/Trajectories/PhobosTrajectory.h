#pragma once

//#include <Ext/Bullet/Body.h>

#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

enum class TrajectoryFlag : int
{
	Invalid = -1,
	Straight = 0,
	Bombard = 1,
	Artillery = 2,
	Bounce= 3 ,
	Vertical = 4,
	Meteor = 5,
	Spiral = 6,
	Wave = 7,

	Count
};

enum class TrajectoryCheckReturnType : int
{
	ExecuteGameCheck = 0,
	SkipGameCheck = 1,
	SatisfyGameCheck = 2,
	Detonate = 3
};

class VelocityClass;
class BulletClass;
class PhobosTrajectoryType
{
public:
	TrajectoryFlag Flag { TrajectoryFlag::Invalid };
	Nullable<Leptons> DetonationDistance;

	PhobosTrajectoryType(noinit_t){ }
	PhobosTrajectoryType(TrajectoryFlag flag) : Flag { flag }
		, DetonationDistance { }
	{}

	virtual void InvalidatePointer(void* ptr, bool bRemoved) { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	virtual bool Save(PhobosStreamWriter& Stm) const;
	virtual bool Read(CCINIClass* const pINI, const char* pSection);

	virtual ~PhobosTrajectoryType() = default;
	static void CreateType(std::unique_ptr<PhobosTrajectoryType>& pType, CCINIClass* const pINI, const char* pSection, const char* pKey);

	static void ProcessFromStream(PhobosStreamReader& Stm, std::unique_ptr<PhobosTrajectoryType>& pType);
	static void ProcessFromStream(PhobosStreamWriter& Stm, std::unique_ptr<PhobosTrajectoryType>& pType);

protected :
	static bool UpdateType(std::unique_ptr<PhobosTrajectoryType>& pType , TrajectoryFlag flag);
};


template<typename T>
concept TrajectoryType = std::is_base_of<PhobosTrajectoryType, T>::value;

class PhobosTrajectory
{
public:

	TrajectoryFlag Flag { TrajectoryFlag::Invalid };
	PhobosTrajectoryType* Type;
	Leptons DetonationDistance;

	PhobosTrajectory(noinit_t) { }
	PhobosTrajectory(TrajectoryFlag flag ) : Flag { flag }
		, Type { nullptr }
		, DetonationDistance { 0 }
	{ }

	PhobosTrajectory(TrajectoryFlag flag , PhobosTrajectoryType* type) : Flag { flag }
		, Type { type }
		, DetonationDistance { 0 }
	{ }

	virtual ~PhobosTrajectory() = default;

	virtual void InvalidatePointer(void* ptr, bool bRemoved) { }
	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	virtual bool Save(PhobosStreamWriter& Stm) const;

	virtual PhobosTrajectoryType* GetTrajectoryType() const { return const_cast<PhobosTrajectoryType*>(Type); }

	virtual void OnUnlimbo(BulletClass* pBullet , CoordStruct* pCoord, VelocityClass* pVelocity) = 0;
	virtual bool OnAI(BulletClass* pBullet) = 0;
	virtual void OnAIPreDetonate(BulletClass* pBullet) = 0;
	virtual void OnAIVelocity(BulletClass* pBullet, VelocityClass* pSpeed, VelocityClass* pPosition) = 0;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(BulletClass* pBullet, CoordStruct& coords) = 0;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(BulletClass* pBullet,TechnoClass* pTechno) = 0;

	double GetTrajectorySpeed(BulletClass* pBullet) const;

	static void CreateInstance(BulletClass* pBullet, CoordStruct* pCoord, VelocityClass* pVelocity);

	static void ProcessFromStream(PhobosStreamReader& Stm, std::unique_ptr<PhobosTrajectory>& pTraj);
	static void ProcessFromStream(PhobosStreamWriter& Stm, std::unique_ptr<PhobosTrajectory>& pTraj);

	static void SetInaccurate(BulletClass* pBullet);

protected:
	static bool UpdateType(std::unique_ptr<PhobosTrajectory>& pTraj , PhobosTrajectoryType*  pType, TrajectoryFlag flag);
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