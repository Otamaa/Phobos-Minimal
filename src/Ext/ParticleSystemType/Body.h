#pragma once
#include <ParticleSystemTypeClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>

class ParticleSystemTypeExt
{
public:
	class ExtData final : public Extension<ParticleSystemTypeClass>
	{
	public:
		static constexpr size_t Canary = 0xEAEEEEEE;
		using base_type = ParticleSystemTypeClass;

	public:

		Valueable<bool> ApplyOptimization { true };

		ExtData(base_type* OwnerObject) : Extension<base_type>(OwnerObject)
		{ }

		virtual ~ExtData() override  = default;

		void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }
		void Initialize();

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<ParticleSystemTypeExt::ExtData>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};