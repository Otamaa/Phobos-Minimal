#pragma once

#include <AircraftClass.h>
#include <Utilities/Enum.h>
#include <Utilities/Container.h>

class AircraftExt
{
public:
	/*
	using base_type = AircraftClass;

	class ExtData final : public Extension<AircraftClass>
	{
	public:

		ExtData(AircraftClass* OwnerObject) : Extension<AircraftClass>(OwnerObject)
		{ }

		virtual ~ExtData() = default;
		virtual size_t Size() const { return sizeof(*this); }
		virtual void InvalidatePointer(void *ptr, bool bRemoved) override { }
		virtual void Uninitialize() override { }
		virtual void LoadFromStream(PhobosStreamReader& Stm)override;
		virtual void SaveToStream(PhobosStreamWriter& Stm)override;
		virtual void InitializeConstants() override { };

	private:
		template <typename T>
		void Serialize(T& Stm);

	};

	class ExtContainer final : public Container<AircraftExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);*/
	static void FireBurst(AircraftClass* pThis, AbstractClass* pTarget, AircraftFireMode shotNumber);
	static void _fastcall TriggerCrashWeapon(TechnoClass* pThis ,DWORD, int nMult);
};

