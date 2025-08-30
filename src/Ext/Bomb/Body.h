#pragma once
#include <BombClass.h>

#include <Utilities/Container.h>

class WeaponTypeExtData;
class BombExtData final : public AbstractExtended
{
public:
	using base_type = BombClass;

public:

	WeaponTypeExtData* Weapon { nullptr };

public:

	BombExtData(BombClass* pObj) : AbstractExtended(pObj) { }
	BombExtData(BombClass* pObj, noinit_t& nn) : AbstractExtended(pObj, nn) { }

	virtual ~BombExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override {

	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override {
		this->Internal_LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) const {
		this->Internal_SaveToStream(Stm);
		const_cast<BombExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return AbstractType::Bomb; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const { }

	virtual BombClass* This() const override { return reinterpret_cast<BombClass*>(this->AbstractExtended::This()); }
	virtual const BombClass* This_Const() const override { return reinterpret_cast<const BombClass*>(this->AbstractExtended::This_Const()); }

private:
	template <typename T>
	void Serialize(T& Stm);
};

class BombExtContainer final : public Container<BombExtData>
{
public:
	static BombExtContainer Instance;

	static void Clear()
	{
		Array.clear();
	}

	static bool LoadGlobals(PhobosStreamReader& Stm)
	{
		return true;
	}

	static bool SaveGlobals(PhobosStreamWriter& Stm)
	{
		return true;
	}

	static void InvalidatePointer(AbstractClass* const ptr, bool bRemoved)
	{
		for (auto& ext : Array)
		{
			ext->InvalidatePointer(ptr, bRemoved);
		}
	}

	virtual bool WriteDataToTheByteStream(BombExtData::base_type* key, IStream* pStm) { };
	virtual bool ReadDataFromTheByteStream(BombExtData::base_type* key, IStream* pStm) { };
};

class NOVTABLE FakeBombClass : public BombClass
{
public:

	HouseClass* _GetOwningHouse() {
		return this->OwnerHouse;
	}

	void _Detach(AbstractClass* target, bool all) { };

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

	BombExtData* _GetExtData() {
		return *reinterpret_cast<BombExtData**>(((DWORD)this) + AbstractExtOffset);
	}

};
static_assert(sizeof(FakeBombClass) == sizeof(BombClass), "Invalid Size !");