#pragma once
#include <Ext/AA/Foot/Body.h>
#include <AircraftClass.h>

class AircraftExt final : public AircraftClass
{
public:
	static constexpr size_t Canary = 0xAAAAAAAA;
	using base_type = AircraftClass;

	class ExtData : public FootExt::ExtData
	{
	public:

		ExtData(const base_type* OwnerObject) : FootExt::ExtData(OwnerObject)
		{ }

		virtual ~ExtData() = default;

		virtual void SaveToStream(PhobosStreamWriter& Stm) { FootExt::ExtData::SaveToStream(Stm); }
		virtual void LoadFromStream(PhobosStreamReader& Stm) { FootExt::ExtData::LoadFromStream(Stm); }

		virtual void InvalidatePointer(void* target, bool all = true) override {
			FootExt::ExtData::InvalidatePointer(target, all);
		}

		virtual base_type* OwnerObject() const override { return reinterpret_cast<base_type*>(FootExt::ExtData::OwnerObject()); }

	protected:
		ExtData(const ExtData&) = delete;
		void operator = (const ExtData&) = delete;

	public:

		//TODO , remove this virtual junk
		virtual void InitializeConstants() override
		{
			FootExt::ExtData::InitializeConstants();
		}

		virtual void InitializeRuled() override
		{
			FootExt::ExtData::InitializeRuled();

		}

		virtual void Initialize() override
		{
			FootExt::ExtData::Initialize();
		}

		virtual void LoadFromRulesFile(CCINIClass* pINI) override
		{
			FootExt::ExtData::LoadFromRulesFile(pINI);
		}

		virtual void LoadFromINIFile(CCINIClass* pINI) override
		{
			FootExt::ExtData::LoadFromINIFile(pINI);
		}

	};
};

static_assert(sizeof(AircraftExt) == sizeof(AircraftClass), "Missmatch Size !");