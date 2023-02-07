#pragma once
#include <TemporalClass.h>

#include <Utilities/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>

class WeaponTypeClass;
class TemporalExt
{
public:

	static constexpr size_t Canary = 0x82229781;
	using base_type = TemporalClass;

	class ExtData final : public Extension<TemporalClass>
	{
	public:

		WeaponTypeClass* Weapon;
		ExtData(TemporalClass* OwnerObject) : Extension<TemporalClass>(OwnerObject)
			, Weapon { nullptr }
		{ }

		virtual ~ExtData() override = default;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
		virtual bool InvalidateIgnorable(void* const ptr) const override { return true; }
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		virtual void InitializeConstants() override { }

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<TemporalExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

};