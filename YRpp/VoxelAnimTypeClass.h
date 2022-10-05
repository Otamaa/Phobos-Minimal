/*
	VoxelAnimTypes are initialized by INI files.
*/

#pragma once

#include <ObjectTypeClass.h>

//forward declarations
class AnimTypeClass;
class ParticleSystemTypeClass;
class WarheadTypeClass;

class DECLSPEC_UUID("2EBB6D66-0D4D-11D2-8172-006008055BB5")
	NOVTABLE VoxelAnimTypeClass : public ObjectTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::VoxelAnimType;

	//Array
	ABSTRACTTYPE_ARRAY(VoxelAnimTypeClass, 0xA8EB28u);

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x74B7D0);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x74B810);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x74B8D0);

	//Destructor
	virtual ~VoxelAnimTypeClass() override JMP_THIS(0x74BA30);

	//AbstractClass
	virtual void PointerExpired(AbstractClass* pAbstract, bool removed) override JMP_THIS(0x74B8F0);
	virtual AbstractType WhatAmI() const RT(AbstractType);
	virtual int	Size() const R0;

	//AbstractTypeClass
	virtual bool LoadFromINI(CCINIClass* pINI) override JMP_THIS(0x74B050);

	//ObjectTypeClass
	virtual bool SpawnAtMapCoords(CellStruct* pMapCoords, HouseClass* pOwner) R0;
	virtual ObjectClass* CreateObject(HouseClass* owner) R0; // ! this just returns NULL instead of creating the anim, fucking slackers

	//VoxelAnimTypeClass


	//Constructor
	VoxelAnimTypeClass(const char* pID)
		: VoxelAnimTypeClass(noinit_t())
	{ JMP_THIS(0x74AD80); }

protected:
	explicit __forceinline VoxelAnimTypeClass(noinit_t)
		: ObjectTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	bool Normalized;
	bool Translucent;
	bool SourceShared;
	PROTECTED_PROPERTY(BYTE, unused_297);
	int VoxelIndex;
	int Duration;
	double Elasticity;
	double MinAngularVelocity;
	double MaxAngularVelocity;
	double MinZVel;
	double MaxZVel;
	double MaxXYVel;
	bool IsMeteor;
	PROTECTED_PROPERTY(BYTE, unused_2D1[3]);
	VoxelAnimTypeClass* Spawns;
	int SpawnCount;
	int StartSound;
	int StopSound;
	AnimTypeClass* BounceAnim;
	AnimTypeClass* ExpireAnim;
	AnimTypeClass* TrailerAnim;
	int Damage;
	int DamageRadius;
	WarheadTypeClass* Warhead;
	ParticleSystemTypeClass* AttachedSystem;
	bool IsTiberium;
	PROTECTED_PROPERTY(BYTE, unused_301[3]);
	PROTECTED_PROPERTY(DWORD, align_304);
};

static_assert(sizeof(VoxelAnimTypeClass) == 0x308, "Invalid size.");