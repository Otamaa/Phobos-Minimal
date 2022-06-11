/*
	I have not the slightest idea what this is good for...
*/

#pragma once

#include <AbstractClass.h>
#include <ArrayClasses.h>

class DECLSPEC_UUID("241AB316-4CF5-11D2-BC26-00104B8FB04D")
	NOVTABLE NeuronClass : public AbstractClass
{
public:
	static const AbstractType AbsID = AbstractType::Neuron;

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) R0;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) R0;
	virtual HRESULT __stdcall Save(IStream* pStm,BOOL fClearDirty) R0;

	//Destructor
	virtual ~NeuronClass() RX;

	//AbstractClass
	virtual AbstractType WhatAmI() const RT(AbstractType);

	virtual int Size() const R0;

	//Constructor
	NeuronClass() noexcept
		: NeuronClass(noinit_t())
	{ JMP_THIS(0x43A350); }

protected:
	explicit __forceinline NeuronClass(noinit_t) noexcept
		: AbstractClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	void* unknown_ptr_24;
	void* unknown_ptr_28;
	void* unknown_ptr_2C;
	TimerStruct unknown_timer_30;
};

//Even more questions marks on the use of this... >.<
class BrainClass
{
public:
	virtual ~BrainClass() RX;

	BrainClass() noexcept  : BrainClass(noinit_t())
		{ JMP_THIS(0x43A600); }

	void Clear()
		{ JMP_THIS(0x43A670); }

	void Reset(int ndword_1C,int nMaxBrainCount)
		{ JMP_THIS(0x43A6B0); }

	bool Add(NeuronClass* pNeuron)
		{ JMP_THIS(0x43A700); }

	//save
	//load
protected:
	explicit __forceinline BrainClass(noinit_t) noexcept
	{ }

public:
	//Properties
	VectorClass<NeuronClass*> Neurons;
	int BrainCount;
	void* ptr_18;
	int dword_1C;
	int MaxBrainCount;
};

//static_assert(sizeof(BrainClass) == 0x24);
