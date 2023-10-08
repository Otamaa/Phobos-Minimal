#pragma once

#include <OverlayTypeClass.h>

#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>

class OverlayTypeExt
{
public:
	class ExtData final : public Extension<OverlayTypeClass>
	{
	public:
		using base_type = OverlayTypeClass;
		static constexpr size_t Canary = 0x414B4B4A;

	public:

		Valueable<PaletteManager*> Palette { };

		ExtData(OverlayTypeClass* OwnerObject) : Extension<OverlayTypeClass>(OwnerObject)
		{ }

		virtual ~ExtData() = default;

		void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<OverlayTypeExt::ExtData>
	{
	public:
		CONSTEXPR_NOCOPY_CLASS(OverlayTypeExt::ExtData, "OverlayTypeClass");
	};

	static ExtContainer ExtMap;
};