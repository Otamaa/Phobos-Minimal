#pragma once
#include <Ext/AA/Object/Body.h>
#include <AnimClass.h>

class AnimExt final : public AnimClass
{
public:
	static constexpr size_t Canary = 0xAAAAAAAA;
	using base_type = AnimClass;

	class ExtData final : public ObjectExt::ExtData
	{
	public:

		ExtData(const base_type* OwnerObject) : ObjectExt::ExtData(OwnerObject)
		{ }

		virtual ~ExtData() = default;

		virtual void SaveToStream(PhobosStreamWriter& Stm) { ObjectExt::ExtData::SaveToStream(Stm); }
		virtual void LoadFromStream(PhobosStreamReader& Stm) { ObjectExt::ExtData::LoadFromStream(Stm); }

		virtual void InvalidatePointer(void* target, bool all = true) override {
			ObjectExt::ExtData::InvalidatePointer(target, all);
		}

		virtual base_type* OwnerObject() const override { return reinterpret_cast<base_type*>(ObjectExt::ExtData::OwnerObject()); }

	protected:
		ExtData(const ExtData&) = delete;
		void operator = (const ExtData&) = delete;

	public:

		//TODO , remove this virtual junk
		virtual void InitializeConstants() override
		{
			ObjectExt::ExtData::InitializeConstants();
		}

		virtual void InitializeRuled() override
		{
			ObjectExt::ExtData::InitializeRuled();

		}

		virtual void Initialize() override
		{
			ObjectExt::ExtData::Initialize();
		}

		virtual void LoadFromRulesFile(CCINIClass* pINI) override
		{
			ObjectExt::ExtData::LoadFromRulesFile(pINI);
		}

		virtual void LoadFromINIFile(CCINIClass* pINI) override
		{
			ObjectExt::ExtData::LoadFromINIFile(pINI);
		}

	};
};

static_assert(sizeof(AnimExt) == sizeof(AnimClass), "Missmatch Size !");