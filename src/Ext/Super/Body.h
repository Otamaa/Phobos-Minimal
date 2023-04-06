#pragma once
#include <SuperClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>

class SuperExt
{
public:

	static constexpr size_t Canary = 0x12311111;
	using base_type = SuperClass;

	class ExtData final : public Extension<SuperClass>
	{
	public:

		bool Temp_IsPlayer;
		CellStruct Temp_CellStruct;
		ExtData(SuperClass* OwnerObject) : Extension<SuperClass>(OwnerObject)
			, Temp_IsPlayer { false }
			, Temp_CellStruct { }
		{ }

		virtual ~ExtData() override  = default;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
		virtual bool InvalidateIgnorable(void* const ptr) const override { return true; }
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<SuperExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};