#pragma once
#include <Ext/AA/Ext/Abstract/Body.h>
#include <ObjectClass.h>

class ObjectExt final : public ObjectClass
{
public:
	static constexpr size_t Canary = 0x3939618A;
	using base_type = ObjectClass;

	class ExtData : public AbstractExt::ExtData
	{
	public:

		ExtData(const base_type* OwnerObject) : AbstractExt::ExtData(OwnerObject)
		{ }

		virtual ~ExtData() = default;

		virtual void SaveToStream(PhobosStreamWriter& Stm) { AbstractExt::ExtData::Internal_Save(Stm); }
		virtual void LoadFromStream(PhobosStreamReader& Stm) { AbstractExt::ExtData::Internal_Load(Stm); }

		virtual void InvalidatePointer(void* target, bool all = true) override {
			AbstractExt::ExtData::InvalidatePointer(target, all);
		}

		virtual base_type* OwnerObject() const override { return reinterpret_cast<base_type*>(AbstractExt::ExtData::OwnerObject()); }

	protected:
		ExtData(const ExtData&) = delete;
		void operator = (const ExtData&) = delete;

	public:

		//TODO , remove this virtual junk
		virtual void InitializeConstants() override { 
			AbstractExt::ExtData::InitializeConstants();
		}

		virtual void InitializeRuled() override {
			AbstractExt::ExtData::InitializeRuled();

		}

		virtual void Initialize() override {
			AbstractExt::ExtData::Initialize();
		}

		virtual void LoadFromRulesFile(CCINIClass* pINI) override  { 
			AbstractExt::ExtData::LoadFromRulesFile(pINI);
		}

		virtual void LoadFromINIFile(CCINIClass* pINI) override  { 
			AbstractExt::ExtData::LoadFromINIFile(pINI);
		}

	};

	class ExtContainer final : public ExtensionWrapperAbract<ObjectExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

};

static_assert(sizeof(ObjectExt) == sizeof(ObjectClass), "Missmatch Size !");