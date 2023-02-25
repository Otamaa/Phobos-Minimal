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

	class ExtData final : public Extension<ParasiteClass>
	{
	public:

		CoordStruct LastVictimLocation;
		ExtData(ParasiteClass* OwnerObject) : Extension<ParasiteClass>(OwnerObject)
			, LastVictimLocation {}
		{ }

		virtual ~ExtData() override = default;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
		virtual bool InvalidateIgnorable(void* const ptr) const override { return true; };
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		virtual void InitializeConstants() override { }

	private:
		template <typename T>
		void Serialize(T& Stm);
	};


	class ExtContainer final : public Container<ParasiteExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};