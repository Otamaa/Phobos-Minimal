#pragma once
#include <BombClass.h>

#include <Utilities/Container.h>
#include <Ext/WeaponType/Body.h>

class WeaponTypeClass;
class BombExtData final
{
public:
	static constexpr size_t Canary = 0x87659781;
	using base_type = BombClass;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:

	WeaponTypeExtData* Weapon { nullptr };

	BombExtData() noexcept = default;
	~BombExtData() noexcept = default;

	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	constexpr FORCEINLINE static size_t size_Of()
	{
		return sizeof(BombExtData) -
			(4u //AttachedToObject
			 );
	}

	static HouseClass* __fastcall GetOwningHouse(BombClass* pThis, void*);
	static void __fastcall InvalidatePointer(BombClass* pThis, void*, void* const ptr, bool removed);

private:
	template <typename T>
	void Serialize(T& Stm);
};

class BombExtContainer final : public Container<BombExtData>
{
public:
	static BombExtContainer Instance;

	CONSTEXPR_NOCOPY_CLASSB(BombExtContainer, BombExtData, "BombClass");
};
