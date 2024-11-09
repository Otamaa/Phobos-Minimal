/*
; **************************************************************************
; ************************ Particle Systems ********************************
; **************************************************************************
; *** Particle Systems ***
;
; HoldsWhat = type of particle (see below) that this system manages (required)
; Spawns = does this system spawn particles by itself (def = no)
; SpawnFrames = number of frames to wait before spawning another particle
; ParticleCap = maximum number of particles that can be in this system

; this is the global psych gas system
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
	static constexpr inline DWORD vtable = 0x7F00A8;
	static constexpr reference<TypeList<ParticleSystemTypeClass*>, 0x7F4F9Cu> const TypeListArray{};
	static constexpr reference<const char*, 0x836EE0, 5u> const BehavesString {};

	static ParticleSystemTypeBehavesLike __fastcall GetBehave(const char* behaveID);// JMP_STD(0x644850);
	static const char* GetBehaveName(ParticleSystemTypeBehavesLike nBhv) { return BehavesString[(int)nBhv]; }


	//Array
	static constexpr constant_ptr<DynamicVectorClass<ParticleSystemTypeClass*>, 0xA83D68u> const Array {};

	IMPL_Find(ParticleSystemTypeClass)

	static ParticleSystemTypeClass* __fastcall FindOrAllocate(const char* pID)
		;//{ JMP_STD(0x644890); }

	IMPL_FindIndexById(ParticleSystemTypeClass)

	static int __fastcall FindIndexByIdOrAllocate(const char* pID)
		;//{ JMP_STD(0x644630); }

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override;//JMP_STD(0x6447A0);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override;//JMP_STD(0x6447E0);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override;//JMP_STD(0x644830);

	//Destructor
	virtual ~ParticleSystemTypeClass() override;

	//AbstractClass
	virtual AbstractType WhatAmI() const override { return AbstractType::ParticleSystemType; }
	virtual int Size() const override { return 0x310; }

	//AbstractTypeClass
	virtual bool LoadFromINI(CCINIClass* pINI) override;//JMP_THIS(0x6442D0);

	//ObjectTypeClass
	virtual bool SpawnAtMapCoords(CellStruct* pMapCoords, HouseClass* pOwner) override { return false; }
	virtual ObjectClass* CreateObject(HouseClass* owner) override { return nullptr; }

	static ParticleSystemTypeBehavesLike __fastcall BehavesFromString(const char* pStr)
		;//{ JMP_STD(0x644850); }

	//Constructor
	ParticleSystemTypeClass(const char* pID);

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
	ParticleSystemTypeBehavesLike BehavesLike;
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

static_assert(sizeof(ParticleSystemTypeClass) == 0x310);
