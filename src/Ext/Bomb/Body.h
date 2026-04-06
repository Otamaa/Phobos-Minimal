#pragma once
#include <BombClass.h>

#include <Utilities/Container.h>

class WeaponTypeExtData;
class BombExtData final : public AbstractExtended
{
public:
	using base_type = BombClass;
	static COMPILETIMEEVAL const char* ClassName = "BombExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "BombClass";
	
public:

	WeaponTypeExtData* Weapon { nullptr };

public:

	BombExtData(BombClass* pObj) : AbstractExtended(pObj) {
		this->AbsType = BombClass::AbsID;
	}

	BombExtData(BombClass* pObj, noinit_t nn) : AbstractExtended(pObj, nn) { }

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
{
public:

	static COMPILETIMEEVAL const char* ClassName = "BombExtContainer";

public:
	static BombExtContainer Instance;

	virtual bool LoadAll(const PhobosStreamReader& stm) { return true; }
	virtual bool SaveAll(PhobosStreamWriter& stm){ return true; }
};

class NOVTABLE FakeBombClass : public BombClass
{
public:

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