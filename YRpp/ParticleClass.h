/*
	Particles
*/

#pragma once

#include <ObjectClass.h>
#include <ParticleTypeClass.h>

//forward declarations
class ParticleSystemClass;

class DECLSPEC_UUID("0E272DCC-9C0F-11D1-B709-00A024DDAFD1")
	NOVTABLE ParticleClass : public ObjectClass
{
public:
	static const AbstractType AbsID = AbstractType::Particle;

	//Static
	static constexpr constant_ptr<DynamicVectorClass<ParticleClass*>, 0xA83DC8u> const Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) R0;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) R0;
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) R0;

	//Destructor
	virtual ~ParticleClass() RX;

	//AbstractClass
	virtual AbstractType WhatAmI() const RT(AbstractType);
	virtual int Size() const R0;

	//ParticleClass
	virtual int vt_entry_1E8() R0;

	//Constructor
	ParticleClass(
		ParticleTypeClass* pParticleType, CoordStruct* pCrd1,
		CoordStruct* pCrd2, ParticleSystemClass* pParticleSystem) noexcept
		: ParticleClass(noinit_t())
	{ JMP_THIS(0x62B5E0); }

protected:
	explicit __forceinline ParticleClass(noinit_t) noexcept
		: ObjectClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	ParticleTypeClass* Type;
	ColorStruct Color;
	DWORD  RefCount; //B4
	CoordStruct  unknown_B8;
	Vector3D<float> unknown_C4;
	double unknown_double_D0;
	CoordStruct  unknown_D8;
	float  Velocity;
	CoordStruct Crd2_E8; //Crd2 in CTOR
	CoordStruct Crd1_F4; //Crd1 in CTOR
	CoordStruct unknown_coords_100; //{ 0, 0, 0} in CTOR
	Vector3D<float> unknown_vector3d_10C;
	Vector3D<float> unknown_vector3d_118;
	ParticleSystemClass*   ParticleSystem;
	WORD   RemainingEC;
	WORD   RemainingDC;
	BYTE   StateAIAdvance;
	BYTE   unknown_12D;
	BYTE   StartStateAI;
	BYTE   Translucency;
	BYTE   unknown_130;
	BYTE   hasremaining; //131
	PROTECTED_PROPERTY(DWORD, unused_132); //??
};

static_assert(sizeof(ParticleClass) == 0x138);