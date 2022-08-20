/*
	ParticleSystems
*/

#pragma once

#include <ObjectClass.h>
#include <ParticleSystemTypeClass.h>
#include <ParticleClass.h>

class HouseClass;
class DECLSPEC_UUID("0E272DC8-9C0F-11D1-B709-00A024DDAFD1")
	NOVTABLE ParticleSystemClass : public ObjectClass
{
public:
	static const AbstractType AbsID = AbstractType::ParticleSystem;

	//Static
	static constexpr constant_ptr<DynamicVectorClass<ParticleSystemClass*>, 0xA80208u> const Array{};
	static constexpr reference<ParticleSystemClass*, 0xA8ED78u> Instance{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) R0;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) R0;
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) R0;

	//Destructor
	virtual ~ParticleSystemClass() RX;

	//AbstractClass
	virtual AbstractType WhatAmI() const RT(AbstractType);
	virtual int Size() const R0;

	ParticleClass* SpawnParticle(ParticleTypeClass* pType, CoordStruct* coords)
		{JMP_THIS(0x62E430);}

	ParticleClass* SpawnHeldParticle(CoordStruct* pcoord_a, CoordStruct* pcoord_b)
		{JMP_THIS(0x62E380);}

	ParticleClass* SpawnHeldParticleRandom(CoordStruct* pcoord_a, CoordStruct* pcoord_b ,int nArgs)
		{JMP_THIS(0x62E4C0);}

	//Constructor
	ParticleSystemClass(
		ParticleSystemTypeClass* pParticleSystemType,
		const CoordStruct& coords,
		AbstractClass* pTarget = nullptr,
		ObjectClass* pOwner = nullptr,
		const CoordStruct& targetCoords = CoordStruct::Empty,
		HouseClass* pOwnerHouse = nullptr) noexcept : ParticleSystemClass(noinit_t())
			{ JMP_THIS(0x62DC50); }

protected:
	explicit __forceinline ParticleSystemClass(noinit_t) noexcept
		: ObjectClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	ParticleSystemTypeClass* Type;
	CoordStruct  SpawnDistanceToOwner;
	DynamicVectorClass<ParticleClass*> Particles;
	CoordStruct TargetCoords;
	ObjectClass* Owner;
	AbstractClass* Target; // CellClass or TechnoClass
	int          SpawnFrames; //from ParSysTypeClass
	int          Lifetime; //from ParSysTypeClass
	int          SparkSpawnFrames; //from ParSysTypeClass
	int          SpotlightRadius; //defaults to 29
	bool         TimeToDie;
	bool         unknown_bool_F9;
	HouseClass*  OwnerHouse;
};

//static_assert(sizeof(ParticleSystemClass) == 0x100);
