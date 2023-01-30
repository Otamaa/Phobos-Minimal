/*
	ParticleTypes are initialized by INI files.
*/

#pragma once

#include <ObjectTypeClass.h>
#include <CoordStruct.h>
#include <ColorStruct.h>

//forward declarations
class WarheadTypeClass;

class DECLSPEC_UUID("703E044B-0FB1-11D2-8172-006008055BB5")
	NOVTABLE ParticleTypeClass : public ObjectTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::ParticleType;

	//Array
	static constexpr constant_ptr<DynamicVectorClass<ParticleTypeClass*>, 0xA83D98u> const Array {};

	static NOINLINE ParticleTypeClass* __fastcall Find(const char* pID)
	{
		for (auto pItem : *Array){
			if (!CRT::strcmpi(pItem->ID, pID))
				return pItem;
		}

		return nullptr;
	}

	static ParticleTypeClass* __fastcall FindOrAllocate(const char* pID) {
		JMP_STD(0x645820);
	}

	static int __fastcall FindIndexById(const char* pID) {
		JMP_STD(0x645430);
	}

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x645620);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x645660);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x6457A0);

	//Destructor
	virtual ~ParticleTypeClass() override JMP_THIS(0x645950);

	//AbstractClass
	virtual void PointerExpired(AbstractClass* pAbstract, bool removed) override JMP_THIS(0x6458B0);
	virtual AbstractType WhatAmI() const override { return AbstractType::ParticleType; }
	virtual int Size() const override { return 0x318; }

	//AbstractTypeClass
	virtual bool LoadFromINI(CCINIClass* pINI) override JMP_THIS(0x644F50);

	//ObjectTypeClass
	virtual bool SpawnAtMapCoords(CellStruct* cell, HouseClass* pOwner) override JMP_THIS(0x645930); //return false
	virtual ObjectClass* CreateObject(HouseClass* owner) override JMP_THIS(0x645940);	//return nullptr

	//Constructor
	ParticleTypeClass(const char* pID) noexcept
		: ParticleTypeClass(noinit_t())
	{ JMP_THIS(0x644BE0); }

protected:
	explicit __forceinline ParticleTypeClass(noinit_t) noexcept
		: ObjectTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	CoordStruct NextParticleOffset;
	int    XVelocity;
	int    YVelocity;
	int    MinZVelocity;
	int    ZVelocityRange;
	double ColorSpeed;
	TypeList<RGBClass*> ColorList; //Was TypeList
	ColorStruct StartColor1;
	ColorStruct StartColor2;
	int    MaxDC;
	int    MaxEC;
	WarheadTypeClass* Warhead;
	int    Damage;
	int    StartFrame;
	int    NumLoopFrames;
	int    Translucency;
	int    WindEffect;
	float  Velocity;
	float  Deacc;
	int    Radius;
	bool   DeleteOnStateLimit;
	BYTE   EndStateAI;
	BYTE   StartStateAI;
	BYTE   StateAIAdvance;
	BYTE   FinalDamageState;
	BYTE   Translucent25State;
	BYTE   Translucent50State;
	bool   Normalized;
	ParticleTypeClass* NextParticle;
	BehavesLike BehavesLike;

};

static_assert(sizeof(ParticleTypeClass) == 0x318, "Invalid Size ! ");