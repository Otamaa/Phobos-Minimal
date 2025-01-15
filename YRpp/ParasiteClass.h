#pragma once

#include <AbstractClass.h>
#include <CoordStruct.h>

class AnimClass;
class FootClass;

class DECLSPEC_UUID("1D016B81-B24B-11D3-BE16-00104B62A16C")
	NOVTABLE ParasiteClass : public AbstractClass
{
public:
	static const AbstractType AbsID = AbstractType::Parasite;

	static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<ParasiteClass*>, 0xAC4910u> const Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x6296D0);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x6295B0);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x6296B0);

	//Destructor
	virtual ~ParasiteClass() override JMP_THIS(0x62AF70);

	//AbstractClass
	virtual void PointerExpired(AbstractClass* pAbstract, bool bremoved) override JMP_THIS(0x62A260);
	virtual AbstractType WhatAmI() const override { return AbstractType::Parasite; }
	virtual int Size() const override { return 0x58; }
	virtual void Update() override JMP_THIS(0x629FD0);

	//non-virtual
	void UpdateSquid()
		{ JMP_THIS(0x6297F0); }

	bool UpdateGrapple()
		{ JMP_THIS(0x629720); }

	void ExitUnit()
		{ JMP_THIS(0x62A4A0); }

	bool CanInfect(FootClass *pTarget) const
		{ JMP_THIS(0x62A8E0); }

	void TryInfect(FootClass *pTarget)
		{ JMP_THIS(0x62A980); }

	bool CanExistOnVictimCell() const
		{ JMP_THIS(0x62AB40); }

	CoordStruct* DetachFromVictim(CoordStruct* Where) const
		{JMP_THIS(0x62AC30);}

	//Constructor
	ParasiteClass(FootClass* pOwner = nullptr) noexcept
		: ParasiteClass(noinit_t())
	{ JMP_THIS(0x6292B0); }

protected:
	explicit __forceinline ParasiteClass(noinit_t) noexcept
		: AbstractClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	FootClass*      Owner;
	FootClass*      Victim;
	CDTimerClass    SuppressionTimer;
	CDTimerClass    DamageDeliveryTimer;
	AnimClass*      GrappleAnim;
	ParasiteState   GrappleState;
	int             GrappleAnimFrame;
	int             GrappleAnimDelay;
	bool            GrappleAnimGotInvalid;
};
