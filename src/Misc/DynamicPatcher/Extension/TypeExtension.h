#pragma once

#include <typeinfo>
#include <stack>

#include <Helpers/Macro.h>
#include <Misc/DynamicPatcher/Helpers/Container.h>

#include <Misc/DynamicPatcher/Common/INI/INI.h>
#include <Misc/DynamicPatcher/Common/INI/INIConfig.h>

class AttachEffectTypeData;

template <typename TBase, typename TExt>
class TypeExtension
{
public:
	using base_type = TBase;

	class ExtData : public Extension<TBase>
	{
	public:
		ExtData(TBase *OwnerObject) : Extension<TBase>(OwnerObject)
		{ };

		virtual ~ExtData() override{};

		virtual void Initialize() override{};

		virtual void LoadFromINIFile(CCINIClass *pINI) override
		{
			// auto pThis = this->OwnerObject();
			// const char *pSection = pThis->ID;

			// if (!pINI->GetSection(pSection))
			// {
			// 	return;
			// }

			// INI_EX exINI(pINI);

			// read ini
		};

		virtual void LoadFromStream(PhobosStreamReader &Stm) override
		{
			Extension<TBase>::LoadFromStream(Stm);
			this->Serialize(Stm);
		};

		virtual void SaveToStream(PhobosStreamWriter &Stm) override
		{
			Extension<TBase>::SaveToStream(Stm);
			this->Serialize(Stm);
		};

		INIReader* pTypeData = nullptr;

		// AE配置缓存，只是弹头用
		AttachEffectTypeData* pTypeAEData = nullptr;

	private:
		template <typename T>
		void Serialize(T &Stm){};
	};

	class ExtContainer : public ExtMapCointainer<TExt>
	{
	public:
		ExtContainer() : ExtMapCointainer<TExt>(typeid(TExt).name()){};
		~ExtContainer() = default;
	};

	template<typename TypeData>
	static TypeData* GetData(TBase* base)
	{
		return GetTypeData<TExt, TypeData>(base);
	}
};
