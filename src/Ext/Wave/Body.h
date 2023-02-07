#pragma once
#include <WaveClass.h>

#include <Ext/Abstract/Body.h>
#include <Utilities/Template.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Debug.h>
#include <Helpers/Macro.h>

class WaveExt
{
public:

	static constexpr size_t Canary = 0xAAAAAAAC;
	using base_type = WaveClass;

	class ExtData final : public Extension<WaveClass>
	{
	public:

		ExtData(WaveClass* OwnerObject) : Extension<WaveClass>(OwnerObject)
		{ }

		virtual ~ExtData() override = default;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
		virtual bool InvalidateIgnorable(void* const ptr) const override { return true; }
		virtual void LoadFromStream(PhobosStreamReader& Stm)override;
		virtual void SaveToStream(PhobosStreamWriter& Stm)override;
		virtual void Initialize() override;
		
		void InitWeaponData();
	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<WaveExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};
