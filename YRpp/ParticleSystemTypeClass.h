/*
	ParticleSystemTypes are initialized by INI files.
*/

#pragma once

#include <ObjectTypeClass.h>
#include <ColorStruct.h>
#include <GeneralDefinitions.h>

//forward declarations

class  DECLSPEC_UUID("703E044A-0FB1-11D2-8172-006008055BB5")
	NOVTABLE ParticleSystemTypeClass : public ObjectTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::ParticleSystemType;
	static constexpr reference<TypeList<ParticleSystemTypeClass*>, 0x7F4F9Cu> const TypeListArray{};

	//Array
	ABSTRACTTYPE_ARRAY(ParticleSystemTypeClass, 0xA83D68u);

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) R0;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) R0;
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) R0;

	//Destructor
	virtual ~ParticleSystemTypeClass() RX;

	//AbstractClass
	virtual AbstractType WhatAmI() const RT(AbstractType);
	virtual int Size() const R0;

	//ObjectTypeClass
	virtual bool SpawnAtMapCoords(CellStruct* mcoords, HouseClass* owner) R0;
	virtual ObjectClass* CreateObject(HouseClass* owner) R0;

	//Constructor
	ParticleSystemTypeClass(const char* pID) noexcept
		: ParticleSystemTypeClass(noinit_t())
	{ JMP_THIS(0x6440A0); }

protected:
	explicit __forceinline ParticleSystemTypeClass(noinit_t) noexcept
		: ObjectTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int      HoldsWhat; //ParticleType Array index
	bool     Spawns;
	int      SpawnFrames;
	float    Slowdown;
	int      ParticleCap;
	int      SpawnRadius;
	float    SpawnCutoff;
	float    SpawnTranslucencyCutoff;
	BehavesLike BehavesLike;
	int      Lifetime;
	Vector3D<float> SpawnDirection;
	double   ParticlesPerCoord;
	double   SpiralDeltaPerCoord;
	double   SpiralRadius;
	double   PositionPerturbationCoefficient;
	double   MovementPerturbationCoefficient;
	double   VelocityPerturbationCoefficient;
	double   SpawnSparkPercentage;
	int      SparkSpawnFrames;
	int      LightSize;
	ColorStruct LaserColor;
	bool     Laser;
	bool     OneFrameLight;
};

//static_assert(sizeof(ParticleSystemTypeClass) == 0x310);
