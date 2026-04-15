#pragma once

#include <Utilities/Container.h>

#include <ObjectClass.h>
#include <Utilities/PhobosFixedString.h>

class ObjectExtData : public AbstractExtended
{
public:
	PhobosFixedString<0x18> Name {};

public:

	ObjectExtData(ObjectClass* abs) : AbstractExtended(abs) {};

	ObjectExtData(ObjectClass* abs, noinit_t& noint) : AbstractExtended(abs, noint) { };

	virtual ~ObjectExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved, AbstractType  type) override {}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override {
		this->Internal_LoadFromStream(Stm);
		Stm.Process(Name);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) override {
		this->Internal_SaveToStream(Stm);
		Stm.Process(Name);
	}

	virtual int GetSize() const { return sizeof(*this); };

	ObjectClass* This() const { return reinterpret_cast<ObjectClass*>(AttachedToObject); }
	const ObjectClass* This_Const() const { return reinterpret_cast<const ObjectClass*>(AttachedToObject); }

	virtual void CalculateCRC(CRCEngine& crc) const override { }

public:

};

class NOVTABLE FakeObjectClass
{
public:

	static void __fastcall _DrawRadialIndicator(ObjectClass* pThis, discard_t, int val);
	static int __fastcall _GetDistanceOfObj(ObjectClass* pThis, discard_t, AbstractClass* pThat);
	static int __fastcall _GetDistanceOfCoord(ObjectClass* pThis, discard_t, CoordStruct* pThat);
	static CellClass* __fastcall _GetCell(ObjectClass* pThis, discard_t);
	static DamageState __fastcall __Take_Damage(ObjectClass* pThis, discard_t, int* damage, int distance, WarheadTypeClass* warhead, TechnoClass* source, bool ignoreDefenses, bool PreventsPassengerEscape, HouseClass* sourceHouse);

};