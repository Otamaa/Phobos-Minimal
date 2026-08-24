#pragma once
#include <BombClass.h>
#include <BombListClass.h>

#include <Utilities/Container.h>

class WeaponTypeExtData;
class BombExtData final : public AbstractExtended
{
public:
	using base_type = BombClass;
	static COMPILETIMEEVAL const char* ClassName = "BombExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "BombClass";
	static COMPILETIMEEVAL DWORD Canary = 0x5353FF6F;

public:

	WeaponTypeExtData* Weapon { nullptr };

public:

	BombExtData(BombClass* pObj) : AbstractExtended(pObj) {
		this->AbsType = BombClass::AbsID;
	}

	BombExtData() = default;

	virtual ~BombExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved, AbstractType  type) override {
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override {
		this->Internal_LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) {
		this->Internal_SaveToStream(Stm);
		const_cast<BombExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return AbstractType::Bomb; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const { }

	BombClass* This() const { return reinterpret_cast<BombClass*>(this->AttachedToObject); }
	const BombClass* This_Const() const { return reinterpret_cast<const BombClass*>(this->AttachedToObject); }

private:
	template <typename T>
	void Serialize(T& Stm);
};

class BombExtContainer final : public Container<BombExtData>
	, public ContainerSaveLoad<BombExtContainer, BombExtData>
{
public:

	static COMPILETIMEEVAL const char* ClassName = "BombExtContainer";

public:
	static BombExtContainer Instance;

	virtual bool SaveGlobal(PhobosStreamWriter& stm) { return true; }
	virtual bool LoadGlobal(PhobosStreamReader& stm) { return true; }

};

class NOVTABLE FakeBombClass : public BombClass
{
public:

	HRESULT __stdcall __Load(IStream* pStm);
	HRESULT __stdcall __Save(IStream* pStm, BOOL fClearDirty);

	HouseClass* _GetOwningHouse() {
		return this->OwnerHouse;
	}

	void _Detach(AbstractClass* target, bool all) { };
	void __Detonate();
	int __GetBombFrame();

	BombExtData* _GetExtData() {
		return *reinterpret_cast<BombExtData**>(((DWORD)this) + AbstractExtOffset);
	}

};
static_assert(sizeof(FakeBombClass) == sizeof(BombClass), "Invalid Size !");

class NOVTABLE FakeBombListClass
{
public:
	void __AI();
	void __Plant(TechnoClass* pOwner, ObjectClass* pTarget);
	void __PlantB(TechnoClass* pOwner, ObjectClass* pTarget, WeaponTypeClass* pWeapon);
public:

	DynamicVectorClass<BombClass*> Bombs;				//all the BombClass instances on the map
	DynamicVectorClass<TechnoClass*> Detectors;		//all the BombSight'ed objects currently on the map
	int UpdateDelay; // defaults to 100, some iterators set it to 1
};

static_assert(sizeof(FakeBombListClass) == sizeof(BombListClass), "Invalid Size !");