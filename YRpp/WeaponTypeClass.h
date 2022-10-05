/*
	Weapons
*/

#pragma once

#include <AbstractTypeClass.h>

//forward declarations
class AnimTypeClass;
class BulletTypeClass;
class ParticleSystemTypeClass;
class WarheadTypeClass;

class DECLSPEC_UUID("9FD219CA-0F7B-11D2-8172-006008055BB5")
	NOVTABLE WeaponTypeClass : public AbstractTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::WeaponType;

	//Array
	ABSTRACTTYPE_ARRAY(WeaponTypeClass, 0x887568u);

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x772C90);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x772CD0);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x772EB0);

	//Destructor
	virtual ~WeaponTypeClass() override JMP_THIS(0x7730F0);

	//AbstractClass
	virtual AbstractType WhatAmI() const RT(AbstractType);
	virtual int Size() const R0;

	//AbstractTypeClass
	virtual bool LoadFromINI(CCINIClass* pINI) override JMP_THIS(0x772080);

	void CalculateSpeed() const
		{ JMP_THIS(0x7729F0); }

    int GetWeaponSpeed(int range) const
    { JMP_THIS(0x773070); }

	int GetWeaponSpeed(CoordStruct const& sourcePos, CoordStruct const& targetPos) const {
		return GetWeaponSpeed(sourcePos.DistanceFromI(targetPos));
	}

	bool IsWallDestroyer() const
	{ JMP_THIS(0x772AC0); }

	TargetFlags EligibleTarget() const
	{ JMP_THIS(0x772A90); }

	//Constructor
	WeaponTypeClass(const char* pID = nullptr)
		: WeaponTypeClass(noinit_t())
	{ JMP_THIS(0x771C70); }

	explicit WeaponTypeClass(IStream* pStm) 
		: WeaponTypeClass(noinit_t())
	{ JMP_THIS(0x771F00); }

protected:
	explicit __forceinline WeaponTypeClass(noinit_t)
		: AbstractTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int AmbientDamage;
	int Burst;
	BulletTypeClass* Projectile;
	int Damage;
	int Speed;
	WarheadTypeClass* Warhead;
	int ROF;
	int Range; // int(256 * ini value)
	int MinimumRange; // int(256 * ini value)
	TypeList<int> Report;		//sound indices
	TypeList<int> DownReport;	//sound indices
	TypeList<AnimTypeClass*> Anim;
	AnimTypeClass* OccupantAnim;
	AnimTypeClass* AssaultAnim;
	AnimTypeClass* OpenToppedAnim;
	ParticleSystemTypeClass* AttachedParticleSystem;
	ColorStruct LaserInnerColor;
	ColorStruct LaserOuterColor;
	ColorStruct LaserOuterSpread;
	bool UseFireParticles;
	bool UseSparkParticles;
	bool OmniFire;
	bool DistributedWeaponFire;
	bool IsRailgun;
	bool Lobber;
	bool Bright;
	bool IsSonic;
	bool Spawner;
	bool LimboLaunch;
	bool DecloakToFire;
	bool CellRangefinding;
	bool FireOnce;
	bool NeverUse;
	bool RevealOnFire;
	bool TerrainFire;
	bool SabotageCursor;
	bool MigAttackCursor;
	bool DisguiseFireOnly;
	int DisguiseFakeBlinkTime;
	bool InfiniteMindControl;
	bool FireWhileMoving;
	bool DrainWeapon;
	bool FireInTransport;
	bool Suicide;
	bool TurboBoost;
	bool Supress;
	bool Camera;
	bool Charges;
	bool IsLaser;
	bool DiskLaser;
	bool IsLine;
	bool IsBigLaser;
	bool IsHouseColor;
	char LaserDuration;
	bool IonSensitive;
	bool AreaFire;
	bool IsElectricBolt;
	bool DrawBoltAsLaser;
	bool IsAlternateColor;
	bool IsRadBeam;
	bool IsRadEruption;
	int RadLevel;
	bool IsMagBeam;
};
static_assert(sizeof(WeaponTypeClass) == 0x160, "Invalid size.");