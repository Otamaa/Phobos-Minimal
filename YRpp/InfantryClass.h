/*
	Infantry
*/

#pragma once

#include <FootClass.h>
#include <InfantryTypeClass.h>

class DECLSPEC_UUID("0E272DC4-9C0F-11D1-B709-00A024DDAFD1")
	NOVTABLE InfantryClass : public FootClass
{
public:
	static const AbstractType AbsID = AbstractType::Infantry;

	//Static
	static constexpr constant_ptr<DynamicVectorClass<InfantryClass*>, 0xA83DE8u> const Array{};
	static constexpr inline DWORD vtable = 0x7EB058;

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x523300);
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x521960);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x521B00);

	//Destructor
	virtual ~InfantryClass() override JMP_THIS(0x523350);

	//AbstractClass
	virtual void PointerExpired(AbstractClass* pAbstract, bool removed) override { JMP_THIS(0x51AA10); }
	virtual AbstractType WhatAmI() const override  RT(AbstractType);
	virtual int	Size() const override  R0;
	virtual void Update() override JMP_THIS(0x51BAB0);

	//ObjectClass
	virtual Action MouseOverObject(ObjectClass const* pObject, bool ignoreForce = false) const override { JMP_THIS(0x51E3B0); }
	virtual DamageState ReceiveDamage(int* pDamage, int DistanceFromEpicenter, WarheadTypeClass* pWH, ObjectClass* Attacker, bool IgnoreDefenses, bool PreventPassengerEscape, HouseClass* pAttackingHouse) override { JMP_THIS(0x517FA0); }

	//TechnoClass
	virtual int SelectWeapon(AbstractClass* pTarget) const override { JMP_THIS(0x5218E0); }
	virtual FireError GetFireError(AbstractClass *pTarget, int nWeaponIndex, bool ignoreRange) const override { JMP_THIS(0x51C8B0); }
	virtual CoordStruct* GetFLH(CoordStruct* pDest, int idxWeapon, CoordStruct BaseCoords) const override { JMP_THIS(0x523250); }

	//FootClass
	virtual bool ApproachTarget(bool bSomething) override { JMP_THIS(0x522340); }

	//InfantryClass
	virtual bool IsDeployed() const R0;
	virtual bool PlayAnim(DoType index, bool force = false, bool randomStartFrame = false) { JMP_THIS(0x51D6F0); } //`InfantryClass::Do_Action

	bool IsDoingDeploySequence() { JMP_THIS(0x522510); }
	void UnslaveMe();
	void RemoveMe_FromGunnerTransport();
	void ForceHarvest() const { JMP_THIS(0x522D00); }
	bool IsHarvesting() const { JMP_THIS(0x522FC0); }
	int UpdateDeploy() const { JMP_THIS(0x521320); } //return frames

	//Constructor
	InfantryClass(InfantryTypeClass* pType, HouseClass* pOwner) noexcept
		: InfantryClass(noinit_t())
	{ JMP_THIS(0x517A50); }

protected:
	explicit __forceinline InfantryClass(noinit_t) noexcept
		: FootClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	InfantryTypeClass* Type;
	DoType SequenceAnim; //which is currently playing
	CDTimerClass unknown_Timer_6C8;
	int          PanicDurationLeft; // set in ReceiveDamage on panicky units
	bool           PermanentBerzerk; // set by script action, not cleared anywhere
	bool           Technician;
	bool           IsStroked; //0x6DA
	bool           Crawling;
	bool           IsZoneCheat; //0x6DC
	bool           _WasSelected; //0x6DD
	BYTE			padding_6DE[2];
	DWORD          unknown_6E0;
	bool           ShouldDeploy;
	LandType            _OnLand; //6E8
	PROTECTED_PROPERTY(DWORD, unused_6EC); //??
};
static_assert(offsetof(InfantryClass, padding_6DE) == 0x6DE, "ClassMember Shifted !");
static_assert(offsetof(InfantryClass, unknown_6E0) == 0x6E0, "ClassMember Shifted !");
static_assert(sizeof(InfantryClass) == 0x6F0, "Invalid Size !");