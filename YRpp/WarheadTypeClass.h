/*
	Warheads
*/

#pragma once

#include <AbstractTypeClass.h>

//forward declarations
class AnimTypeClass;
class ParticleSystemTypeClass;
class VoxelAnimTypeClass;

class DECLSPEC_UUID("A8C54DA4-0F7B-11D2-8172-006008055BB5")
	NOVTABLE WarheadTypeClass : public AbstractTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::WarheadType;
	static COMPILETIMEEVAL OPTIONALINLINE DWORD vtable = 0x7F6B30;

	//Array
	static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<WarheadTypeClass*>, 0x8874C0u> const Array {};

	IMPL_Find(WarheadTypeClass)

	static WarheadTypeClass* __fastcall FindOrAllocate(const char* pID) {
		JMP_STD(0x75E3B0);
	}

	static int __fastcall FindIndexById(const char* pID) {
		JMP_STD(0x75E4A0);
	}

	static int __fastcall FindIndexByIdOrAllocate(const char* pID) {
		JMP_STD(0x40F510);
	}

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x75E080);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x75E0C0);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x75E2C0);

	//Destructor
	virtual ~WarheadTypeClass() JMP_THIS(0x75E510);

	//AbstractClass
	virtual void PointerExpired(AbstractClass* pAbstract, bool bremoved) override JMP_THIS(0x75E440);
	virtual AbstractType WhatAmI() const override RT(AbstractType);
	virtual int Size() const R0;

	//AbstractTypeClass
	virtual bool LoadFromINI(CCINIClass* pINI) override JMP_THIS(0x75D3A0);

	//Constructor
	WarheadTypeClass(const char* pID)
		: WarheadTypeClass(noinit_t())
	{ JMP_THIS(0x75CEC0); }

protected:
	explicit __forceinline WarheadTypeClass(noinit_t)
		: AbstractTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	double  Deform;

	double Verses [0xB];

	double  ProneDamage;
	int     DeformTreshold;

	TypeList<AnimTypeClass*> AnimList;

	InfDeath InfDeath;
	float   CellSpread;
	float   CellInset;
	float   PercentAtMax;
	bool    CausesDelayKill;
	int     DelayKillFrames;
	float   DelayKillAtMax;
	float   CombatLightSize;
	ParticleSystemTypeClass* Particle;
	bool    Wall;
	bool    WallAbsoluteDestroyer;
	bool    PenetratesBunker;
	bool    Wood;
	bool    Tiberium;
	bool    IsOrganic; //149
	bool    Sparky;
	bool    Sonic;
	bool    Fire;
	bool    Conventional;
	bool    Rocker;
	bool    DirectRocker;
	bool    Bright;
	bool    CLDisableRed;
	bool    CLDisableGreen;
	bool    CLDisableBlue;
	bool    EMEffect;
	bool    MindControl;
	bool    Poison;
	bool    IvanBomb;
	bool    ElectricAssault;
	bool    Parasite;
	bool    Temporal;
	bool    IsLocomotor;
	_GUID   Locomotor;
	bool    Airstrike;
	bool    Psychedelic;
	bool    BombDisarm;
	int     Paralyzes;
	bool    Culling;
	bool    MakesDisguise;
	bool    NukeMaker;
	bool    Radiation;
	bool    PsychicDamage;
	bool    AffectsAllies;
	bool    Bullets;
	bool    Veinhole;
	int     ShakeXlo;
	int     ShakeXhi;
	int     ShakeYlo;
	int     ShakeYhi;

	TypeList<VoxelAnimTypeClass*> DebrisTypes;
	TypeList<int> DebrisMaximums;

	int     MaxDebris;
	int     MinDebris;
	DWORD unused_1CC; //Unused
};
static_assert(sizeof(WarheadTypeClass) == 0x1D0, "Invalid size.");
