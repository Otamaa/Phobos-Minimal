#pragma once
#include <TemporalClass.h>

#include <Utilities/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>

class WeaponTypeClass;
class TemporalExt
{
public:
	class ExtData final : public Extension<TemporalClass>
	{
	public:
		static constexpr size_t Canary = 0x82229781;
		using base_type = TemporalClass;

	public:

		WeaponTypeClass* Weapon;
		ExtData(TemporalClass* OwnerObject) : Extension<TemporalClass>(OwnerObject)
			, Weapon { nullptr }
		{ }

		virtual ~ExtData() override = default;
		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<TemporalExt::ExtData>
	{
	public:
		ExtContainer();
		~ExtContainer();

	};

	static ExtContainer ExtMap;
};