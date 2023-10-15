#pragma once

#include <OverlayTypeClass.h>

#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>

class OverlayTypeExtData final
{
public:
	using base_type = OverlayTypeClass;
	static constexpr size_t Canary = 0x414B4B4A;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:

	Valueable<PaletteManager*> Palette { };

	OverlayTypeExtData(base_type* OwnerObject) noexcept
	{
		AttachedToObject = OwnerObject;
	}

	~OverlayTypeExtData() noexcept = default;

	void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

private:
	template <typename T>
	void Serialize(T& Stm);
};

class OverlayTypeExtContainer final : public Container<OverlayTypeExtData>
{
public:
	static OverlayTypeExtContainer Instance;

	CONSTEXPR_NOCOPY_CLASSB(OverlayTypeExtContainer, OverlayTypeExtData, "OverlayTypeClass");
};

