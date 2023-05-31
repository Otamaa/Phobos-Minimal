#pragma once
#include <BombClass.h>

#include <Ext/Abstract/Body.h>
#include <Ext/WeaponType/Body.h>

class WeaponTypeClass;
class BombExt
{
	public:
	class ExtData final : public Extension<BombClass>
	{
	public:
		static constexpr size_t Canary = 0x87659781;
		using base_type = BombClass;

	public:

		WeaponTypeExt::ExtData* Weapon;
		ExtData(BombClass* OwnerObject) : Extension<BombClass>(OwnerObject)
			, Weapon { nullptr }
		{ }

		virtual ~ExtData() override = default;

		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<BombExt::ExtData>
	{
	public:
		ExtContainer();
		~ExtContainer();

	};

	static ExtContainer ExtMap;

	static HouseClass* FC GetOwningHouse(BombClass* pThis, void*);
	static void FC InvalidatePointer(BombClass* pThis, void*, void* const ptr, bool removed);
};