#pragma once
#include <Ext/AA/Radio/Body.h>
#include <TechnoClass.h>

class TechnoExt final : public TechnoClass
{
public:
	static constexpr size_t Canary = 0xAAAAAAAA;
	using base_type = TechnoClass;

	class ExtData : public RadioExt::ExtData
	{
	public:

		ExtData(const base_type* OwnerObject) : RadioExt::ExtData(OwnerObject)
		{ }

		virtual ~ExtData() = default;

		virtual void SaveToStream(PhobosStreamWriter& Stm) { RadioExt::ExtData::SaveToStream(Stm); }
		virtual void LoadFromStream(PhobosStreamReader& Stm) { RadioExt::ExtData::LoadFromStream(Stm); }

		virtual void InvalidatePointer(void* target, bool all = true) override {
			RadioExt::ExtData::InvalidatePointer(target, all);
		}

		virtual base_type* OwnerObject() const override { return reinterpret_cast<base_type*>(RadioExt::ExtData::OwnerObject()); }

	protected:
		ExtData(const ExtData&) = delete;
		void operator = (const ExtData&) = delete;

	public:

		//TODO , remove this virtual junk
		virtual void InitializeConstants() override
		{
			RadioExt::ExtData::InitializeConstants();
		}

		virtual void InitializeRuled() override
		{
			RadioExt::ExtData::InitializeRuled();

		}

		virtual void Initialize() override
		{
			RadioExt::ExtData::Initialize();
		}

		virtual void LoadFromRulesFile(CCINIClass* pINI) override
		{
			RadioExt::ExtData::LoadFromRulesFile(pINI);
		}

		virtual void LoadFromINIFile(CCINIClass* pINI) override
		{
			RadioExt::ExtData::LoadFromINIFile(pINI);
		}

	};
};

static_assert(sizeof(TechnoExt) == sizeof(TechnoClass), "Missmatch Size !");