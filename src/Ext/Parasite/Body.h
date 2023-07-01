#pragma once
#include <ParasiteClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>

class ParasiteExt
{
public:
	class ExtData final : public Extension<ParasiteClass>
	{
	public:
		static constexpr size_t Canary = 0x99954321;
		using base_type = ParasiteClass;

	public:

		CoordStruct LastVictimLocation {};
		ExtData(ParasiteClass* OwnerObject) : Extension<ParasiteClass>(OwnerObject)
		{ }

		virtual ~ExtData() override = default;
		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	private:
		template <typename T>
		void Serialize(T& Stm);
	};


	class ExtContainer final : public Container<ParasiteExt::ExtData>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};