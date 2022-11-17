#pragma once
#include <Ext/AA/Mission/Body.h>
#include <RadioClass.h>

class RadioExt final : public RadioClass
{
public:
	static constexpr size_t Canary = 0xAAAAAAAA;
	using base_type = RadioClass;

	class ExtData : public MissionExt::ExtData
	{
	public:

		ExtData(const base_type* OwnerObject) : MissionExt::ExtData(OwnerObject)
		{ }

		virtual ~ExtData() = default;

		virtual void SaveToStream(PhobosStreamWriter& Stm) { MissionExt::ExtData::SaveToStream(Stm); }
		virtual void LoadFromStream(PhobosStreamReader& Stm) { MissionExt::ExtData::LoadFromStream(Stm); }

		virtual void InvalidatePointer(void* target, bool all = true) override {
			MissionExt::ExtData::InvalidatePointer(target, all);
		}

		virtual base_type* OwnerObject() const override { return reinterpret_cast<base_type*>(MissionExt::ExtData::OwnerObject()); }

	protected:
		ExtData(const ExtData&) = delete;
		void operator = (const ExtData&) = delete;

	public:

		//TODO , remove this virtual junk
		virtual void InitializeConstants() override
		{
			MissionExt::ExtData::InitializeConstants();
		}

		virtual void InitializeRuled() override
		{
			MissionExt::ExtData::InitializeRuled();

		}

		virtual void Initialize() override
		{
			MissionExt::ExtData::Initialize();
		}

		virtual void LoadFromRulesFile(CCINIClass* pINI) override
		{
			MissionExt::ExtData::LoadFromRulesFile(pINI);
		}

		virtual void LoadFromINIFile(CCINIClass* pINI) override
		{
			MissionExt::ExtData::LoadFromINIFile(pINI);
		}

	};
};

static_assert(sizeof(RadioExt) == sizeof(RadioClass), "Missmatch Size !");