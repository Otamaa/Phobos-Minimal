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

	OverlayTypeExtData() noexcept = default;
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
	std::unordered_map<OverlayTypeClass*, OverlayTypeExtData*> Map;

	virtual bool Load(OverlayTypeClass* key, IStream* pStm);

	void Clear()
	{
		this->Map.clear();
	}

	OverlayTypeExtContainer() : Container<OverlayTypeExtData> { "OverlayTypeClass" }
		, Map {}
	{ }

	virtual ~OverlayTypeExtContainer() override = default;

private:
	OverlayTypeExtContainer(const OverlayTypeExtContainer&) = delete;
	OverlayTypeExtContainer(OverlayTypeExtContainer&&) = delete;
	OverlayTypeExtContainer& operator=(const OverlayTypeExtContainer& other) = delete;
};

