/*
	Projectiles
*/

#pragma once

#include <ObjectClass.h>
#include <GeneralStructures.h>
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

	TimerStruct UnknownTimer;
	TimerStruct ArmTimer;
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

	//Array
	static constexpr constant_ptr<DynamicVectorClass<BulletClass*>, 0xA8ED40u> const Array{};

	static constexpr reference<DynamicVectorClass<BulletClass*>, 0x89DE18u> const ScalableBullets{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) R0;

	//IPersistStream
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) R0;

	//Destructor
	virtual ~BulletClass() RX;

	//AbstractClass
	virtual AbstractType WhatAmI() const RT(AbstractType);
	virtual int Size() const R0;

	//BulletClass
	virtual BYTE GetAnimFrame() const R0;
	virtual void SetTarget(AbstractClass* pTarget) RX;
	virtual bool MoveTo(const CoordStruct& nwhere, const VelocityClass& velocity) R0;

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

	static void ApplyRadiationToCell(const CellStruct& cell, int radius, int amount)
		{ JMP_STD(0x46ADE0); }

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

	{ JMP_STD(0x5B20F0); }

	// helpers
	CoordStruct GetBulletTargetCoords() const {
		if(this->Target) {
			return this->Target->GetCoords();
		} else {
			return this->GetCoords();
		}
	}

	//Constructor
protected:
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
	BulletData Data;
	bool Bright;
	DWORD unknown_E4;
	VelocityClass Velocity;
	DWORD unknown_100;
	bool __CourseLocked;
	int __CourseLockedDuration;
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
	CoordStruct SourceCoords;
	CoordStruct TargetCoords;
	CellStruct LastMapCoords;
	int DamageMultiplier;
	AnimClass* NextAnim;
	bool SpawnNextAnim;
	int Range;
};
