#pragma once

#include <AbstractClass.h>

//forward declarations
class SuperClass;
class TechnoClass;

//The AirstrikeClass handles the airstrikes Boris calls in.
class DECLSPEC_UUID("94112424-E403-11D3-8E6E-005004AAB2FB")
	NOVTABLE TemporalClass : public AbstractClass
{
public:
	static const AbstractType AbsID = AbstractType::Temporal;
	static constexpr inline DWORD vtable = 0x7F5180;

	//Static
	static constexpr constant_ptr<DynamicVectorClass<TemporalClass*>, 0xB0EC60u> const Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x71A720);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x71A660);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x71A700);

	//Destructor
	virtual ~TemporalClass() override JMP_THIS(0x71A5D0);

	//AbstractClass
	virtual AbstractType WhatAmI() const override { return AbstractType::Temporal; }
	virtual int Size() const override { return 0x50; }

	//non-virtual
	void Fire(TechnoClass* pTarget)
		{ JMP_THIS(0x71AF20); }
	bool CanWarpTarget(TechnoClass* pTarget) const
		{ JMP_THIS(0x71AE50); }

	// hardcoded to accumulate only up to 50 helpers
	int GetWarpPerStep( int nHelperCount = 0 ) const
		{ JMP_THIS(0x71AB10); }

	void LetGo()
		{ JMP_THIS(0x71ABC0); }
	void JustLetGo()
		{ JMP_THIS(0x71AD40); }
	void Detach()
		{ JMP_THIS(0x71ADE0); }

	//Constructor
	TemporalClass(TechnoClass* pOwnerUnit) noexcept
		: TemporalClass(noinit_t())
	{ JMP_THIS(0x71A4E0); }

protected:
	explicit __forceinline TemporalClass(noinit_t) noexcept
		: AbstractClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	TechnoClass*       Owner;
	TechnoClass*       Target;
	CDTimerClass       LifeTimer;
	void*              unknown_pointer_38;
	SuperClass*        SourceSW;

	TemporalClass*     NextTemporal;
	TemporalClass*     PrevTemporal;

	int                WarpRemaining;
	int                WarpPerStep;
};
