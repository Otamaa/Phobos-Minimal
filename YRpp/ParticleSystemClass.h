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
	static constexpr inline DWORD vtable = 0x7EFB9C;

	//Static
	static constexpr constant_ptr<DynamicVectorClass<ParticleSystemClass*>, 0xA80208u> const Array{};
	static constexpr reference<ParticleSystemClass*, 0xA8ED78u> Instance{};

	static constexpr reference<int, 0x836704u, (size_t)FacingType::Count> const FireWind_X {};
	static constexpr reference<int, 0x836724u, (size_t)FacingType::Count> const FireWind_Y {};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x6301A0);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x62FF20);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x630090);

	//Destructor
	virtual ~ParticleSystemClass() override JMP_THIS(0x630230);

	//AbstractClass
	virtual void PointerExpired(AbstractClass* pAbstract, bool removed) override JMP_THIS(0x62FE90);
	virtual AbstractType WhatAmI() const override RT(AbstractType);
	virtual int Size() const override R0;
	virtual void Update() override JMP_THIS(0x62FD60);

	//ObjectClass
	virtual Layer InWhichLayer() const override JMP_THIS(0x62FE80);
	virtual void UnInit() override JMP_THIS(0x6301E0);

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

	ParticleSystemClass(
		ParticleSystemTypeClass* pParticleSystemType,
		CoordStruct* coords, 
		AbstractClass* pTarget,
		ObjectClass* pOwner,
		CoordStruct* targetCoords,
		HouseClass* pOwnerHouse) noexcept : ParticleSystemClass(noinit_t())
	{
		JMP_THIS(0x62DC50);
	}

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
	float          SpawnFrames; //from ParSysTypeClass
	int          Lifetime; //from ParSysTypeClass
	int          SparkSpawnFrames; //from ParSysTypeClass
	int          SpotlightRadius; //defaults to 29
	bool         TimeToDie;
	bool         unknown_bool_F9;
	HouseClass*  OwnerHouse;
};

static_assert(sizeof(ParticleSystemClass) == 0x100);

struct UninitAttachedSystem
{
	void operator() (ParticleSystemClass* pAnim) const
	{
		if (pAnim && pAnim->IsAlive)
		{
			pAnim->Owner = nullptr;
			pAnim->UnInit();
		}

		pAnim = nullptr;
	}
};
