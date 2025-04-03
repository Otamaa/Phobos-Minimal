#pragma once
#include <SpawnManagerClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class SpawnManagerExt
{
public:
	/*
	class ExtData final : public Extension<SpawnManagerClass>
	{
	public:
		static COMPILETIMEEVAL size_t Canary = 0x99954321;
		using base_type = SpawnManagerClass;

	public:

		ExtData(SpawnManagerClass* OwnerObject) : Extension<SpawnManagerClass>(OwnerObject)
		{ }

		virtual ~ExtData() override = default;
		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<SpawnManagerExt::ExtData>
	{
	public:
		CONSTEXPR_NOCOPY_CLASS(SpawnManagerExt::ExtData, "SpawnManagerClass");
	};

	static ExtContainer ExtMap;
	*/
};
