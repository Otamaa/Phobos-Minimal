#pragma once
#include <Ext/AA/Ext/AbstractType/Body.h>
#include <ObjectTypeClass.h>

class ObjectTypeExt : public ObjectTypeClass
{
public:
	using base_type = ObjectTypeClass;

	class ExtData : public AbstractTypeExt::ExtData
	{
	public:

		ExtData(const ObjectTypeClass* this_ptr) : AbstractTypeExt::ExtData(this_ptr)
		{ }

		virtual ~ExtData() = default;

		virtual void SaveToStream(PhobosStreamWriter& Stm)
		{
			AbstractTypeExt::ExtData::SaveToStream(Stm);
		}

		virtual void LoadFromStream(PhobosStreamReader& Stm)
		{
			AbstractTypeExt::ExtData::LoadFromStream(Stm);
		}

		virtual void InvalidatePointer(void* target, bool all = true) { AbstractTypeExt::ExtData::InvalidatePointer(target, all); }
		virtual base_type* OwnerObject() const override { return reinterpret_cast<base_type*>(AbstractTypeExt::ExtData::OwnerObject()); }

	private:
		ExtData(const ExtData&) = delete;
		void operator = (const ExtData&) = delete;

	public:

		virtual void InitializeConstants() override { AbstractTypeExt::ExtData::InitializeConstants(); }
		virtual void InitializeRuled() { AbstractTypeExt::ExtData::InitializeRuled(); }
		virtual void Initialize() { AbstractTypeExt::ExtData::Initialize(); }
		virtual void LoadFromRulesFile(CCINIClass* pINI) { AbstractTypeExt::ExtData::LoadFromRulesFile(pINI); }
		virtual void LoadFromINIFile(CCINIClass* pINI) { AbstractTypeExt::ExtData::LoadFromINIFile(pINI); }

	};


	class ExtContainer final : public ExtensionWrapperAbract<AbstractTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

};

static_assert(sizeof(ObjectTypeExt) == sizeof(ObjectTypeClass), "Missmatch Size !");
