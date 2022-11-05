#pragma once
#include <Ext/AA/Abstract/Body.h>
#include <CellClass.h>

class CellExt final : public CellClass
{
public:
	static constexpr size_t Canary = 0x3939618A;
	using base_type = CellClass;

	class ExtData : public AbstractClassExtData
	{
	public:

		ExtData(const base_type* OwnerObject) : AbstractClassExtData(OwnerObject)
		{ }

		virtual ~ExtData() = default;

		virtual void SaveToStream(PhobosStreamWriter& Stm) { AbstractClassExtData::Internal_Save(Stm); }
		virtual void LoadFromStream(PhobosStreamReader& Stm) { AbstractClassExtData::Internal_Load(Stm); }

		virtual void InvalidatePointer(void* target, bool all = true) override {
			AbstractClassExtData::InvalidatePointer(target, all);
		}

		virtual base_type* OwnerObject() const override { return reinterpret_cast<base_type*>(AbstractClassExtData::OwnerObject()); }

	protected:
		ExtData(const ExtData&) = delete;
		void operator = (const ExtData&) = delete;

	public:

		//TODO , remove this virtual junk
		virtual void InitializeConstants() override { 
			AbstractClassExtData::InitializeConstants();
		}

		virtual void InitializeRuled() override {
			AbstractClassExtData::InitializeRuled();

		}

		virtual void Initialize() override {
			AbstractClassExtData::Initialize();
		}

		virtual void LoadFromRulesFile(CCINIClass* pINI) override  { 
			AbstractClassExtData::LoadFromRulesFile(pINI);
		}

		virtual void LoadFromINIFile(CCINIClass* pINI) override  { 
			AbstractClassExtData::LoadFromINIFile(pINI);
		}

	};
};

static_assert(sizeof(CellExt) == sizeof(CellClass), "Missmatch Size !");