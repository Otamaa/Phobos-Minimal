#pragma once
#include <BombClass.h>

#include <Utilities/Container.h>

class WeaponTypeExtData;
class BombExtData final : public AbstractExtended
{
public:
	using base_type = BombClass;
	static constexpr unsigned Marker = UuidFirstPart<base_type>::value;

public:

	WeaponTypeExtData* Weapon;

public:

	BombExtData(BombClass* pObj) : AbstractExtended(pObj) , Weapon(nullptr) {
		this->Name = "BombClass";
		this->AbsType = BombClass::AbsID;
	}
	BombExtData(BombClass* pObj, noinit_t nn) : AbstractExtended(pObj, nn) { }

	virtual ~BombExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override {

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

	BombClass* This() const override { return reinterpret_cast<BombClass*>(this->AttachedToObject); }
	const BombClass* This_Const() const override { return reinterpret_cast<const BombClass*>(this->AttachedToObject); }

private:
	template <typename T>
	void Serialize(T& Stm);
};

class BombExtContainer final : public Container<BombExtData>
{
public:
	static BombExtContainer Instance;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static void InvalidatePointer(AbstractClass* const ptr, bool bRemoved)
	{
		for (auto& ext : Array)
		{
			ext->InvalidatePointer(ptr, bRemoved);
		}
	}
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

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, BOOL clearDirty);

	BombExtData* _GetExtData() {
		return *reinterpret_cast<BombExtData**>(((DWORD)this) + AbstractExtOffset);
	}

};
static_assert(sizeof(FakeBombClass) == sizeof(BombClass), "Invalid Size !");