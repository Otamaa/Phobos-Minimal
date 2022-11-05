#pragma once
#include <Ext/AA/Object/Body.h>
#include <BulletClass.h>

class BulletExt final : public BulletClass
{
public:
	static constexpr size_t Canary = 0x3939618A;
	using base_type = BulletClass;

	class ExtData final : public ObjectExt::ExtData
	{
	public:

		ExtData(const base_type* OwnerObject) : ObjectExt::ExtData(OwnerObject)
		{ }

		virtual ~ExtData() = default;

		virtual void SaveToStream(PhobosStreamWriter& Stm) { ObjectExt::ExtData::Internal_Save(Stm); }
		virtual void LoadFromStream(PhobosStreamReader& Stm) { ObjectExt::ExtData::Internal_Load(Stm); }

		virtual void InvalidatePointer(void* target, bool all = true) override {
			ObjectExt::ExtData::InvalidatePointer(target, all);
		}

		virtual base_type* OwnerObject() const override { return reinterpret_cast<base_type*>(ObjectExt::ExtData::OwnerObject()); }

	protected:
		ExtData(const ExtData&) = delete;
		void operator = (const ExtData&) = delete;

	public:

		//TODO , remove this virtual junk
		virtual void InitializeConstants() override { 
			ObjectExt::ExtData::InitializeConstants();
		}

		virtual void InitializeRuled() override {
			ObjectExt::ExtData::InitializeRuled();

		}

		virtual void Initialize() override {
			ObjectExt::ExtData::Initialize();
		}

		virtual void LoadFromRulesFile(CCINIClass* pINI) override  { 
			ObjectExt::ExtData::LoadFromRulesFile(pINI);
		}

		virtual void LoadFromINIFile(CCINIClass* pINI) override  { 
			ObjectExt::ExtData::LoadFromINIFile(pINI);
		}

	};
};

static_assert(sizeof(BulletExt) == sizeof(BulletClass), "Missmatch Size !");