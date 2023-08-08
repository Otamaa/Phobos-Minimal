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
	static constexpr inline DWORD vtable = 0x7EF954;

	//Static
	static constexpr constant_ptr<DynamicVectorClass<ParticleClass*>, 0xA83DC8u> const Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x62D930);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x62D7A0);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x62D810);

	//Destructor
	virtual ~ParticleClass() override JMP_THIS(0x62D9A0);

	//AbstractClass
	virtual AbstractType WhatAmI() const override { return AbstractType::Particle; }
	virtual int Size() const override { return 0x138; }

	virtual Layer InWhichLayer() const override JMP_THIS(0x62D770);
	virtual CellStruct const* GetFoundationData(bool includeBib = false) const override JMP_THIS(0x62D710);
	virtual bool UpdatePlacement(PlacementType value) override JMP_THIS(0x62D6F0);
	
	//ParticleClass
	virtual int vt_entry_1E8() JMP_THIS(0x62D830);

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
	DECLARE_PROPERTY(ColorStruct, byteB0);
	PROTECTED_PROPERTY(BYTE, align_B3);
	int RefCount;
	double ColorSpeedResult;
	DECLARE_PROPERTY(CoordStruct, GasCoord);
	DECLARE_PROPERTY(Vector3D<float>, VelocitySmoke);
	DECLARE_PROPERTY(CoordStruct, CoordStructD8);
	float Velocity;
	DECLARE_PROPERTY(CoordStruct, gapE8);
	DECLARE_PROPERTY(CoordStruct, dwordF4);
	DECLARE_PROPERTY(CoordStruct, Fire100);
	DECLARE_PROPERTY(Vector3D<float>, Spark10C);
	DECLARE_PROPERTY(Vector3D<float>, vector3_118);
	ParticleSystemClass* ParticleSystem;
	short RemainingEC;
	short RemainingDC;
	BYTE StateAIAdvance;
	BYTE CoordChange;
	BYTE StartStateAI;
	BYTE Translucency;
	BYTE byte130;
	BYTE hasremaining;
	BYTE field_132;
	BYTE field_133;
	PROTECTED_PROPERTY(DWORD, align_134);

};

static_assert(sizeof(ParticleClass) == 0x138);