/*
	Projectiles
*/

#pragma once

#include <ObjectTypeClass.h>

//forward declarations
class AnimTypeClass;
class BulletClass;
class ColorScheme;
class CellClass;
class TechnoClass;
class WeaponTypeClass;
class WarheadTypeClass;

class DECLSPEC_UUID("5AF2CE77-0634-11D2-ACA4-006008055BB5")
	NOVTABLE BulletTypeClass : public ObjectTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::BulletType;
	static constexpr inline DWORD vtable = 0x7E4948;

	//Array
	static constexpr constant_ptr<DynamicVectorClass<BulletTypeClass*>, 0xA83C80u> const Array {};

	//static void __fastcall Allocate(const char* pID) {
	//	if (!pID || !*pID || !CRT::strlen(pID) , GameStrings::IsBlank(pID))
	//		return;
	//
	//	GameCreate<BulletTypeClass>(pID);
	//}

	IMPL_Find(BulletTypeClass)

	static BulletTypeClass* __fastcall FindOrAllocate(const char* pID) {
		JMP_STD(0x46C790);
	}

	static int __fastcall FindIndexById(const char* pID) {
		JMP_STD(0x46C440);
	}

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x46C750);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x46C6A0);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x46C730);

	//Destructor
	virtual ~BulletTypeClass() override JMP_THIS(0x46C890);

	//AbstractClass
	//virtual void PointerExpired(AbstractClass* pAbstract, bool removed) override JMP_THIS(0x46C820);
	virtual AbstractType WhatAmI() const override { return AbstractType::BulletType; }
	virtual int Size() const override { return 0x2F8; }

	//AbstractTypeClass
	//ObjectTypeClass
	virtual bool SpawnAtMapCoords(CellStruct* pMapCoords,HouseClass* pOwner) override R0;
	virtual ObjectClass* CreateObject(HouseClass* owner) override R0;

	bool Rotates() const {
		return !this->NoRotate;
	}

	void SetScaledSpawnDelay(int delay) {
		 JMP_THIS(0x46C840);
		//this->ScaledSpawnDelay = delay;
	}

	BulletClass* __fastcall CreateBullet(
		AbstractClass* Target,
		TechnoClass* Owner,
		int Damage,
		WarheadTypeClass *WH,
		int Speed,
		bool Bright)
		{ JMP_STD(0x46B050); }

	//Constructor
	BulletTypeClass(const char* pID) noexcept
		: BulletTypeClass(noinit_t())
	{ JMP_THIS(0x46BBC0); }

protected:
	explicit __forceinline BulletTypeClass(noinit_t) noexcept
		: ObjectTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	bool Airburst;
	bool Floater;
	bool SubjectToCliffs;
	bool SubjectToElevation;
	bool SubjectToWalls;
	bool VeryHigh;
	bool Shadow;
	bool Arcing;
	bool Dropping;
	bool Level;
	bool Inviso;
	bool Proximity;
	bool Ranged; //IsFueled
	bool NoRotate; // actually has opposite meaning of Rotates. false means Rotates=yes. , IsFaceless
	bool Inaccurate;
	bool FlakScatter;
	bool AA;
	bool AG;
	bool Degenerates;
	bool Bouncy;
	bool AnimPalette;
	bool FirersPalette;
	int Cluster;
	WeaponTypeClass* AirburstWeapon;
	WeaponTypeClass* ShrapnelWeapon;
	int ShrapnelCount;
	int DetonationAltitude;
	bool Vertical;
	DWORD field_2C4;  //unused , can be used to store ExtData
	double Elasticity;
	int Acceleration;
	int Color; //not a pointer , but index ! -Otamaa
	AnimTypeClass* Trailer;
	int ROT;
	int CourseLockDuration;
	int SpawnDelay;
	int ScaledSpawnDelay;
	bool Scalable;
	int Arm;
	byte AnimLow;
	byte AnimHigh;
	byte AnimRate;
	bool Flat;
};

static_assert(sizeof(BulletTypeClass) == 0x2F8, "Invalid size.");