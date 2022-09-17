#pragma once
#include <BombClass.h>

#include <Utilities/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class BombExt
{
public:
#ifdef ENABLE_NEWHOOKS
	static constexpr size_t Canary = 0x87659781;
	using base_type = BombClass;
	static constexpr size_t ExtOffset = sizeof(base_type);


	class ExtData final : public Extension<BombClass>
	{
	public:

		ExtData(BombClass* OwnerObject) : Extension<BombClass>(OwnerObject)
		{ }

		virtual ~ExtData() override = default;
		// void InvalidatePointer(void* ptr, bool bRemoved) { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

		// void InitializeConstants() override { }

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<BombExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		void InvalidatePointer(void* ptr, bool bRemoved);
	};

	static ExtContainer ExtMap;
#endif
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static BombClass* BombTemp;
	static HouseClass* __fastcall GetOwningHouse(BombClass* pThis, void* _);
	static DamageAreaResult __fastcall DamageArea(CoordStruct* pCoord, int Damage, TechnoClass* Source, WarheadTypeClass* Warhead, bool AffectTiberium, HouseClass* SourceHouse);
};