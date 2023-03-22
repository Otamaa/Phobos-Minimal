#pragma once

#include <GeneralStructures.h>
#include <ObjectClass.h>

class DECLSPEC_UUID("54822258-D8A8-11D1-B462-006097C6A979")
	NOVTABLE BuildingLightClass : public ObjectClass
{
public:
	static const AbstractType AbsID = AbstractType::BuildingLight;

	//Static
	static constexpr constant_ptr<DynamicVectorClass<BuildingLightClass*>, 0x8B4190u> const Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override R0;

	//IPersistStream
	virtual HRESULT __stdcall Save(IStream* pStm,BOOL fClearDirty) override R0;

	//AbstractClass
	virtual AbstractType WhatAmI() const override RT(AbstractType);
	virtual int	Size() const override R0;

	//Destructor
	virtual ~BuildingLightClass() RX;

	// non-virtual
	int GetMovementRadius() const
		{ JMP_THIS(0x436E80); }

	int GetSpotlightRadius() const
		{ JMP_THIS(0x436DA0); }

	void SetBehaviour(SpotlightBehaviour mode)
		{ JMP_THIS(0x436BE0); }

	void Func_436A40(ObjectClass* pAttachedTo) const
		{ JMP_THIS(0x436A40); }

	//Constructor
	BuildingLightClass(ObjectClass* pOwner) noexcept
		: BuildingLightClass(noinit_t())
	{ JMP_THIS(0x435820); }

protected:
	explicit __forceinline BuildingLightClass(noinit_t) noexcept
		: ObjectClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	double Speed;
	CoordStruct field_B8;
	CoordStruct field_C4;
	double Acceleration;
	bool Direction;
	SpotlightBehaviour BehaviourMode;
	ObjectClass * FollowingObject;
	TechnoClass * OwnerObject;
};
