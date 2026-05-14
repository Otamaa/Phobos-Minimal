#pragma once

#include <Ext/AbstractType/Body.h>
#include <ObjectTypeClass.h>

class ObjectTypeExtData : public AbstractTypeExtData
{
public:

	ObjectTypeExtData(ObjectTypeClass* pObj) : AbstractTypeExtData(pObj) { }
	ObjectTypeExtData(ObjectTypeClass* pObj, noinit_t nn) : AbstractTypeExtData(pObj, nn) { }

	virtual ~ObjectTypeExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved, AbstractType  type) override {
		this->AbstractTypeExtData::InvalidatePointer(ptr, bRemoved, type);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->AbstractTypeExtData::Internal_LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		this->AbstractTypeExtData::Internal_SaveToStream(Stm);
	}

	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const {
		this->AbstractTypeExtData::CalculateCRC(crc);
	}

	ObjectTypeClass* This() const { return reinterpret_cast<ObjectTypeClass*>(this->AttachedToObject); }
	const ObjectTypeClass* This_Const() const { return reinterpret_cast<const ObjectTypeClass*>(this->AttachedToObject); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr) { return parseFailAddr ? false : true; }
	virtual bool WriteToINI(CCINIClass* pINI) const { return true; }
};

class BuildingTypeClass;
class HouseClass;
class FakeObjectTypeClass final
{
public:

	static BuildingClass* __fastcall WhoCanBuildMe(ObjectTypeClass* pThis, discard_t, bool intheory, bool bool2, bool legal, HouseClass* house);

	// Backport of ObjectTypeClass::LoadVoxel (0x5F8110–0x5F8CDB).
	// Loads VXL/HVA art assets for the type's main body, turrets, and barrels.
	// Integrates Phobos NoSpawnAlt, multi-turret, and multi-barrel extensions inline.
	// static void __fastcall _LoadVoxel(ObjectTypeClass* pThis, discard_t);
};