/*
	Projectiles
*/

#pragma once

#include <ObjectClass.h>
#include <VelocityClass.h>
#include <BulletTypeClass.h>

class TechnoClass;
class ObjectClass;
class WarheadTypeClass;

enum class Fuse : unsigned int
{ DontIgnite, Ignite, Ignite_DistaceFactor };

struct BulletData
{
	//4E11F0
	Fuse BulletStateCheck(CoordStruct const& Destination) const
	{ JMP_THIS(0x4E11F0); }

	CDTimerClass UnknownTimer;
	CDTimerClass ArmTimer;
	CoordStruct Location;
	int Distance;
};

// the velocities along the axes, or something like that
//using BulletVelocity = Vector3D<double>; // :3 -pd

class DECLSPEC_UUID("0E272DC9-9C0F-11D1-B709-00A024DDAFD1")
	NOVTABLE BulletClass : public ObjectClass
{
public:
	static const AbstractType AbsID = AbstractType::Bullet;
	static OPTIONALINLINE COMPILETIMEEVAL VelocityClass EmptyVelocity = { 0.0 , 0.0 , 0.0 };
	static COMPILETIMEEVAL OPTIONALINLINE DWORD vtable = 0x7E46E4;

	//Array
	static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<BulletClass*>, 0xA8ED40u> const Array{};

	static COMPILETIMEEVAL reference<DynamicVectorClass<BulletClass*>, 0x89DE18u> const ScalableBullets{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override R0;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x46AE70);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x46AFB0);

	//Destructor
	virtual ~BulletClass() RX;

	//AbstractClass
	virtual AbstractType WhatAmI() const override RT(AbstractType);
	virtual int ClassSize() const override R0;
	virtual void PointerExpired(AbstractClass* pAbstract, bool bremoved) override JMP_THIS(0x4684E0);

	//ObjectClass
	//virtual bool Limbo() override { static_assert(true, "BulletLimboCalled"); }

	//BulletClass
	virtual BYTE GetAnimFrame() const JMP_THIS(0x468000);
	virtual void SetTarget(AbstractClass* pTarget) JMP_THIS(0x46B5A0);
	virtual bool MoveTo(const CoordStruct& nwhere, const VelocityClass& velocity) JMP_THIS(0x468670);

	// non-virtual
	// after CoCreateInstance creates a bullet, this configures it
	void Construct (
		BulletTypeClass* pType,
		AbstractClass* pTarget,
		TechnoClass* pOwner,
		int damage,
		WarheadTypeClass* pWarhead,
		int speed,
		bool bright) const
		{ JMP_THIS(0x4664C0); }

	// calls Detonate with the appropriate coords
	void Explode(bool destroy = false) const
		{ JMP_THIS(0x468D80); }

	// detonate the bullet at specific coords
	void Detonate(const CoordStruct& coords) const
		{ JMP_THIS(0x4690B0); }

	// spawns off the proper amount of shrapnel projectiles
	void Shrapnel() const
		{ JMP_THIS(0x46A310); }

	void ApplyRadiationToCell(const CellStruct& cell, int radius, int amount)
		{ JMP_THIS(0x46ADE0); }

	// this bullet will miss and hit the ground instead.
	// if the original target is in air, it will disappear.
	void LoseTarget() const
		{ JMP_THIS(0x468430); }

	bool IsHoming() const
		{ return this->Type->ROT > 0; }

	void SetWeaponType(WeaponTypeClass *weapon)
		{ this->WeaponType = weapon; }

	WeaponTypeClass* GetWeaponType() const
		{ return this->WeaponType; }

	// only called in UnitClass::Fire if Type->Scalable
	void InitScalable()
		{ JMP_THIS(0x46B280); }

	// call only after the target, args, etc., have been set
	void NukeMaker()
		{ JMP_THIS(0x46B310); }

	//468BB0
	bool IsForceToExplode(const CoordStruct& loc)
		{JMP_THIS(0x468BB0);}

	static DirStruct __fastcall ProjectileMotion(
		const CoordStruct& pCoord,
		VelocityClass* pVel,
		const CoordStruct& pSecondCoord,
		DirStruct* pDir,
		bool bInAir,
		bool bAirburs,
		bool bVeryHigh,
		bool bLevel)

	{ JMP_FAST(0x5B20F0); }

	// helpers
	CoordStruct GetBulletTargetCoords() const {
		CoordStruct _coords {};

		if(this->Target) {
			this->Target->GetCoords(&_coords);
		} else {
			this->GetCoords(&_coords);
		}

		return _coords;
	}

	CoordStruct GetDestinationCoords() const {
		//Inviso projectile is snapping onto the target
		return this->Type->Inviso ? this->Location : this->TargetCoords;
	}

	static AnimClass* CreateDamagingBulletAnim(HouseClass* pHouse, CellClass* pTarget, BulletClass* pBullet, AnimTypeClass* pAnimType);

	//Constructor
	BulletClass() noexcept
		: BulletClass(noinit_t())
	{ JMP_THIS(0x466380); }

protected:
	explicit __forceinline BulletClass(noinit_t) noexcept
		: ObjectClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	BulletTypeClass* Type;
	TechnoClass* Owner;
	bool IsInaccurate; //B4
	DECLARE_PROPERTY(BulletData ,Data);
	bool Bright;
	DWORD unknown_E4;
	DECLARE_PROPERTY(VelocityClass ,Velocity);
	DWORD unknown_100;
	bool unknown_104;
	bool CourseLock;
	BYTE padding[2];
	int CourseLockCounter;
	AbstractClass* Target;
	int Speed;
	int InheritedColor;
	DWORD SomeIntIncrement_118;
	DWORD unknown_11C;
	double unknown_120;
	WarheadTypeClass* WH;
	byte AnimFrame;
	byte AnimRateCounter;
	WeaponTypeClass* WeaponType;
	DECLARE_PROPERTY(CoordStruct, SourceCoords);
	DECLARE_PROPERTY(CoordStruct, TargetCoords);
	DECLARE_PROPERTY(CellStruct, LastMapCoords);
	int DamageMultiplier;
	AnimClass* NextAnim;
	bool SpawnNextAnim;
	int Range;
};
static_assert(sizeof(BulletClass) == 0x160, "Invalid size.");