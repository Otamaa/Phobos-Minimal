#pragma once

#include <AbstractClass.h>
#include <Audio.h>

//forward declarations
class ObjectClass;
class TechnoClass;
class HouseClass;

enum class BombState : int { Planted, Removed };
enum class BombType : int { NormalBomb , DeathBomb };

class DECLSPEC_UUID("0679E983-AD9D-11D3-BE16-00104B62A16C")
NOVTABLE BombClass : public AbstractClass
{
public:
	static const AbstractType AbsID = AbstractType::Bomb;
	static constexpr constant_ptr<DynamicVectorClass<BombClass*>, 0x89C668u> const Array { };

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) R0;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) R0;
	virtual HRESULT __stdcall Save(IStream* pStm,BOOL fClearDirty) R0;

	//Destructor
	virtual ~BombClass() RX;

	//AbstractClass
	virtual AbstractType WhatAmI() const RT(AbstractType);
	virtual int	Size() const R0;

	void Detonate()
		{ JMP_THIS(0x438720); }

	void Disarm()
		{ JMP_THIS(0x4389B0); }

	BombType GetBombType() const
		{ JMP_THIS(0x4389F0); }

	int GetCurrentFlickerFrame() const // which frame of the ticking bomb to draw
		{ JMP_THIS(0x438A00); }

	bool TimeToExplode() const
		{ JMP_THIS(0x438A70); }

	//Constructor
	//Bombs have a special constructor that just should not be called like this...
	//See BombListClass::Plant
	BombClass() noexcept
		: BombClass(noinit_t())
	{ JMP_THIS(0x4385D0); }

protected:
	explicit __forceinline BombClass(noinit_t) noexcept
		: AbstractClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	TechnoClass* Owner;		//Most likely Ivan.
	HouseClass* OwnerHouse;
	ObjectClass* Target; // attaching to objects is possible, but it will never detonate
	BombType Type; // unused - if so, [General]CanDetonateDeathBomb applies instead of CanDetonateTimeBomb
	int PlantingFrame;
	int DetonationFrame;
	DECLARE_PROPERTY(AudioController, TickAudioController);
	int TickSound;
	BOOL ShouldPlayTickingSound; // seems so
	BombState State; // (mostly) set to 0 on plant, 1 on detonation/removal ?
};

static_assert(sizeof(BombClass) == 0x5C, "Invalid size.");