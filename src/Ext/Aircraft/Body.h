#pragma once

#include <AircraftClass.h>
#include <Utilities/Enum.h>
#include <Ext/Abstract/Body.h>

class AircraftExt
{
public:
//	static constexpr size_t Canary = 0x3939618A;
//	using base_type = AircraftClass;
//
//	class ExtData final : public TExtension<AircraftClass>
//	{
//	public:
//
//		ExtData(AircraftClass* OwnerObject) : TExtension<AircraftClass>(OwnerObject)
//		{ }
//
//		virtual ~ExtData() = default;
//		virtual size_t Size() const { return sizeof(*this); }
//		void InvalidatePointer(void *ptr, bool bRemoved) { }
//		virtual void LoadFromStream(PhobosStreamReader& Stm)override;
//		virtual void SaveToStream(PhobosStreamWriter& Stm)override;
//		virtual void InitializeConstants() override { };
//
//	private:
//		template <typename T>
//		void Serialize(T& Stm);
//
//	};
//
//	class ExtContainer final : public TExtensionContainer<AircraftExt>
//	{
//	public:
//		ExtContainer();
//		~ExtContainer();
//	};
//
//	static ExtContainer ExtMap;

	//static bool LoadGlobals(PhobosStreamReader& Stm);
	//static bool SaveGlobals(PhobosStreamWriter& Stm);
	static void FireBurst(AircraftClass* pThis, AbstractClass* pTarget, AircraftFireMode shotNumber);
	static void FireBurst(AircraftClass* pThis, AbstractClass* pTarget, AircraftFireMode shotNumber , int WeaponIdx);
	static void FireBurst(AircraftClass* pThis, AbstractClass* pTarget, AircraftFireMode shotNumber, int WeaponIdx, WeaponTypeClass* pWeapon);
	static void TriggerCrashWeapon(TechnoClass* pThis , int nMult);
};

