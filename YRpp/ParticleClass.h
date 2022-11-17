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
	ColorStruct byteB0;
	int RefCount;
	double ColorSpeedResult;
	CoordStruct GasCoord;
	Vector3D<float> VelocitySmoke;
	CoordStruct CoordStructD8;
	float Velocity;
	CoordStruct gapE8;
	CoordStruct dwordF4;
	CoordStruct FireVelocity;
	Vector3D<float> SparkVelocity;
	Vector3D<float> vector3_118;
	ParticleSystemClass* ParticleSystem;
	short RemainingEC;
	short RemainingDC;
	char StateAIAdvance;
	char CoordChange;
	char StartStateAI;
	char Translucency;
	char byte130;
	char hasremaining;
	char field_132;
	char field_133;
	char field_134;
	char field_135;
	char field_136;
	char field_137;
};

static_assert(sizeof(ParticleClass) == 0x138);