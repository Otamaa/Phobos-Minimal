#pragma once
#include <ParasiteClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>

class ParasiteExt
{
public:
	static constexpr size_t Canary = 0x99954321;
	using base_type = ParasiteClass;

	class ExtData final : public TExtension<ParasiteClass>
	{
	public:

		ExtData(ParasiteClass* OwnerObject) : TExtension<ParasiteClass>(OwnerObject)
		{ }

		virtual ~ExtData() override = default;
		//void InvalidatePointer(void* ptr, bool bRemoved) {}
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		void InitializeConstants() { }
	private:
		template <typename T>
		void Serialize(T& Stm);
	};


	class ExtContainer final : public TExtensionContainer<ParasiteExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};