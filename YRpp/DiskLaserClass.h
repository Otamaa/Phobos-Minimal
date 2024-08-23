/*
	DiskLasers are the floating disks' purple lasers.
*/

#pragma once

#include <AbstractClass.h>

class LaserDrawClass;
class TechnoClass;
class WeaponTypeClass;

class DECLSPEC_UUID("5230C9A8-846A-47EC-BDA2-7E95445E1D49")
	NOVTABLE DiskLaserClass : public AbstractClass
{
public:
	static const AbstractType AbsID = AbstractType::DiskLaser;

	// static
	static constexpr constant_ptr<DynamicVectorClass<DiskLaserClass*>, 0x8A0208u> const Array{};
	static inline constexpr int Radius = 240;
	static constexpr reference<Point2D, 0x8A0180u, 16u> const DrawCoords {};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override R0;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override R0;
	virtual HRESULT __stdcall Save(IStream* pStm,BOOL fClearDirty) override R0;

	//Destructor
	virtual ~DiskLaserClass() RX;

	//AbstractClass
	virtual AbstractType WhatAmI() const override RT(AbstractType);
	virtual int Size() const override R0;

	//non-virtual
	void Fire(TechnoClass* pOwner, AbstractClass* pTarget, WeaponTypeClass* pWeapon, int nDamage)
		{ JMP_THIS(0x4A71A0); }

	void PointerGotInvalid(AbstractClass* pInvalid)
		{ JMP_THIS(0x4A7900); }

	//Constructor
	DiskLaserClass() noexcept
		: DiskLaserClass(noinit_t())
	{ JMP_THIS(0x4A7A30); }

protected:
	explicit __forceinline DiskLaserClass(noinit_t) noexcept
		: AbstractClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	TechnoClass* Owner;
	TechnoClass* Target;
	WeaponTypeClass* Weapon;
	int DrawRateCounter;//dec per update
	DWORD Facing;//0-15
	int DrawCounter;//inc per draw
	int Damage;
};
