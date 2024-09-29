#pragma once

#include <OverlayTypeClass.h>

#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/PhobosMap.h>

class OverlayTypeExtData final
{
public:
	using base_type = OverlayTypeClass;
	static constexpr size_t Canary = 0x414B4B4A;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:

	Valueable<PaletteManager*> Palette { };
	Valueable<int> ZAdjust { 0 };

	void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }
	constexpr FORCEINLINE static size_t size_Of()
	{
		return sizeof(OverlayTypeExtData) -
			(4u //AttachedToObject
			 );
	}
private:
	template <typename T>
	void Serialize(T& Stm);
};

class OverlayTypeExtContainer final : public Container<OverlayTypeExtData>
{
public:
	static OverlayTypeExtContainer Instance;
	PhobosMap<OverlayTypeClass*, OverlayTypeExtData*> Map {};

	virtual bool Load(OverlayTypeClass* key, IStream* pStm);

	void Clear()
	{
		this->Map.clear();
	}

//	OverlayTypeExtContainer() : Container<OverlayTypeExtData> { "OverlayTypeClass" }
//		, Map {}
//	{ }
//
//	virtual ~OverlayTypeExtContainer() override = default;
//
//private:
//	OverlayTypeExtContainer(const OverlayTypeExtContainer&) = delete;
//	OverlayTypeExtContainer(OverlayTypeExtContainer&&) = delete;
//	OverlayTypeExtContainer& operator=(const OverlayTypeExtContainer& other) = delete;
};

