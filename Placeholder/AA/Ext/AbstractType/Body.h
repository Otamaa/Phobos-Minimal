#pragma once
#include <Ext/AA/Ext/Abstract/Body.h>
#include <AbstractTypeClass.h>

class AbstractTypeExt : public AbstractTypeClass
{
public:
	using base_type = AbstractTypeClass;

	class ExtData : public AbstractExt::ExtData
	{
	public:

		ExtData(const AbstractTypeClass* this_ptr) : AbstractExt::ExtData(this_ptr)
		{ }

		virtual ~ExtData() = default;

		virtual void SaveToStream(PhobosStreamWriter& Stm)
		{
			AbstractExt::ExtData::Internal_Save(Stm);
		}

		virtual void LoadFromStream(PhobosStreamReader& Stm)
		{
			AbstractExt::ExtData::Internal_Load(Stm);
		}

		virtual void InvalidatePointer(void* target, bool all = true) { AbstractExt::ExtData::InvalidatePointer(target,all); }
		virtual base_type* OwnerObject() const override { return reinterpret_cast<base_type*>(AbstractExt::ExtData::OwnerObject()); }

	private:
		ExtData(const ExtData&) = delete;
		void operator = (const ExtData&) = delete;

	public:

		virtual void InitializeConstants() override { AbstractExt::ExtData::InitializeConstants(); }
		virtual void InitializeRuled() { AbstractExt::ExtData::InitializeRuled(); }
		virtual void Initialize() { AbstractExt::ExtData::Initialize(); }
		virtual void LoadFromRulesFile(CCINIClass* pINI) { AbstractExt::ExtData::LoadFromRulesFile(pINI); }
		virtual void LoadFromINIFile(CCINIClass* pINI) { AbstractExt::ExtData::LoadFromINIFile(pINI); }

	};

	class ExtContainer final : public ExtensionWrapperAbract<AbstractTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

};

static_assert(sizeof(AbstractTypeExt) == sizeof(AbstractTypeClass), "Missmatch Size !");
