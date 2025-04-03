/*
	Weapons
*/

#pragma once

#include <AbstractTypeClass.h>
#include <OverlayTypeClass.h>
#include <WarheadTypeClass.h>
#include <Leptons.h>

//forward declarations
class AnimTypeClass;
class BulletTypeClass;
class ParticleSystemTypeClass;

class DECLSPEC_UUID("9FD219CA-0F7B-11D2-8172-006008055BB5")
	NOVTABLE WeaponTypeClass : public AbstractTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::WeaponType;
	static COMPILETIMEEVAL OPTIONALINLINE DWORD vtable = 0x7F73B8;

	//Array
	static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<WeaponTypeClass*>, 0x887568u> const Array {};

	IMPL_Find(WeaponTypeClass)

	static WeaponTypeClass* __fastcall FindOrAllocate(const char* pID) {
		JMP_STD(0x772FA0);
	}

	static int __fastcall FindIndexById(const char* pID) {
		JMP_STD(0x773030);
	}

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x772C90);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x772CD0);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x772EB0);

	//Destructor
	virtual ~WeaponTypeClass() override JMP_THIS(0x7730F0);

	//AbstractClass
	virtual AbstractType WhatAmI() const override RT(AbstractType);
	virtual int Size() const override R0;

	//AbstractTypeClass
	virtual bool LoadFromINI(CCINIClass* pINI) override JMP_THIS(0x772080);

	void CalculateSpeed() const
		{ JMP_THIS(0x7729F0); }

	int GetWeaponSpeed(int range) const
	{ JMP_THIS(0x773070); }

	// using dll sqrt and int rounding , be carefull
	int GetWeaponSpeed(CoordStruct const& sourcePos, CoordStruct const& targetPos) const {
		return GetWeaponSpeed((int)sourcePos.DistanceFrom(targetPos));
	}

	bool IsWallDestroyer(OverlayTypeClass* pWhat) const
	{
		if (Warhead) {
			if ((Warhead->Wall || Warhead->WallAbsoluteDestroyer) && pWhat->Wall) {
				if(Warhead->Wood)
					return pWhat->Armor == Armor::Wood;

				return true;
			}
		}

		return false;
	}

	bool IsWallDestoyer() const { JMP_THIS(0x772AC0); }

	ThreatType EligibleTarget() const
		{ JMP_THIS(0x772A90); }

	ThreatType AllowedThreats() const
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
	Leptons Range; // int(256 * ini value)
	Leptons MinimumRange; // int(256 * ini value)
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
