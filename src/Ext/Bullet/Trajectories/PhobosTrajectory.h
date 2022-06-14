#pragma once

#include <Ext/WeaponType/Body.h> //for weaponTypeExt::ExtData
#include <Ext/Bullet/Body.h>

#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Savegame.h>

#include <BulletClass.h>

enum class TrajectoryFlag : int
{
	Invalid = -1,
	Straight = 0,
	Bombard = 1,
};

enum class TrajectoryCheckReturnType : int
{
	ExecuteGameCheck = 0,
	SkipGameCheck = 1,
	SatisfyGameCheck = 2,
	Detonate = 3
};

class PhobosTrajectoryType
{
public:
	PhobosTrajectoryType(noinit_t) { }
	PhobosTrajectoryType(TrajectoryFlag flag) : Flag { flag } { }

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	virtual bool Save(PhobosStreamWriter& Stm) const;

	virtual void Read(CCINIClass* const pINI, const char* pSection) = 0;

	static void CreateType(PhobosTrajectoryType*& pType, CCINIClass* const pINI, const char* pSection, const char* pKey);

	static PhobosTrajectoryType* LoadFromStream(PhobosStreamReader& Stm);
	static void WriteToStream(PhobosStreamWriter& Stm, PhobosTrajectoryType* pType);
	static PhobosTrajectoryType* ProcessFromStream(PhobosStreamReader& Stm, PhobosTrajectoryType* pType);
	static PhobosTrajectoryType* ProcessFromStream(PhobosStreamWriter& Stm, PhobosTrajectoryType* pType);

	TrajectoryFlag Flag;
};


template<typename T>
concept TrajectoryType = std::is_base_of<PhobosTrajectoryType, T>::value;

class PhobosTrajectory
{
public:
	const BulletExt::ExtData* BulletExtData { nullptr };

	PhobosTrajectory(noinit_t) { }
	PhobosTrajectory(TrajectoryFlag flag) : Flag { flag }
		, BulletExtData { nullptr }
	{ }

	PhobosTrajectory(TrajectoryFlag flag , const BulletExt::ExtData* pData) : Flag { flag }
		, BulletExtData { pData }
	{ }

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	virtual bool Save(PhobosStreamWriter& Stm) const;

	virtual void OnUnlimbo(CoordStruct* pCoord, BulletVelocity* pVelocity) = 0;
	virtual bool OnAI() = 0;
	virtual void OnAIPreDetonate() = 0;
	virtual void OnAIVelocity(BulletVelocity* pSpeed, BulletVelocity* pPosition) = 0;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(CoordStruct coords) = 0;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(TechnoClass* pTechno) = 0;

	template<TrajectoryType T>
	T* GetTrajectoryType() const
	{
		if (!BulletExtData || !BulletExtData->TypeExt) {
			Debug::FatalErrorAndExit("GetTrajectoryType Failed ! , Missing Pointer ! \n");
			return nullptr;
		}

		if (BulletExtData->TypeExt->TrajectoryType->Flag != Flag) {
			Debug::FatalErrorAndExit("GetTrajectoryType Failed ! ,  Tryng to Cast From Different Flag! \n");
			return nullptr;
		}

		return static_cast<T*>(BulletExtData->TypeExt->TrajectoryType);
	}

	double GetTrajectorySpeed(BulletClass* pBullet) const;

	static PhobosTrajectory* CreateInstance(PhobosTrajectoryType* pType, BulletExt::ExtData* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity);

	static PhobosTrajectory* LoadFromStream(PhobosStreamReader& Stm);
	static void WriteToStream(PhobosStreamWriter& Stm, PhobosTrajectory* pTraj);
	static PhobosTrajectory* ProcessFromStream(PhobosStreamReader& Stm, PhobosTrajectory* pTraj);
	static PhobosTrajectory* ProcessFromStream(PhobosStreamWriter& Stm, PhobosTrajectory* pTraj);

	TrajectoryFlag Flag { TrajectoryFlag::Invalid };
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