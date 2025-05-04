#pragma once
#include <BombClass.h>

#include <Utilities/Container.h>
#include <Ext/WeaponType/Body.h>

class WeaponTypeClass;
class BombExtData final
{
public:
	static COMPILETIMEEVAL size_t Canary = 0x87659781;
	using base_type = BombClass;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:

	WeaponTypeExtData* Weapon { nullptr };

	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	COMPILETIMEEVAL FORCEDINLINE static size_t size_Of()
	{
		return sizeof(BombExtData) -
			(4u //AttachedToObject
			 );
	}

private:
	template <typename T>
	void Serialize(T& Stm);
};

class BombExtContainer final : public Container<BombExtData>
{
public:
	static BombExtContainer Instance;

	//CONSTEXPR_NOCOPY_CLASSB(BombExtContainer, BombExtData, "BombClass");
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