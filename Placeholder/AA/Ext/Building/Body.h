#pragma once
#include <Ext/AA/Techno/Body.h>
#include <BuildingClass.h>

class BuildingExt final : public BuildingClass
{
public:
	static constexpr size_t Canary = 0xAAAAAAAA;
	using base_type = BuildingClass;

	class ExtData : public TechnoExt::ExtData
	{
	public:

		ExtData(const base_type* OwnerObject) : TechnoExt::ExtData(OwnerObject)
		{ }

		virtual ~ExtData() = default;

		virtual void SaveToStream(PhobosStreamWriter& Stm) { TechnoExt::ExtData::SaveToStream(Stm); }
		virtual void LoadFromStream(PhobosStreamReader& Stm) { TechnoExt::ExtData::LoadFromStream(Stm); }

		virtual void InvalidatePointer(void* target, bool all = true) override {
			TechnoExt::ExtData::InvalidatePointer(target, all);
		}

		virtual base_type* OwnerObject() const override { return reinterpret_cast<base_type*>(TechnoExt::ExtData::OwnerObject()); }

	protected:
		ExtData(const ExtData&) = delete;
		void operator = (const ExtData&) = delete;

	public:

		//TODO , remove this virtual junk
		virtual void InitializeConstants() override
		{
			TechnoExt::ExtData::InitializeConstants();
		}

		virtual void InitializeRuled() override
		{
			TechnoExt::ExtData::InitializeRuled();

		}

		virtual void Initialize() override
		{
			TechnoExt::ExtData::Initialize();
		}

		virtual void LoadFromRulesFile(CCINIClass* pINI) override
		{
			TechnoExt::ExtData::LoadFromRulesFile(pINI);
		}

		virtual void LoadFromINIFile(CCINIClass* pINI) override
		{
			TechnoExt::ExtData::LoadFromINIFile(pINI);
		}

	};
};

static_assert(sizeof(BuildingExt) == sizeof(BuildingClass), "Missmatch Size !");