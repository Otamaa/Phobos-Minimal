/*
	Voxel Animations
*/

#pragma once

#include <ObjectClass.h>
#include <VoxelAnimTypeClass.h>
#include <BounceClass.h>

//forward declarations
class HouseClass;
class ParticleSystemClass;

class DECLSPEC_UUID("0E272DC1-9C0F-11D1-B709-00A024DDAFD1")
	NOVTABLE VoxelAnimClass : public ObjectClass
{
public:
	static const AbstractType AbsID = AbstractType::VoxelAnim;
	static constexpr inline DWORD vtable = 0x7F6318;

	//Static
	static constexpr constant_ptr<DynamicVectorClass<VoxelAnimClass*>, 0x887388u> const Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) R0;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm);//JMP_STD(0x74A970);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty);//JMP_STD(0x74AA10);

	//Destructor
	virtual ~VoxelAnimClass() RX;

	//AbstractClass
	virtual AbstractType WhatAmI() const override RT(AbstractType);
	virtual int	Size() const override R0;

	//ObjectClass
	//VoxelAnimClass
	VoxelAnimTypeClass* GetVoxelAnimType() const
		;//{ JMP_THIS(0x74AB30); }

	//Constructor
	VoxelAnimClass(VoxelAnimTypeClass* pVoxelAnimType, CoordStruct* pLocation, HouseClass* pOwnerHouse);
	VoxelAnimClass(VoxelAnimTypeClass* pVoxelAnimType, const CoordStruct& Location, HouseClass* pOwnerHouse);

public:

	DWORD unused_AC;
	BounceClass Bounce;
	int unknown_int_100;
	VoxelAnimTypeClass* Type;
	ParticleSystemClass* AttachedSystem;
	HouseClass* OwnerHouse;
	bool TimeToDie; // remove on next update
	BYTE unused_111[3];
	AudioController Audio3;
	AudioController Audio4;
	bool Invisible; // don't draw, but Update state anyway
	BYTE unused_13D[3];
	int Duration; // counting down to zero
	DWORD unused_144;
};

static_assert(sizeof(VoxelAnimClass) == 0x148, "Invalid size.");